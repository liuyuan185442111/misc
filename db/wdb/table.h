#ifndef _TABLE_H
#define _TABLE_H

#include <map>
#include <set>
#include <vector>
#include "marshal.h"
#include "pagecache.h"
#include "data_coder.h"
#include "mutex.h"

namespace lcore {

class Table;
typedef std::map<std::string, Table *> TABLE_MAP;
typedef std::vector<Table *> TABLE_LIST;

class Transaction
{
	bool _locked;
	bool _canceled;
	TABLE_LIST _tables;

	void* operator new(size_t);
	void operator delete(void*);
	Transaction(const Transaction &);
	Transaction& operator=(const Transaction &);

public:
	Transaction() : _locked(false), _canceled(false) { }
	~Transaction();
	Table* add_table(Table *t) { _tables.push_back(t); return t; }
	void commit();
	void abort() { _canceled = true; }
	void lock();
};

class Storage;
class Table
{
	struct CompareOctets {
		bool operator()(const Octets &o1, const Octets &o2) const
		{
			size_t s1 = o1.size();
			size_t s2 = o2.size();
			if(int r = memcmp(o1.begin(), o2.begin(), std::min(s1, s2)))
				return r < 0;
			return s1 < s2;
		}
	};

	typedef std::map<const Octets, std::pair<void *, size_t>, CompareOctets > Prepared;
	typedef std::set<Octets, CompareOctets> NewKey;

	Prepared  _prepared; //保存overwrite/del操作的旧value
	NewKey    _new_key; //保存新插入的key

	PageCache *_manager;
	DataCoder *_compressor;
	Mutex _locker;

	std::string _name;
	size_t _cache_high;
	size_t _cache_low;

public:
	Table(const std::string &name, size_t cache_high = 4096, size_t cache_low = 2048);
	~Table() { close(); }

	void open();
	void close();
	PageCache *getcache() { return _manager; }

	bool find(const Octets &key, Octets &val, Transaction &txn);
	Octets find(const Octets &key, Transaction &txn);
	void insert(const Octets &key, const Octets &val, Transaction &txn, int flags = 0); //flags暂支持DB_NOOVERWRITE
	void del(const Octets &key, Transaction& txn);
	size_t count() const { return _manager->record_count(); }

	void walk(IQueryKey *query, Transaction &txn);

private:
	friend Storage;
	friend Transaction;
	void _lock() { _locker.Lock(); }
	void _snapshot_create();
	void _snapshot_release();
	void __commit(bool cancel, bool unlock);
	void _commit(bool cancel) { __commit(cancel, false); }
	void _commit_and_unlock(bool cancel) { __commit(cancel, true); }

	//todo
	//不应在从Table获得的Cursor的walk系列函数中调用StorageEnv::checkpoint(), 原因见PageCache::walk()中的注释
	//如果一定要调用StorageEnv::checkpoint(), 请使用TableWrapper来walk表
	//Cursor cursor() { return Cursor(db, compressor); }
};

}

#endif
