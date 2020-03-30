#include "skada.h"

struct SkadaManager
{
	int64 currbattle = 0;
	std::unordered_map<int64, SkadaCpp::Battle> allbattle; //0是总计

	void begin_battle(int64 battle)
	{
		currbattle = battle;
	}
	void finish_battle()
	{
		allbattle[currbattle].finish();
		currbattle = 0;
	}
	void remove_battle(int64 battle)
	{
		allbattle.erase(battle);
		allbattle.erase(0);
	}

	void addbuff(int64 roleid, int buffid, int64 addtime, int period)
	{
		if(currbattle == 0) return;
		allbattle[currbattle]._buff.addbuff(roleid, buffid, addtime, period);
	}
	void delbuff(int64 roleid, int buffid)
	{
		if(currbattle == 0) return;
		allbattle[currbattle]._buff.delbuff(roleid, buffid);
	}
	//查询接口
	//数据的导入导出
};

int main()
{
	SkadaManager mgr;
	mgr.begin_battle(time(nullptr));
	mgr.addbuff(101, 1, 100, 2);
	mgr.delbuff(101, 1);
	mgr.finish_battle();
	return 0;
}

namespace SkadaCpp {

void BuffCover::addbuff(int64 roleid, int buffid, int64 addtime, int period)
{
	auto &elem = pools[std::make_pair(roleid, buffid)];
	if(elem.addtime > 0) //未del便add
	{
		int64 nowtime = time(nullptr);
		elem.covertime = elem.lasttime + std::min(nowtime - elem.addtime, (int64)elem.period);
	}
	elem.addtime = addtime;
	elem.period = period;
	buff_set[buffid].roles.insert(roleid);
	role_set[roleid].buffs.insert(buffid);
}

void BuffCover::delbuff(int64 roleid, int buffid)
{
	auto iter = pools.find(std::make_pair(roleid, buffid));
	if(iter == pools.end())
	{
		addbuff(roleid, buffid, starttime, 0);
		delbuff(roleid, buffid);
	}
	else
	{
		auto &elem = iter->second;
		if(elem.addtime > 0)
		{
			int64 nowtime = time(nullptr);
			elem.covertime = elem.lasttime + nowtime - elem.addtime;
			elem.addtime = 0;
		}
	}
}

bool BuffCover::calbuff(bool finish)
{
	if(OK) return false;
	if(finish) OK = true;
	for(auto &e : pools)
	{
		auto &elem = e.second;
		if(elem.active == 0)
		{
			elem.active = 1;
		}
		if(elem.addtime > 0)
		{
			int64 nowtime = time(nullptr);
			elem.covertime = elem.lasttime + std::min(nowtime - elem.addtime, (int64)elem.period);
		}
		if(finish)
		{
			elem.addtime = 0;
			elem.lasttime = 0;
		}
		elem.coverage = elem.covertime / 1;
	}

	for(auto &e : buff_set)
	{
		auto &roles = e.second.roles;
		auto &sorted_roles = e.second.sorted_roles;
		if(e.second.active == 1)
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
		using T = typename std::remove_reference<decltype(sorted_roles)>::type::const_reference;
		std::sort(sorted_roles.begin(), sorted_roles.end(), [](T a, T b){return a.first>b.first;});
	}
	using T = typename std::remove_reference<decltype(buff_sort)>::type::const_reference;
	std::sort(buff_sort.begin(), buff_sort.end(), [](T a, T b){return a.count>b.count;});
	std::sort(debuff_sort.begin(), debuff_sort.end(), [](T a, T b){return a.count>b.count;});

	for(auto &e : role_set)
	{
		auto &content = e.second;
		auto key = std::make_pair(e.first, 0); 
		int count1 = 0, count2 = 0;
		int coverage1 = 0, coverage2 = 0;
		for(auto &f : content.buffs)
		{
			key.second = f;
			const auto &elem = pools[key];
			if(elem.active == 1)
			{
				++count1;
				coverage1 += elem.coverage;
				content.sorted_buffs.push_back(std::make_pair(elem.coverage, f));
			}
			else
			{
				++count2;
				coverage2 += elem.coverage;
				content.sorted_debuffs.push_back(std::make_pair(elem.coverage, f));
			}
		}
		using T = typename decltype(content.sorted_buffs)::const_reference;
		std::sort(content.sorted_buffs.begin(), content.sorted_buffs.end(), [](T a, T b){return a.first>b.first;});
		std::sort(content.sorted_debuffs.begin(), content.sorted_debuffs.end(), [](T a, T b){return a.first>b.first;});
		role_sort1.clear();
		role_sort1.emplace_back(e.first, coverage1, count1);
		role_sort2.clear();
		role_sort2.emplace_back(e.first, coverage2, count2);
	}
	using U = typename decltype(role_sort1)::const_reference;
	std::sort(role_sort1.begin(), role_sort1.end(), [](U a, U b){return a.coverage>b.coverage;});
	std::sort(role_sort2.begin(), role_sort2.end(), [](U a, U b){return a.coverage>b.coverage;});
	return true;
}

BuffCover& BuffCover::operator+=(const BuffCover &cover)
{
	for(const auto &e : cover.pools)
	{
		pools[e.first].covertime += e.second.covertime;
	}
	for(const auto &e : cover.buff_set)
	{
		auto &t = buff_set[e.first];
		for(const auto &f : e.second.roles)
			t.roles.insert(f);
	}
	for(const auto &e : cover.role_set)
	{
		auto &t = role_set[e.first];
		for(const auto &f : e.second.buffs)
			t.buffs.insert(f);
	}
	return *this;
}

}
