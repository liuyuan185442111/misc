#include <cstddef>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <string>
#include <assert.h>
#include <map>

template <typename T, typename Cmp = std::less<T>>
void insert_sort(T *a, int n, const Cmp &cmpor = Cmp())
{
	for(int i=1, j; i<n; ++i)
	{
		T picket = a[i];
		for(j=i-1; j>=0 && cmpor(picket,a[j]); --j)
			a[j+1] = a[j];
		a[j+1] = picket;
	}
}
template <typename T, typename Cmp = std::less<T>>
void insert_sort(T *a, T *b, const Cmp &cmpor = Cmp())
{
	if(std::distance(a,b) < 2) return;
	for(T *i=a+1, *j; i!=b; ++i)
	{
		T picket = *i;
		for(j=i-1; j!=a && cmpor(picket,*j); --j)
			*(j+1) = *j;
		*(j+1) = picket;
	}
}

template <typename KeyType, typename ScoreType, typename InfoType, typename ScoreCmp=std::greater<ScoreType>>
class RankingList
{
	struct RankInfo
	{
		KeyType key;
		ScoreType score;
		InfoType other_info;
		int last_ranking;
		RankInfo(KeyType k, ScoreType s, InfoType i) : key(k), score(s), other_info(i), last_ranking(0) {}
	};
	ScoreCmp _cmpor;
	size_t _wanted;
	size_t _maxsize;
	RankInfo *_rank;
	RankInfo *_min;
	std::unordered_map<KeyType, RankInfo*> _hashMap;

	size_t _update_counter;
	std::string _data;
	bool _ready;
	time_t _last_reform_time;

	void update_min()
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			if(_cmpor(i->score, _min->score)) _min = i;
	}
	void update_min_and_map()
	{
		int rank = 0;
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
		{
			if(_cmpor(i->score, _min->score)) _min = i;
			i->last_ranking = ++rank;
			_hashMap[i->key] = i;
		}
	}
	//false: not changed
	bool sort()
	{
		if(_update_counter == 0) return false;
		if(_update_counter < _maxsize / 10 * 7)
			insert_sort(_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
		else
			std::stable_sort(_rank, _rank + _hashMap.size(), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(rhs.score, lhs.score);});
		_update_counter = 0;
		return true;
	}

