#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <set>
#include <map>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>

template <typename KeyType, typename ScoreType, typename InfoType, typename ScoreCmp=std::less<ScoreType>>
struct RankInfo
{
	KeyType key;
	ScoreType score;
	InfoType other_info;
	int last_ranking;
	RankInfo(KeyType k, ScoreType s) : key(k), score(s), last_ranking(0) { }
};

template <typename Key, int maxsize, typename Cmp = std::greater<Key>>
class TopTable
{
    Key top[maxsize];
    int size;
	Cmp comparator;
public:
    TopTable():size(0){}
    bool Push(const Key &key)
    {
        if(size < maxsize)
        {
            top[size++] = key;
            if(size == maxsize)
                make_heap(top, top+size, comparator);
            return true;
        }
		if(comparator(key, top[0]))
		{
			__adjust_heap(top, 0, maxsize, key, comparator);
			return true;
		}
		return false;
    }
	template <typename RankType>
    int Build(std::vector<RankType> &table)
    {
        if(size < maxsize)
            sort(top, top+size, comparator);
        else
            sort_heap(top, top+size, comparator);
		table.reserve(size);
		for(int i=0;i<size;++i)
			table.push_back(RankType(top[i].second, top[i].first));
		return size;
    }
};

typedef int RoleKeyType;

struct RoleInfo
{
	std::string name;
};
std::map<RoleKeyType, RoleInfo> role_map;
void collect_role(RoleKeyType roleid)
{
	role_map.insert(std::make_pair(roleid, RoleInfo()));
}
template <typename RankType>
void collect_roles(std::vector<RankType> &table)
{
	for(auto &i:table)
		collect_role(i.key);
}
void collect_roleinfo()
{
	for(auto &i:role_map)
	{
		i.second.name = "ha";
	}
}


///////////////////////////////////////////////////////////////
const int LevelRankSize = 100;
struct LevelInfo
{
	std::string name;
};
typedef std::pair<int,int> LevelKeyType;
typedef RankInfo<int,int,LevelInfo> LevelRankType;
void assign(LevelInfo &level_info, const RoleInfo &role_info)
{
	level_info.name = role_info.name;
}
void fill_rank_table(std::vector<LevelRankType> &level_rank)
{
	for(auto &i:level_rank)
	{
		assign(i.other_info, role_map[i.key]);
	}
}
///////////////////////////////////////////////////////////////


using namespace std;
int main(int argc, char **argv)
{
	//define tables
	TopTable<LevelKeyType, LevelRankSize> level_table;

	//push
	for(int i=0;i<9999;++i)
		level_table.Push(std::make_pair(rand(),++i));

	//build
	vector<LevelRankType> level_rank;
	level_table.Build(level_rank);

	//collect
	collect_roles(level_rank);

	//collect infos
	collect_roleinfo();

	//fill info
	fill_rank_table(level_rank);

	//save to db
	cout << "key:level:name\n";
	for(auto &i:level_rank)
		cout << i.key << ":" << i.score << ":" << i.other_info.name << endl;

	return 0;
}
