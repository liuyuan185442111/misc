#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>

using int64 = long;

//不要在std里写东西了
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

namespace SkadaCpp {

class BuffCover
{
private:
	struct elem
	{
		int active = 0;//1:有益 2:有害
		int64 addtime = 0;//上次添加时间
		int period;
		int covertime = 0;//覆盖时间
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
		role_sort_content(int64 a, int b, int c) : roleid(a), coverage(b), count(c) {}
	};
	std::vector<role_sort_content> role_sort1;//按平均覆盖率排序的roles，由role_set得来
	std::vector<role_sort_content> role_sort2;//按平均覆盖率排序的roles，由role_set得来

	bool OK = false;
	int64 starttime = 0;

public:
	void addbuff(int64 roleid, int buffid, int64 addtime, int period);
	void delbuff(int64 roleid, int buffid);
	bool calbuff(bool finish);
	BuffCover& operator+=(const BuffCover &cover);
	//一个名字和职业的缓存
};

struct Battle
{
	BuffCover _buff;
	void finish()
	{
		_buff.calbuff(true);
	}
};

}
