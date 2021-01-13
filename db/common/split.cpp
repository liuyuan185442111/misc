/**
 * <string.h>中的strtok会修改传入的字符串, 线程不安全, 无法同时解析两个不同的字符串.
 * 这里以strtok为蓝本写一个新的函数SplitString.
 */
#include <cstring>
#include <string>
#include <vector>
namespace lcore {
void SplitString(std::vector<std::string> &strs, const char *str, const char *token)
{
	strs.clear();
    for(const char *send = str;;)
	{
		str += strspn(send, token);
		send = str + strcspn(str, token);
		strs.push_back(std::string(str, send));
		if(!*send) break;
		str = ++send;
	}
}
}
