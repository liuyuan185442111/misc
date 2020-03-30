#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
using namespace std;

using int64 = long;

namespace std
{
	template <typename T1, typename T2>
	struct hash<pair<T1, T2>>
	{
		size_t operator()(pair<T1,T2> val) const
		{
			return hash<T1>(val.first) + hash<T2>(val.second);
		}
	};
}

struct elem
{
	bool active;
	int64 lastaddtime;//上次添加时间
	int covertime;//覆盖时间
	int coverage;//覆盖率
};
std::unordered_map<std::pair<int64, int>, elem> pools;

struct buff_set_content
{
	bool active;
	std::unordered_set<int64> roles;
	std::vector<std::pair<int, int64>> sorted_roles;//按覆盖率排序的roles
};
std::unordered_map<int, buff_set_content> buff_set;
struct buff_sort_content
{
	int buffid;
	int count;
};
std::vector<buff_sort_content> buff_sort;//按数量排序的buffs，由buff_set得来
std::vector<buff_sort_content> debuff_sort;//按数量排序的debuffs，由buff_set得来

struct role_set_content
{
	std::unordered_set<int> buffs;
	std::vector<std::pair<int, int>> sorted_buffs;//按覆盖率排序的增益
	std::vector<std::pair<int, int>> sorted_debuffs;//按覆盖率排序的减益
};
std::unordered_map<int64, role_set_content> role_set;
struct role_sort_content
{
	int64 roleid;
	int coverage;//平均覆盖率
	int count;//数量
};
std::vector<role_sort_content> role_sort;//按平均覆盖率排序的roles，由role_set得来

int main()
{
	return 0;
}
