#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
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
		int64 addtime = 0;//上次添加时间
		int period;
		int covertime;//覆盖时间
		int lasttime = 0;//覆盖时间
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
		buff_sort_content(int a, int b) : buffid(a), count(b) {}
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
	int64 starttime = 0;

	void addbuff(int64 roleid, int buffid, int64 addtime, int period)
	{
		bool active = true;
		auto &elem = pools[std::make_pair(roleid, buffid)];
		if(elem.addtime > 0) //未del便add
		{
			int64 nowtime;
			elem.covertime = elem.lasttime + std::min(nowtime - elem.addtime, (int64)elem.period);
		}
		elem.active = active;
		elem.addtime = addtime;
		elem.period = period;
		buff_set[buffid].active = active;
		buff_set[buffid].roles.insert(roleid);
		role_set[roleid].buffs.insert(buffid);
	}
	void delbuff(int64 roleid, int buffid, bool finish)
	{
		auto iter = pools.find(std::make_pair(roleid, buffid));
		if(iter == pools.end())
		{
			addbuff(roleid, buffid, starttime, 0);
			delbuff(roleid, buffid, finish);
		}
		else
		{
			auto &elem = iter->second;
			if(elem.addtime > 0)
			{
				int64 nowtime;
				if(finish)
					elem.covertime = elem.lasttime + std::min(nowtime - elem.addtime, (int64)elem.period);
				else
					elem.covertime = elem.lasttime + nowtime - elem.addtime;
				elem.addtime = 0;
			}
		}
	}
	//nowtime为0表示最终的计算
	bool calbuff(int64 nowtime)
	{
		OK = true;
		for(auto &e : pools)
		{
			auto &elem = e.second;
			if(elem.addtime > 0)
			{
				int64 nowtime;
				elem.covertime = elem.lasttime + std::min(nowtime - elem.addtime, (int64)elem.period);
			}
			elem.coverage = elem.covertime / 1;
		}

		for(auto &e : buff_set)
		{
			auto &roles = e.second.roles;
			auto &sorted_roles = e.second.sorted_roles;
			if(e.second.active)
			{
				buff_sort.emplace_back(e.first, (int)roles.size());
			}
			else
			{
				debuff_sort.emplace_back(e.first, (int)roles.size());
			}
			sorted_roles.clear();
			sorted_roles.reserve(roles.size());
			auto key = std::make_pair((int64)0, e.first);
			for(auto f : roles)
			{
				key.first = f;
				sorted_roles.push_back(std::make_pair(pools[key].coverage, f));
			}
			using T = typename remove_reference<decltype(sorted_roles)>::type::const_reference;
			std::sort(sorted_roles.begin(), sorted_roles.end(), [](T a, T b){return a.first>b.first;});
		}
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
