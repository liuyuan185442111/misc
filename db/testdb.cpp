#include <map>
#include <string>
#include <vector>
#include <iostream>
#include "storage.h"
#include <sys/mman.h>
#include "conf.h"
#include "checkfileempty.h"
#include "table_wrapper.h"
using namespace std;

void ASSERT(bool check)
{
	if(!check) abort();
}

string conf_file = "game.conf";
char val_buf[0x10000];

void test_through()
{
	if(!lcore::Storage::open(conf_file))
	{
		cerr << "Initialize storage environment failed." << endl;
		return;
	}

	size_t insert_n = 0;
	size_t del_n = 0;
	puts("test insert and del begin...");

	vector<int> keys;
	srand(time(NULL));
	for(int i=0; i<10000; ++i)
	{
		int num = rand();
		if(num & 3) // 3/4
		{
			try
			{
				lcore::Transaction txn;
				lcore::Table *ptest = lcore::Storage::get_table("test", txn);
				ptest->insert((lcore::OctetsStream() << num), lcore::Octets(val_buf, (num & 0xFFFF)), txn, lcore::DB_NOOVERWRITE);
			}
			catch(const lcore::DbException &e)
			{
				puts(e.what());
				continue;
			}
			++insert_n;
			keys.push_back(num);
		}
		else if(!keys.empty())
		{
			try
			{
				lcore::Transaction txn;
				lcore::Table *ptest = lcore::Storage::get_table("test", txn);
				lcore::OctetsStream os;
				os << keys[num % keys.size()];
				ptest->del(os, txn);
				++del_n;
				keys.erase(keys.begin() + num % keys.size());
			}
			catch(const lcore::DbException &e)
			{
				puts(e.what());
			}
		}
		if(i % 1000 == 0) lcore::Storage::checkpoint();
	}
	puts("test insert and del end.");
	cout << "insert:" << insert_n << ", del:" << del_n << ", keys:" << keys.size() << endl;
	ASSERT(insert_n-del_n == keys.size());

	lcore::Storage::checkpoint();

	puts("test find begin...");
	random_shuffle(keys.begin(), keys.end());
	try
	{
		lcore::Transaction txn;
		lcore::Table *ptest = lcore::Storage::get_table("test", txn);
		for(vector<int>::iterator it = keys.begin(), ie = keys.end(); it != ie; ++it)
		{
			lcore::OctetsStream os_val;
			if(!ptest->find((lcore::OctetsStream() << *it), os_val, txn))
			{
				cout << "cannot find " << *it << endl;
				abort();
			}
			ASSERT(os_val.size() == (*it & 0xFFFF));
			if(memcmp(os_val.begin(), val_buf, os_val.size()) != 0)
			{
				cout << "value of " << *it << " err " << endl;
				abort();
			}
		}
	}
	catch(const lcore::DbException &e)
	{
		puts(e.what());
	}
	puts("test find end.\n");

	lcore::Storage::close();
}

//需要在单线程下测试
//因为内存不够, 所以需要多checkpoint几次, 但是在txn还没销毁时checkpoint会拿不到锁
void test_bigkey()
{
	ASSERT(lcore::Storage::open(conf_file));
	try
	{
		lcore::Transaction txn;
		lcore::Table *ptest = lcore::Storage::get_table("test", txn);
		int max = sizeof(val_buf);
		puts("insert...");
		for(int i=1; i<=max; ++i)
		{
			ptest->insert(lcore::Octets(val_buf, i), lcore::Octets(val_buf, i), txn);
			if(i % 100 == 0)
			{
				txn.commit();
				lcore::Storage::checkpoint();
			}
		}
		puts("find...");
		for(int i=1; i<=max; ++i)
		{
			lcore::Octets val;
			ASSERT(ptest->find(lcore::Octets(val_buf, i), val, txn));
			ASSERT(i == (int)val.size());
			ASSERT(memcmp(val_buf, val.begin(), i) == 0);
			if(i % 100 == 0)
			{
				txn.commit();
				lcore::Storage::checkpoint();
			}
		}
		puts("del...");
		for(int i=1; i<=max; ++i)
		{
			ptest->del(lcore::Octets(val_buf, i), txn);
			if(i % 100 == 0)
			{
				txn.commit();
				lcore::Storage::checkpoint();
			}
		}
	}
	catch(const lcore::DbException &e)
	{
		puts(e.what());
	}
	lcore::Storage::checkpoint();
	lcore::Storage::close();
}

