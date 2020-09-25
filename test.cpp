#include <iostream>
using namespace std;

class Base
{
public:
    virtual ~Base()
    {
        cout <<  "~Base()" << endl;
    }
    virtual void fun()
    {
        cout <<  "Base::fun()"  << endl;
    }
};

class Derived :  public Base
{
public:
    ~Derived()
    {
        cout <<  "~Derived()" << endl;
    }
    void fun()
    {
        cout <<  "Derived::fun()"  << endl;
    }
};

int main()
{
    Derived *dp =  new Derived;
    Base *p = dp;
    p->fun();
    cout <<  sizeof(Base) << endl;
    cout <<  sizeof(Derived) << endl;
    cout << ( void *)dp << endl;
    cout << ( void *)p << endl;
    delete p;
    p =  NULL;

    return  0;
}
