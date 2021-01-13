#include "httpclient.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>

namespace GNET {
void LOG_TRACE(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stdout,format,args);
	va_end(args);
	puts("");
}
bool get_local_ip(std::vector<std::string> &ips)
{
    int sockfd;
    if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
        return false;
    ips.clear();
    struct ifconf ifconf;
    char buf[512];
    ifconf.ifc_len = sizeof(buf);
    ifconf.ifc_buf = buf;
    ioctl(sockfd, SIOCGIFCONF, &ifconf); //获取所有接口信息

    //一个一个的获取IP地址
    struct ifreq *req = (struct ifreq*)ifconf.ifc_buf;
    for(int i = (ifconf.ifc_len/sizeof(struct ifreq)); i>0; --i)
    {
        if(req->ifr_flags == AF_INET) //for ipv4
		{
            //printf("name=[%s]\n", req->ifr_name);
            //printf("local_addr=[%s]\n", inet_ntoa(((struct sockaddr_in*)&(req->ifr_addr))->sin_addr));
			ips.push_back(inet_ntoa(((struct sockaddr_in*)&(req->ifr_addr))->sin_addr));
            ++req;
        }
    }
    return !ips.empty();
}
HttpConnectIO *CreateHttpLink(const HttpClientSession &session, const char *ip, int port)
{
	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(s < 0) return NULL;

	struct sockaddr addr;
	memset(&addr, 0, sizeof(addr));
	struct sockaddr_in *pad = (struct sockaddr_in *)&addr;
	pad->sin_family = AF_INET;
	pad->sin_addr.s_addr = inet_addr(ip);
	pad->sin_port = htons(port);

	int optval = 1;
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
	optval = 1;
	setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));

	return (HttpConnectIO*)PollController::Register(new HttpConnectIO(s, addr, session), true, true);
}
}