public:
	void save()
	{
		int *buf = (int *)_rank - 1;
		*buf = (int)_hashMap.size();
		size_t len = sizeof(int) + sizeof(RankInfo) * *buf;
		_data.assign((const char *)buf, len);
		//send to db
	}

	void load(void *buf, size_t len)
	{
		if(_ready) return;
		if(buf && len)
		{
			int size = *(int *)buf;
			if(len != sizeof(int) + size * sizeof(RankInfo))
			{
				//err
				return;
			}
			std::memcpy((int *)_rank - 1, buf, len);
			sort();
			update_min_and_map();
			_data.assign((const char *)buf, len);
		}
		_ready = true;
	}
	void forward_merge(void *buf, size_t len)
	{
		if(buf && len)
		{
			int size = *(int *)buf;
			if(len != sizeof(int) + size * sizeof(RankInfo))
			{
				//err
				assert(false);
				return;
			}
			for(RankInfo *i = (RankInfo *)((int *)buf + 1), *e = i + size; i != e; ++i)
				update(i->key, i->score, i->other_info);
		}
	}

	RankingList(size_t n) : _cmpor(ScoreCmp()), _wanted(n?n:1), _maxsize(_wanted/* + std::max(_wanted/10, (size_t)10)*/), _min(nullptr), _update_counter(0), _ready(false), _last_reform_time(0)
	{
		_rank = (RankInfo *)((int *)malloc(sizeof(int) + (_maxsize+1) * sizeof(RankInfo)) + 1);
		memset(_rank, 0, _maxsize * sizeof(RankInfo));
		_hashMap.reserve(_maxsize);
	}
	~RankingList() { free((int *)_rank - 1); }
	void reset()
	{
		_min = nullptr;
		_hashMap.clear();
		memset(_rank, 0, _maxsize * sizeof(RankInfo));
	}

	bool update(KeyType key, ScoreType score, const InfoType &info = InfoType())
	{
		if(!_ready) return false;
		auto it = _hashMap.find(key);
		if(it != _hashMap.end())
		{
			//found in _rank
			if(_cmpor(score, it->second->score))
			{
				if(it->second != _rank)
				{
					RankInfo *p = it->second;
					for(; p != _rank; --p)
					{
						if(_cmpor(score, (p-1)->score))
						{
							*p = *(p-1);
							_hashMap[p->key] = p;
						}
						else
							break;
					}
					p->key = key;
					p->score = score;
					p->other_info = info;
					_hashMap[key] = p;
				}
				else
				{
					it->second->score = score;
					it->second->other_info = info;
				}
			}
			else if(_cmpor(it->second->score, score))
			{
				if(it->second != _rank + _hashMap.size() - 1)
				{
					RankInfo *p = it->second;
					for(; p!=_rank+_hashMap.size()-1; ++p)
					{
						if(_cmpor((p+1)->score, score))
						{
							*p = *(p+1);
							_hashMap[p->key] = p;
						}
						else
							break;
					}
					p->key = key;
					p->score = score;
					p->other_info = info;
					_hashMap[key] = p;
				}
				else
				{
					it->second->score = score;
					it->second->other_info = info;
				}
			}
		}
		else
		{
			size_t cursize = _hashMap.size();
			if(cursize != _maxsize)
			{
				//_rank not full
				RankInfo *pos = std::upper_bound(_rank, _rank + cursize, RankInfo(key,score,info), [this](const RankInfo &lhs, const RankInfo &rhs){return _cmpor(lhs.score, rhs.score);});
				RankInfo *p = _rank + cursize;
				for(; p != pos; --p)
				{
					*p = *(p-1);
					_hashMap[(p-1)->key] = p;
				}
				p->key = key;
				p->score = score;
				p->other_info = info;
				_hashMap.insert(std::make_pair(key, p));
			}
			else if(_cmpor(score, (_rank + _maxsize - 1)->score))
			{
				RankInfo *p = _rank + _maxsize - 1;
				_hashMap.erase(p->key);
				for(; p!=_rank; --p)
				{
					if(_cmpor(score, (p-1)->score))
					{
						*p = *(p-1);
						_hashMap[p->key] = p;
					}
					else
						break;
				}
				p->key = key;
				p->score = score;
				p->other_info = info;
				_hashMap[key] = p;
			}
		}
	}
	bool remove(KeyType key)
	{
		if(!_ready) return false;
		auto it = _hashMap.find(key);
		if(it == _hashMap.end()) return false;
		RankInfo *target = it->second;
		_hashMap.erase(key);

		RankInfo *last = _rank + _hashMap.size();
		if(target == last)
		{
			if(_rank == last)
			{
				_min = nullptr;
			}
			else if(target == _min)
			{
				--_min;
				update_min();
			}
		}
		else
		{
			_hashMap[last->key] = target;
			*target = *last;
			if(target == _min)
			{
				update_min();
			}
		}
		//不通知gs问题也不大
		++_update_counter;
		return true;
	}

	//名单满的时候返回最小分数
	//用于提前做判断剔除榜外数据
	ScoreType get_min_score()
	{
		if(_min && _maxsize == _hashMap.size()) return _min->score;
		return ScoreType();
	}
	time_t get_last_reform_time()
	{
		return _last_reform_time;
	}
	void get_all_data(std::string &data)
	{
		data = _data;
	}
	void get_wanted_data(std::string &data)
	{
	}

	void dump(std::ostream &out = std::cout)
	{
		for(auto i = _rank, e = _rank + _hashMap.size(); i != e; ++i)
			out << "key:" << i->key << ", score:" << i->score << ", last_ranking:" << i->last_ranking << std::endl;
	}
};

template <typename InfoType>
InfoType &make_info(InfoType &info) { return info; }

using std::string;
using std::cout;
using std::endl;

int topn()
{
	srand(time(NULL));
	srand(0);
	RankingList<int, int, int> r(500);
	r.load(NULL, 0);
	for(int i=6666666;i>0;--i)
	{
		int t = rand();
		r.update(t%10000,t);
		//if(rand()&1) r.remove(t%10000);
	}
	r.dump(std::cout);
	std::cout << std::endl;
	return 0;
}

#include <unistd.h>
#include <sys/time.h>
int main()
{
	topn();
	return 0;
	RankingList<int, int, int> r(1);
	r.load(NULL, 0);
	r.update(1,2);
	r.update(2,2);
	r.update(3,4);
	r.update(4,4);
	r.update(5,6);
	r.update(6,6);
	r.dump(std::cout);
	cout << endl;
	r.update(5,2);
	r.dump(std::cout);
	return 0;
}

