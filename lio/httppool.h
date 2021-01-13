#include "httpclient.h"
#include "thread.h"
#include "mutex.h"
using namespace GNET;
class HttpSingletonClient : public HttpClientInterface
{
	std::string uri;
	std::string host;
	std::string ip;
	int connecting_link;
	bool prepared;

	Thread::Mutex pool_locker;
	std::map<int, HttpClientSession*> link_pool;
	std::deque<std::string> request_pool;

	virtual void OnOpen(int sid, HttpClientSession *session)
	{
		LOG_TRACE("session %d connected", sid);
		Thread::Mutex::Scoped l(pool_locker);
		--connecting_link;
		link_pool.insert(std::make_pair(sid, session));
		TrySend();
	}
	virtual void OnClose(int sid)
	{
		LOG_TRACE("session %d close", sid);
		Thread::Mutex::Scoped l(pool_locker);
		link_pool.erase(sid);
		TrySend();
	}
	virtual void OnFailed(int sid)
	{
		LOG_TRACE("session %d failed", sid);
		Thread::Mutex::Scoped l(pool_locker);
		--connecting_link;
		TrySend();
	}
	virtual void Dispatch(int sid, OctetsStream &is)
	{
		size_t s = is.size();
		char *p = new char[s+1];
		p[s] = '\0';
		is.pop_byte(p, s);
		LOG_TRACE("session %d content is:\n%s\n", sid,p);
		delete[] p;
		is.clear();
		Thread::Mutex::Scoped l(pool_locker);
		TrySend();
	}

	HttpConnectIO *Create()
	{
		HttpConnectIO *dummy = CreateHttpLink(HttpClientSession(GetInstance()), ip.c_str());
		if(dummy) ++connecting_link;
		return dummy;
	}
	bool Encode(std::string &code, const std::string &message)
	{
		size_t delimiter = message.find(':');
		if(delimiter == std::string::npos || delimiter == 0 || delimiter == message.size()-1) return false;
		std::string phone(message, 0, delimiter);
		std::string mask(message, delimiter+1);

		char buf[24];
		sprintf(buf, "%d", (int)time(NULL));
		std::map<std::string, std::string> param;
		param.insert(std::make_pair("appId","5"));
		param.insert(std::make_pair("clientId","meijie_zx3_fill"));
		param.insert(std::make_pair("mobile",phone));
		param.insert(std::make_pair("content",mask));
		param.insert(std::make_pair("timestamp",buf));
		std::string sign;
		std::map<std::string, std::string>::iterator it = param.begin();
		for(; it != param.end(); ++it)
		{
			sign += (it->second + ",");
		}
		sign.replace(sign.end()-1, sign.end(), "3a0f0390f92743a797d7ae5ea71cf981");
		LOG_TRACE("sign: %s", sign.c_str());

		char md5char[5+32] = "sign=";
		static const char hex[] = "0123456789abcdef";
		unsigned char digest[16];
		//md5_digest(digest, sign.c_str(), sign.size());
		for (size_t i = 0; i < sizeof(digest); ++i)
		{
			md5char[5+i+i] = hex[(digest[i] >> 4) & 0x0f];
			md5char[5+i+i+1] = hex[digest[i] & 0x0f];
		}

		for(it = param.begin(); it != param.end(); ++it)
		{
			code += (it->first + "=" + it->second + "&");
		}
		code.append(md5char, sizeof(md5char));
		return true;
	}
	bool DoSend(HttpClientSession *link, const std::string &message)
	{
		if(link == NULL) return false;
		std::string code;
		if(!Encode(code, message)) return false;
		std::stringstream ss;
		ss << "POST " << uri << " HTTP/1.1\r\nHost: " << host \
			<< "\r\nContent-Type: application/x-www-form-urlencoded\r\nConnection: Keep-Alive\r\nContent-length: "\
			<< message.length() << "\r\n\r\n" << message;
		Octets ps(ss.str().c_str(), strlen(ss.str().c_str()));
		link->Send(ps);
		return true;
	}
	void TrySend()
	{
		if(!request_pool.empty())
		{
			if(link_pool.empty())
			{
				if(connecting_link < 3) Create();
			}
			else
			{
				//考虑随机选取一个连接
				DoSend(link_pool.begin()->second, request_pool.front());
				request_pool.pop_front();
			}
		}
	}
	HttpSingletonClient() : connecting_link(0), prepared(false), pool_locker("HttpClient") { }
	HttpSingletonClient(const HttpSingletonClient&);
public:
	static HttpSingletonClient *GetInstance()
	{
		static HttpSingletonClient instance;
		return &instance;
	}
	bool Prepare(const char *url)
	{
		if(strlen(url) < 8) return false;
		if(strncmp(url, "http://", 7) != 0) return false;
		url += 7;
		const char *c = strchr(url, '/');
		if(c == NULL)
		{
			std::string tmp(url);
			host.swap(tmp);
			uri = std::string("/");
		}
		else
		{
			std::string tmp1(url, c-url);
			host.swap(tmp1);
			std::string tmp2(c);
			uri.swap(tmp2);
		}
		LOG_TRACE("preparing enter gethostbyname");
		struct hostent *hptr = gethostbyname(host.c_str());//可能会阻塞很久
		LOG_TRACE("preparing leave gethostbyname");
		if(hptr && hptr->h_addrtype == AF_INET)
		{
			char str[32];
			memset(str, 0, sizeof(str));
			inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str));
			std::string tmp(str);
			ip.swap(tmp);
		}
		else
		{
			return false;
		}
		LOG_TRACE("preparing host=%s ip=%s uri=%s",host.c_str(),ip.c_str(),uri.c_str());
		prepared = true;
		return true;
	}
	bool SendMessage(const std::string &message)
	{
		if(!prepared) return false;
		if(message.empty()) return false;
		LOG_TRACE("connecting=%d request_pool=%d link_pool=%d message=%s", connecting_link,request_pool.size(),link_pool.size(),message.c_str());
		{
			Thread::Mutex::Scoped l(pool_locker);
			if(request_pool.size() > 999)
			{
				TrySend();
				return false;
			}
			if(link_pool.empty())
			{
				if(connecting_link > 2 || Create())
				{
					request_pool.push_back(message);
					return true;
				}
				return false;
			}
			request_pool.push_back(message);
			TrySend();
		}
		return true;
	}
	void Close()
	{
		Thread::Mutex::Scoped l(pool_locker);
		for(std::map<int, HttpClientSession*>::iterator it = link_pool.begin(); it != link_pool.end(); ++it)
		{
			if(it->second) it->second->Close();
		}
	}
};
