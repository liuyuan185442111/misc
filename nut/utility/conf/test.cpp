#include <iostream>
#include "conf.h"
using namespace std;
using namespace lcore;
int main()
{
	Conf::load("test.conf");
	cout << Conf::find("general", "port") << endl;
	cout << Conf::find("thread", "count") << endl;
	Conf()["thread"]["count"] = "9999";
	cout << Conf::find("thread", "count") << endl;
	cout << Conf::find("thread", "time") << endl;
	Conf()["thread"]["time"] = "2018";
	cout << Conf::find("thread", "time") << endl;
	return 0;
}
