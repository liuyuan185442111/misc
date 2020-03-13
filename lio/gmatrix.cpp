#include "gmatrix.h"
#include "message.h"
#include <stdio.h>

namespace gmatrix
{
MsgQueueList msglist;
void HandleMessage(const MSG& msg)
{
	//根据target和type找相应object的message_handler去处理消息
	printf("HandleMessage:type=%d, from=%d, target=%d, param=%d\n", msg.type, msg.from, msg.target, msg.param);
}
}


#include <iostream>
#include "httppool.h"
using namespace std;
using namespace GNET;

class TestTimer : public IntervalTimer::Observer
{
	virtual bool Update()
	{
		puts("in TestTimer");
		HttpSingletonClient::GetInstance()->SendMessage("10010:796504");
		puts("in TestTimer end");
		return true;
	}
};
class TestTask : public Thread::Runnable
{
	int value;
	virtual void Run()
	{
		printf("in TestTask %d\n", value);
		delete this;
	}
public:
	TestTask(int i) : value(i) {}
};

int main(int argc, char **argv)
{
#if defined(__x86_64__)
	puts("hello __x86_64__");
#elif defined(__i386__)
	puts("hello __i386__");
#endif

	IntervalTimer::StartTimer(500000);//500ms per tick
	IntervalTimer::Attach(&gmatrix::msglist, 1);
	TestTimer s;
	IntervalTimer::Attach(&s, 2);

	HttpSingletonClient::GetInstance()->Prepare("http://common.sms.wanmei.com/web/api/message/send");
	cout << endl << endl;

	Thread::AddTask(PollIO::GetPollIOTask());
	Thread::Start();
	for(int i=0; i<100; ++i)
	{
		//Thread::AddTask(new TestTask(i));
	}
	while(true)
	{
		static int mask = 100000;
		char buf[10];
		sprintf(buf, "%d", ++mask);
		puts("before send in main");
		HttpSingletonClient::GetInstance()->SendMessage(string("10086:")+buf);
		puts("after send in main");

		static int type = 0;
		gmatrix::msglist.AddMsg(MSG(++type));
		sleep(2);
		puts("wake up from sleep");
	}
	return 0;
}
