#ifndef __HTTPCLIENT_H
#define __HTTPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include "marshal.h"
#include "pollio.h"

namespace GNET {

void LOG_TRACE(const char *format, ...);
bool get_local_ip(std::vector<std::string> &ips);

class HttpClientSession;
struct HttpClientInterface
{
	virtual void OnOpen(int sid, HttpClientSession *) = 0;
	virtual void OnClose(int sid) { }
	virtual void OnFailed(int sid) { }
	virtual void Dispatch(int sid, OctetsStream &is) { is.clear(); }
};

class HttpClientIO;
class HttpConnectIO;
class HttpClientSession
{
	friend class HttpClientIO;
	friend class HttpConnectIO;
	HttpClientInterface *client;
	PollIO *assoc_io;
	Thread::Mutex locker;
	bool sending;
	bool closing;
	Octets ibuffer;
	Octets obuffer;
	std::deque<Octets> os;
	OctetsStream is;
	int sid;
	int nextsid()
	{
		Thread::Mutex::Scoped l(locker);
		static int session_id = 0;
		return ++session_id;
	}

	HttpClientSession(const HttpClientSession &rhs) : client(rhs.client),assoc_io(rhs.assoc_io),
		locker("HttpClientSession"),sending(rhs.sending),closing(rhs.closing),sid(rhs.sid)
	{
		ibuffer.reserve(rhs.ibuffer.capacity());
		obuffer.reserve(rhs.obuffer.capacity());
	}
	HttpClientSession *Clone() const { return new HttpClientSession(*this); }

	void OnOpen(PollIO *io)
	{
		assoc_io = io;
		client->OnOpen(sid, this);
	}
	void OnClose()
	{
		client->OnClose(sid);
		delete this;
	}
	void OnFailed()
	{
		client->OnFailed(sid);
		delete this;
	}

	void SendReady()
	{
		if(sending) return;
		sending = true;
		assoc_io->PermitSend();
	}
	void SendFinish()
	{
		Thread::Mutex::Scoped l(locker);
		sending = false;
		assoc_io->ForbidSend();
	}

	Octets& GetOBuffer() { return obuffer; }
	Octets& GetIBuffer() { return ibuffer; }

	void BeforeSend()
	{
		Thread::Mutex::Scoped l(locker);
		for(; !os.empty(); os.pop_front())
		{
			obuffer.insert(obuffer.end(), os.front().begin(), os.front().end());
		}
	}
	void AfterRecv()
	{
		is.insert(is.end(), ibuffer.begin(), ibuffer.end());
		ibuffer.clear();
		client->Dispatch(sid, is);
		assoc_io->PermitRecv();
	}

public:
	HttpClientSession(HttpClientInterface *m) : client(m),assoc_io(NULL),locker("HttpClientSession"),
		sending(false),closing(false),ibuffer(8192),obuffer(8192),sid(nextsid()) { }

	void Send(Octets &ps)
	{
		Thread::Mutex::Scoped l(locker);
		if(ps.size())
		{
			os.push_back(ps);
			SendReady();
		}
	}

	void Close()
	{
		Thread::Mutex::Scoped l(locker);
		if(closing) return;
		closing = true;
		assoc_io->PermitSend();
	}
};

class HttpClientIO : public PollIO
{
	friend class HttpConnectIO;
	HttpClientSession *session;
	void PollIn()
	{
		int recv_bytes;
		Octets& ibuf = session->GetIBuffer();
		do
		{
			if((recv_bytes = read(fd, ibuf.end(), ibuf.capacity() - ibuf.size())) > 0)
			{
				LOG_TRACE("PollIn session %d receive %d bytes", session->sid,recv_bytes);
				ibuf.setsize(ibuf.size() + recv_bytes);
				if(ibuf.size() == ibuf.capacity())
					ForbidRecv();
				session->AfterRecv();
				return;
			}
		} while(recv_bytes == -1 && errno == EINTR);
		if(recv_bytes != -1 || errno != EAGAIN)
		{
			session->BeforeSend();
			session->GetOBuffer().clear();
			if(recv_bytes == 0) session->Close();//CLOSE_ONRECV = 0x10000, //对端调用close正常关闭
			else session->Close();//CLOSE_ONRESET = 0x20000, //连接被对端reset
		}
	}
	void PollOut()
	{
		int send_bytes;
		session->BeforeSend();
		Octets& obuf = session->GetOBuffer();
		do
		{
			if((send_bytes = write(fd, obuf.begin(), obuf.size())) > 0)
			{
				obuf.erase(obuf.begin(), (char*)obuf.begin() + send_bytes);
				LOG_TRACE("PollOut session %d send %d bytes", session->sid,send_bytes);
				if(obuf.size() == 0)
					session->SendFinish();
				return;
			}
		} while(send_bytes == -1 && errno == EINTR);
		if(send_bytes != -1 || errno != EAGAIN)
		{
			obuf.clear();
			session->Close();//CLOSE_ONSEND = 0x30000, //发送时发生错误,错误存储在errno中
		}
	}
	void PollClose()
	{
		if(session->closing) Close();
	}

	HttpClientIO(int fd, HttpClientSession *s) : PollIO(fd), session(s)
	{
		session->OnOpen(this);
	}
	~HttpClientIO()
	{
		session->OnClose();
	}
};

class HttpConnectIO : public PollIO
{
	struct sockaddr sa;
	HttpClientSession *session;
	void PollIn() { Close(); }
	void PollOut() { Close(); }
public:
	HttpConnectIO(int x, const struct sockaddr &addr, const HttpClientSession &s) : PollIO(x), sa(addr), session(s.Clone())
	{
		connect(fd, &sa, sizeof(sa));
	}
	~HttpConnectIO()
	{
		int optval = -1;
		socklen_t optlen = sizeof(optval);
		int optret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen);
		if(optret == 0 && optval == 0)
		{
			PollController::Register(new HttpClientIO(dup(fd), session), true, false);
		}
		else
		{
			int rv = connect(fd, &sa, sizeof(sa));
			if(rv == 0 || (rv == -1 && errno == EISCONN))
			{
				PollController::Register(new HttpClientIO(dup(fd), session), true, false);
			}
			else
			{
				session->OnFailed();
			}
		}
	}
};

HttpConnectIO *CreateHttpLink(const HttpClientSession &session, const char *ip, int port = 80);
}
#endif
