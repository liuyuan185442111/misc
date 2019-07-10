/*
    ����һ��ʾ�����ʵ�libevent�ĳ��򣬼�����TCP��9995�˿ڡ�
    �����ӽ����ɹ����������Client��Ӧһ����Ϣ"Hello, World!\n"
    ������Ϻ�ͽ����ӹرա�

    ����Ҳ������SIGINT (ctrl-c)�źţ��յ�����źź������˳�����

    �������Ҳ�õ���һЩlibevent�Ƚϸ߼���API����bufferevent��
    ����API��buffer�ġ�ˮλ�ߡ�Ҳ�������event���������Ӧ��������
    Windowsƽ̨��IOCP��
*/

// ���볣��Linuxϵͳͷ�ļ� 
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// ����libevent 2.x��ص�ͷ�ļ� 
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>

// �����ַ��������������Ӧ��Client�� 
static const char MESSAGE[] = "Hello, World!\n";

// server�����Ķ˿� 
static const int PORT = 9995;

// ���弸��event callback��prototype��ԭ�ͣ� 
static void listener_cb(struct evconnlistener * , evutil_socket_t,
    struct sockaddr * , int socklen, void * );
static void conn_writecb(struct bufferevent * , void * );
static void conn_eventcb(struct bufferevent * , short, void * );
static void signal_cb(evutil_socket_t, short, void * );

// �����׼��main���� 
int
main(int argc, char ** argv)
{
    // event_base������eventѭ����Ҫ�Ľṹ�� 
    struct event_base * base;
    // libevent�ĸ߼�APIרΪ������FDʹ�� 
    struct evconnlistener * listener;
    // �źŴ���eventָ�� 
    struct event * signal_event;
    // ���������ַ�Ͷ˿ڵĽṹ�� 
    struct sockaddr_in sin;

    // ���䲢��ʼ��event_base 
    base = event_base_new();
    if (!base) {
        // ��������κδ�����stderr����׼�����������һ����־���˳� 
        // ��C������ܶ෵��ָ���API���Է���nullΪ����ķ���ֵ 
        // if (!base) �ȼ��� if (base == null) 
        fprintf(stderr, "Could not initialize libevent!\n");
        return 1;
    }

    // ��ʼ��sockaddr_in�ṹ�壬������0.0.0.0:9995 
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);

    // bind�������ƶ���IP�Ͷ˿ڣ�ͬʱ��ʼ��listen���¼�ѭ����callback��listener_cb 
    // ����listener���¼�ѭ��ע����event_base��base�� 
    listener = evconnlistener_new_bind(base, listener_cb, (void * )base,
        LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1,
        (struct sockaddr*)&sin,
        sizeof(sin));

    if (!listener) {
        // ��������κδ�����stderr����׼�����������һ����־���˳� 
        fprintf(stderr, "Could not create a listener!\n");
        return 1;
    }

    // ��ʼ���źŴ���event 
    signal_event = evsignal_new(base, SIGINT, signal_cb, (void * )base);

    // �����callback����base�� 
    if (!signal_event || event_add(signal_event, NULL)<0) {
        fprintf(stderr, "Could not create/add a signal event!\n");
        return 1;
    }

    // ������������һ��������eventѭ����ֻ���ڵ���event_base_loopexit�� 
    // �Ż����������������أ�������ִ�и������������������������˳� 
    event_base_dispatch(base);

    // ��������free 
    evconnlistener_free(listener);
    event_free(signal_event);
    event_base_free(base);

    printf("done\n");
    return 0;
}

// �����˿ڵ�event callback 
static void
listener_cb(struct evconnlistener * listener, evutil_socket_t fd,
    struct sockaddr * sa, int socklen, void * user_data)
{
    struct event_base * base = user_data;
    struct bufferevent * bev;

    // �½�һ��bufferevent���趨BEV_OPT_CLOSE_ON_FREE�� 
    // ��֤bufferevent��free��ʱ��fdҲ�ᱻ�ر� 
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(base);
        return;
    }
    // �趨дbuffer��event������event 
    bufferevent_setcb(bev, NULL, conn_writecb, conn_eventcb, NULL);
    // ������fd��д��event 
    bufferevent_enable(bev, EV_WRITE);
    // �رմ�fd�ж�д��buffer��event 
    bufferevent_disable(bev, EV_READ);
    // ��buffer��д��"Hello, World!\n" 
    // ����Ĳ�����֤��fd��дʱ����buffer�е�����д��ȥ 
    bufferevent_write(bev, MESSAGE, strlen(MESSAGE));
}


// ÿ��fd��д�����ݷ�����д��󣬻��Ҳ�ѹ�conn_writecb 
// �������ÿ�μ��eventbuffer��ʣ���С�����Ϊ0 
// ��ʾ�����Ѿ�ȫ��д�꣬��eventbuffer free�� 
// �����������趨��BEV_OPT_CLOSE_ON_FREE������fdҲ�ᱻ�ر� 
static void
conn_writecb(struct bufferevent * bev, void * user_data)
{
    struct evbuffer * output = bufferevent_get_output(bev);
    if (evbuffer_get_length(output) == 0) {
        printf("flushed answer\n");
        bufferevent_free(bev);
    }
}

// �������дevent֮���event��callback 
static void
conn_eventcb(struct bufferevent * bev, short events, void * user_data)
{
    if (events & BEV_EVENT_EOF) {
        // Client�˹ر����� 
        printf("Connection closed.\n");
    } else if (events & BEV_EVENT_ERROR) {
        // ���ӳ��� 
        printf("Got an error on the connection: %s\n",
            strerror(errno));
    }
    // �������������eventû�д����Ǿ͹ر����bufferevent 
    bufferevent_free(bev);
}

// �źŴ���event���յ�SIGINT (ctrl-c)�źź��ӳ�2s�˳�eventѭ�� 
static void
signal_cb(evutil_socket_t sig, short events, void * user_data)
{
    struct event_base * base = user_data;
    struct timeval delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

    event_base_loopexit(base, &delay);
}