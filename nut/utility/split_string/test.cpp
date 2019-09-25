#include <iostream>
#include <iterator>
#include "split.h"
using namespace std;
using namespace lcore;

int main()
{
    vector<string> s;
	SplitString(s, "+192.+168.+5.+++++6-235-+.324","-+.");
    copy(s.begin(), s.end(), ostream_iterator<string>(cout, " "));
    return 0;
}
