#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>

using int64 = long;

namespace SkadaCpp {

class BuffCover
{
private:
	struct Elem
	{
		int active = 0;//1:有利 2:不利
		int64 addtime = 0;//上次添加时间
		int period;
		int lasttime = 0;//覆盖时间
		int covertime = 0;//覆盖时间
		int coverage;//覆盖率，万分数
	};
	struct PoolKeyHash
	{
		size_t operator()(const std::pair<int64, int> &val) const
		{
			return std::hash<int64>()(val.first) + std::hash<int>()(val.second);
		}
	};
	std::unordered_map<std::pair<int64, int>, Elem, PoolKeyHash> pool;

	struct BuffSetElem
	{
		int active = 0;
		std::unordered_set<int64> roles;
		std::vector<std::pair<int, int64>> sorted_roles;//按覆盖率排序的roles
	};
	std::unordered_map<int, BuffSetElem> buff_set;

	struct BuffSortElem
	{
		int buffid;
		int count;
		BuffSortElem(int b, int c) : buffid(b), count(c) {}
	};
	std::vector<BuffSortElem> buff_sort;//按数量排序的buffs，由buff_set得来
	std::vector<BuffSortElem> debuff_sort;//按数量排序的debuffs，由buff_set得来

	struct RoleSetElem
	{
		std::unordered_set<int> buffs;
		std::vector<std::pair<int, int>> sorted_buffs;//按覆盖率排序的增益
		std::vector<std::pair<int, int>> sorted_debuffs;//按覆盖率排序的减益
	};
	std::unordered_map<int64, RoleSetElem> role_set;

	struct RoleSortElem
	{
		int64 roleid;
		int avgcoverage;//平均覆盖率
		int buffcount;//buff数量
		RoleSortElem(int64 r, int a, int b) : roleid(r), avgcoverage(a), buffcount(b) {}
	};
	std::vector<RoleSortElem> role_sort1;//按buff平均覆盖率排序的roles，由role_set得来
	std::vector<RoleSortElem> role_sort2;//按debuff平均覆盖率排序的roles，由role_set得来

	bool OK = false;
	int64 starttime = 0;

public:
	void begin(int64 nowtime) { starttime = nowtime; }
	void finish() { calbuff(true); }
	void addbuff(int64 roleid, int buffid, int64 addtime, int period);
	void delbuff(int64 roleid, int buffid);
	bool calbuff(bool finish);
	BuffCover& operator+=(const BuffCover &cover);
};

struct Battle
{
	BuffCover _buff;
	void begin(int64 nowtime) { _buff.begin(nowtime); }
	void finish() { _buff.finish(); }
};

}