void test_common(size_t n)
{
	srand(time(NULL));
	std::vector<int> keys;

	ASSERT(lcore::Storage::open(conf_file));
	cout << "begin insert...\n";
	try
	{
		lcore::Transaction txn;
		lcore::Table *ptest = lcore::Storage::get_table("test", txn);
		//写n条记录
		for(size_t i=1; i<=n; ++i)
		{
			int num = rand();
			keys.push_back(num);
			ptest->insert((lcore::OctetsStream() << num), lcore::Octets(val_buf, num & 0xFF), txn);
		}
	}
	catch(const lcore::DbException &e)
	{
		puts(e.what());
	}
	lcore::Storage::checkpoint();
	lcore::Storage::close();

	ASSERT(lcore::Storage::open(conf_file));
	random_shuffle(keys.begin(), keys.end());
	cout << "begin readback...\n";
	try
	{
		lcore::Transaction txn;
		lcore::Table *ptest = lcore::Storage::get_table("test", txn);
		//读回n条记录
		for(size_t i=0; i<n; ++i)
		{
			lcore::Octets val;
			ASSERT(ptest->find((lcore::OctetsStream() << keys[i]), val, txn));
			ASSERT(val.size() == (keys[i] & 0xFF));
			ASSERT(memcmp(val.begin(), val_buf, val.size()) == 0);
		}
		cout << "begin circle...\n";
		//执行下面的循环5n次
		for(size_t i=1; i<=5*n; ++i)
		{
			lcore::Octets val;
			ASSERT(ptest->find((lcore::OctetsStream() << keys.back()), val, txn));
			random_shuffle(keys.begin(), keys.end());
			//每循环37次随机删除一条记录
			if(i % 37 == 0)
			{
				ptest->del((lcore::OctetsStream() << keys.back()), txn);
				keys.pop_back();
			}
			//每循环11次添加一条新记录并读回
			else if(i % 11 == 0)
			{
				int key = rand();
				lcore::OctetsStream os;
				os << key;
				ptest->insert(os, lcore::Octets(val_buf, key&0xFF), txn);
				ASSERT(ptest->find(os, val, txn));
				ASSERT(val.size() == (key & 0xFF));
				ASSERT(memcmp(val.begin(), val_buf, val.size()) == 0);
				keys.push_back(key);
			}
			//每循环17次随机替换一条记录 一次用大的 一次用小的
			else if(i % 17 == 0)
			{
				switch(rand() % 3)
				{
					case 0:
						ptest->insert((lcore::OctetsStream() << keys.front()), val, txn);
						break;
					case 1:
						ptest->insert((lcore::OctetsStream() << keys.front()), lcore::Octets(val_buf, sizeof(val_buf)), txn);
						break;
					case 2:
						ptest->insert((lcore::OctetsStream() << keys.front()), lcore::Octets(val_buf, val.size()/2), txn);
						break;
					default:
						break;
				}
			}
		}
	}
	catch(const lcore::DbException &e)
	{
		puts(e.what());
	}
	lcore::Storage::checkpoint();
	lcore::Storage::close();

	ASSERT(lcore::Storage::open(conf_file));
	random_shuffle(keys.begin(), keys.end());
	cout << "begin clear...\n";
	try
	{
		lcore::Transaction txn;
		lcore::Table *ptest = lcore::Storage::get_table("test", txn);
		//删除所有记录 每删除一条 随机查找10条
		while(!keys.empty())
		{
			ptest->del((lcore::OctetsStream() << keys.back()), txn);
			keys.pop_back();
			random_shuffle(keys.begin(), keys.end());
			for(int j=0; j<std::min(10,(int)keys.size()); ++j)
			{
				lcore::Octets val;
				ASSERT(ptest->find((lcore::OctetsStream() << keys[j]), val, txn));
			}
		}
	}
	catch(const lcore::DbException &e)
	{
		puts(e.what());
	}
	lcore::Storage::checkpoint();
	lcore::Storage::close();
}

