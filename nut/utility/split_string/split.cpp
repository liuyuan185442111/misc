/**string.h�е�strtok�̲߳���ȫ, �޷�ͬʱ����������ͬ���ַ���, ���޸Ĵ�����ַ���.
 * ������strtokΪ����дһ���µĺ���SplitString.
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
