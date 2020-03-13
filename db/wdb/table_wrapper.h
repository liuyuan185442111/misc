#ifndef _L_TABLEWRAPPER_H
#define _L_TABLEWRAPPER_H

#include "db.h"
#include "data_coder.h"

namespace lcore {

class TableWrapper
{
	DB *db;
	std::string db_path;
	DataCoder *compressor;
	TableWrapper(const TableWrapper &);
	TableWrapper& operator =(const TableWrapper &);

public:
	TableWrapper(const char *dbfile, size_t cache_high = 4096, size_t cache_low = 2048) : db(NULL), db_path(dbfile)
	{
		compressor = new SnappyCoder;
		db = new DB(db_path.c_str(), cache_high, cache_low);
		db->init();
	}
	~TableWrapper()
	{
		delete db;
		delete compressor;
	}
	bool find(const Octets &key, Octets &val)
	{
		if(key.size() == 0) return false;
		size_t val_len;
		if(void *value = db->find(key.begin(), key.size(), val_len))
		{
			compressor->Uncompress(Octets(value, val_len)).swap(val);
			free(value);
			return true;
		}
		return false;
	}
	bool insert(const Octets &key, const Octets &val)
	{
		if(key.size() == 0) return false;
		Octets com_val = compressor->Compress(val);
		db->put(key.begin(), key.size(), com_val.begin(), com_val.size(), true);
		return true;
	}
	void del(const Octets &key) { db->del(key.begin(), key.size()); }
	bool checkpoint() { return db->checkpoint(); }
	size_t count() { return db->count(); }
};

}

#endif
