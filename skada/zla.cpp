#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
using namespace std;

using int64 = long;

namespace std
{
template <>
struct hash<pair<int64, int>>
{
	size_t operator()(pair<int64, int> val) const
	{
		return hash<int64>()(val.first) + hash<int>()(val.second);
	}
};
}

struct BuffCover
{
	struct elem
	{
		bool active;
		int64 lastaddtime;//上次添加时间
		int period;
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

	bool OK = false;

	void addbuff(int64 roleid, int buffid, int period)
	{
	}
	void delbuff(int64 roleid, int buffid)
	{
	}
	bool calbuff()
	{
		OK = true;
	}
	//一个名字和职业的缓存
	//数据的导入导出
};

struct Battle
{
	BuffCover _buff;
};

struct UZCSkadaManager
{
	//0放当前 1放总计
	std::unordered_map<int64, Battle> allbattle;

	void finish_curr_battle()
	{
	}
	void begin_curr_battle()
	{
	}
	void remove_battle()
	{
	}

	void addbuff(int64 roleid, int buffid, int period)
	{
	}
	void delbuff(int64 roleid, int buffid)
	{
	}
	//todo 查询接口

	//数据的导入导出
};

int main()
{
	return 0;
}
