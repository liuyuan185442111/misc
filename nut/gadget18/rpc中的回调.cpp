#include <cstdio>

struct HookBase
{
	virtual void Process()=0;
	virtual ~HookBase()=0;
};
HookBase::~HookBase(){}

class Hook : public HookBase
{
	//some data
public:
	virtual void Process()
	{
		puts("in Hook Process");
		delete this;
	}
};

struct rpc
{
	HookBase *hook;
	rpc(HookBase *h) : hook(h){}
	void client()
	{
		hook->Process();
	}
};

int main()
{
	rpc r(new Hook());
	r.client();
    return 0;
}
