/**string.h中的strtok线程不安全, 无法同时解析两个不同的字符串, 会修改传入的字符串.
 * 这里以strtok为蓝本写一个新的函数SplitString.
 */
#include <cstring>
#include "split.h"
namespace lcore {
void SplitString(std::vector<std::string> &strs, const char *str, const char *token)
{
	if(!strs.empty()) strs.clear();
	const char *send = str;
	while(1)
	{
		str += strspn(send, token);
		send = str + strcspn(str, token);
		strs.push_back(std::string(str, send));
		if(!*send) break;
		str = ++send;
	}
}
}
