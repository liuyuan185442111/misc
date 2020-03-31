#include "buff.h"

class SkadaManager
{
private:
	int64 currbattle = 0;
	std::unordered_map<int64, SkadaCpp::Battle> allbattle; //0是总计

public:
	void begin_battle(int64 battle)
	{
		currbattle = battle;
		allbattle[currbattle].begin(battle);
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
	auto &elem = pool[std::make_pair(roleid, buffid)];
	if(elem.addtime > 0) //未del便add
	{
		int64 nowtime = time(nullptr);
		elem.lasttime = elem.lasttime + std::min(nowtime - elem.addtime, (int64)elem.period);
	}
	elem.addtime = addtime;
	elem.period = period;
	buff_set[buffid].roles.insert(roleid);
	role_set[roleid].buffs.insert(buffid);
}

void BuffCover::delbuff(int64 roleid, int buffid)
{
	auto iter = pool.find(std::make_pair(roleid, buffid));
	if(iter == pool.end())
	{
		//TODO 此时也可能是进入战斗后没收到addbuff，考虑使用buff的period而不是直接用starttime
		addbuff(roleid, buffid, starttime, 0);
		delbuff(roleid, buffid);
	}
	else
	{
		auto &elem = iter->second;
		if(elem.addtime > 0)
		{
			int64 nowtime = time(nullptr);
			elem.lasttime = elem.lasttime + nowtime - elem.addtime;
			elem.addtime = 0;
		}
		else
		{
			//TODO 可能未收到addbuff，考虑使用该buff的period计算时长
		}
	}
}

bool BuffCover::calbuff(bool finish)
{
	if(OK) return false;
	if(finish) OK = true;

	for(auto &e : pool)
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
		else
		{
			elem.covertime = elem.lasttime;
		}
		elem.coverage = elem.covertime / 1;
		if(finish)
		{
			elem.addtime = 0;
			elem.lasttime = 0;
			elem.period = 0;
		}
	}

	buff_sort.clear();
	debuff_sort.clear();
	for(auto &e : buff_set)
	{
		if(e.second.active == 0)
		{
			e.second.active = 1;
		}
		int buffid = e.first;
		auto &roles = e.second.roles;
		if(e.second.active == 1)
		{
			buff_sort.emplace_back(buffid, (int)roles.size());
		}
		else if(e.second.active == 2)
		{
			debuff_sort.emplace_back(buffid, (int)roles.size());
		}
		auto &sorted_roles = e.second.sorted_roles;
		sorted_roles.clear();
		sorted_roles.reserve(roles.size());
		auto key = std::make_pair((int64)0, buffid);
		for(auto roleid : roles)
		{
			key.first = roleid;
			sorted_roles.emplace_back(pool[key].coverage, roleid);
		}
		using T = typename decltype(e.second.sorted_roles)::const_reference;
		std::sort(sorted_roles.begin(), sorted_roles.end(), [](T a, T b){return a.first>b.first;});
	}
	{
		using T = typename decltype(buff_sort)::const_reference;
		std::sort(buff_sort.begin(), buff_sort.end(), [](T a, T b){return a.count>b.count;});
		std::sort(debuff_sort.begin(), debuff_sort.end(), [](T a, T b){return a.count>b.count;});
	}

	role_sort1.clear();
	role_sort2.clear();
	for(auto &e : role_set)
	{
		int64 roleid = e.first;
		auto &sorted_buffs = e.second.sorted_buffs;
		auto &sorted_debuffs = e.second.sorted_debuffs;
		auto key = std::make_pair(roleid, 0); 
		int count1 = 0, count2 = 0;
		int coverage1 = 0, coverage2 = 0;
		for(auto &buffid : e.second.buffs)
		{
			key.second = buffid;
			const auto &elem = pool[key];
			if(elem.active == 1)
			{
				++count1;
				coverage1 += elem.coverage;
				sorted_buffs.emplace_back(elem.coverage, buffid);
			}
			else if(elem.active == 2)
			{
				++count2;
				coverage2 += elem.coverage;
				sorted_debuffs.emplace_back(elem.coverage, buffid);
			}
		}
		using T = typename decltype(e.second.sorted_buffs)::const_reference;
		std::sort(sorted_buffs.begin(), sorted_buffs.end(), [](T a, T b){return a.first>b.first;});
		std::sort(sorted_debuffs.begin(), sorted_debuffs.end(), [](T a, T b){return a.first>b.first;});
		role_sort1.emplace_back(roleid, coverage1, count1);
		role_sort2.emplace_back(roleid, coverage2, count2);
	}
	{
		using T = typename decltype(role_sort1)::const_reference;
		std::sort(role_sort1.begin(), role_sort1.end(), [](T a, T b){return a.avgcoverage>b.avgcoverage;});
		std::sort(role_sort2.begin(), role_sort2.end(), [](T a, T b){return a.avgcoverage>b.avgcoverage;});
	}
	return true;
}

BuffCover& BuffCover::operator+=(const BuffCover &cover)
{
	for(const auto &e : cover.pool)
	{
		pool[e.first].covertime += e.second.covertime;
	}
	for(const auto &e : cover.buff_set)
	{
		auto &roles = buff_set[e.first].roles;
		for(auto role : e.second.roles)
			roles.insert(role);
	}
	for(const auto &e : cover.role_set)
	{
		auto &buffs = role_set[e.first].buffs;
		for(auto buff : e.second.buffs)
			buffs.insert(buff);
	}
	starttime = 0;
	OK = false;
	return *this;
}

}
