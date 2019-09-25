#include <iostream>
#include "pcre_matcher.h"
using namespace std;
int main()
{
	pcre_matcher *matcher;
	cout << "create " << pm_strerror(pm_create(&matcher, "UTF8", NULL, NULL)) << endl;
	cout << "load " << pm_strerror(pm_load(matcher, "role.txt", NULL)) << endl;
	cout << pm_strerror(pm_match(matcher, "aaa12428123", 11, "ASCII")) << endl;
	cout << pm_strerror(pm_match(matcher, "我们", 4, "UTF8")) << endl;
	cout << pm_strerror(pm_match(matcher, "223", 3, "ASCII")) << endl;
	cout << pm_strerror(pm_match(matcher, "t 4t", 4, "ASCII")) << endl;
	pm_remove(matcher);
	return 0;
}