class collect_keys : public lcore::IQueryKey
{
	vector<int> &v;
public:
	collect_keys(vector<int> &keys):v(keys){}
	virtual bool update(const void *key, size_t key_len)
	{
		lcore::OctetsStream os_key(lcore::Octets(key, key_len));
		try {
			int i;
			os_key >> i; 
			v.push_back(i);
		}
		catch(...)
		{
			cout << "unmarshal exception\n";
			return false;
		}
		return true;
	}
};
void test_trunc()
{
	if(!lcore::Storage::open(conf_file))
	{
		cerr << "Initialize storage environment failed." << endl;
		return;
	}
	puts("test truncate begin...");
	puts("start to collect keys...");
	vector<int> keys;
	{
		collect_keys query(keys);
		lcore::Transaction txn;
		lcore::Table *ptest = lcore::Storage::get_table("test", txn);
		ptest->walk(&query, txn);
	}
	cout << "there are " << keys.size() << " keys.\n";

	puts("start to delete data...");
	int counter = 0;
	for(vector<int>::iterator it = keys.begin(), ie = keys.end(); it != ie; ++it)
	{
		try
		{
			lcore::Transaction txn;
			lcore::Table *ptest = lcore::Storage::get_table("test", txn);
			ptest->del((lcore::OctetsStream() << *it), txn);
		}
		catch(...){}
		if(++counter == 2000)
		{
			counter = 0;
			lcore::Storage::checkpoint();
		}
	}
	cout << "truncate ok.\n";
	lcore::Storage::checkpoint();
	lcore::Storage::close();
}

void test_insert(size_t n)
{
	lcore::TableWrapper tab("huge", 20000, 10000);
	time_t last = time(NULL);
	for(size_t i=0; i<n; )
	{
		for(int j=0; j<100000; ++j,++i)
			tab.insert((lcore::OctetsStream() << int64_t(i)), (lcore::OctetsStream() << j));
		tab.checkpoint();
		time_t now = time(NULL);
		printf("index: %09d, used: %ds, time: %s", (int)i, int(now-last), ctime(&now));
		last = now;
	}
	tab.checkpoint();
}

//测试时需要屏蔽掉SAME_BYTE_ORDER宏
void test_prev()
{
	std::set<int, std::greater<int> > keys;
	std::vector<int> vec;
	lcore::TableWrapper tab("prev", 20000, 10000);
	for(int i=1; i<100000; ++i)
	{
		int t = (1 << 24) + i;
		keys.insert(t);
		vec.push_back(t);
	}

	random_shuffle(vec.begin(), vec.end());
	for(std::vector<int>::iterator it=vec.begin();it!=vec.end();++it)
	{
		tab.insert((lcore::OctetsStream() << *it), (lcore::OctetsStream() << 3.1415**it));
	}
	tab.checkpoint();

	cout << "there are " << tab.count() << " elements in table\n";
	cout << "test begin...\n";

	{
	lcore::Octets prev;
	ASSERT( !tab.prev_key((lcore::OctetsStream() << 0), prev) );
	}
	{
	lcore::Octets prev;
	tab.prev_key((lcore::OctetsStream() << (1 << 25)), prev);
	lcore::OctetsStream oskey(prev);
	int num;
	oskey >> num;
	ASSERT(num == (1 << 24) + 99999);
	}

	lcore::Octets okey;
	tab.last_key(okey);
	lcore::OctetsStream oskey(okey);
	std::set<int, std::greater<int> >::iterator it=keys.begin();
	int num;
	oskey >> num;
	ASSERT(num == *it);

	lcore::Octets prev;
	for(++it;it!=keys.end();++it)
	{
		tab.prev_key(okey, prev);
		lcore::OctetsStream oskey(prev);
		int num;
		oskey >> num;
		ASSERT(num == *it);
		okey = prev;
	}

	cout << "test end\n";

	tab.checkpoint();
}

int main(int argc, char **argv)
{
	if(argc == 1)
	{
		cout << argv[0] << " [through]|[bigkey]|[common count]|[insert count]|[prev]" << endl;
		return 0;
	}

	for(int *p = (int *)val_buf, *e = (int *)(val_buf + sizeof(val_buf)); p!=e; ++p) *p = rand();
	lcore::Conf::load(conf_file.c_str());

	if(argc == 2 && strcmp(argv[1], "through") == 0)
	{
		//测试插入/删除/查询
		for(int i=0; i<8; ++i)
			test_through();
		test_trunc();
		ASSERT(check_file_empty("dbhome/data/test") == 0);
	}
	else if(argc == 2 && strcmp(argv[1], "bigkey") == 0)
	{
		test_bigkey();
		ASSERT(check_file_empty("dbhome/data/test") == 0);
	}
	else if(argc == 3 && strcmp(argv[1], "common") == 0)
	{
		//apue上提供的测试方法, c不要太大, 10000就要跑半分钟
		int c = atoi(argv[2]);
		test_common(c);
		ASSERT(check_file_empty("dbhome/data/test") == 0);
	}
	else if(argc == 3 && strcmp(argv[1], "insert") == 0)
	{
		int c = atoi(argv[2]);
		test_insert(c);
	}
	else if(argc == 2 && strcmp(argv[1], "prev") == 0)
	{
		test_prev();
	}

	return 0;
}
