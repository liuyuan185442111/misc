#ifndef _L_DB_H
#define _L_DB_H

#include "logger.h"

namespace lcore {

class _db_interface
{
protected:
	PageCache *page_cache;
	_db_interface() : page_cache(NULL) { }

public:
	//以下方法如果返回值是char *, 除非特别说明, 其返回值需要手动free
	//如果找到, 返回数据, val_len被置为数据的大小, 其返回值需要手动free
	//否则, 返回NULL, val_len不变
	char* find(const void* key, size_t key_len, size_t& val_len)
	{
		return (char *)page_cache->find(key, key_len, val_len);
	}
	//同上, 如果val_buf足够大, 也会将数据放入其中
	//如果返回值不等于val_buf说明传入的buffer过小, 其返回值需要手动free
	char* find(const void* key, size_t key_len, void* val_buf, size_t& val_len)
	{
		return (char *)page_cache->find(key, key_len, val_len, val_buf);
	}
	//已有旧数据且replace为false的情况返回false, 其他情况返回true
	bool put(const void* key, size_t key_len, const void* val, size_t val_len, bool replace)
	{
		return page_cache->put(key, key_len, val, val_len, replace);
	}
	//如果有旧数据, 返回旧数据, val_len被置为旧数据的大小, 其返回值需要手动free
	//否则, 返回NULL, val_len不变
	char* put(const void* key, size_t key_len, const void* val, size_t& val_len)
	{
		return (char *)page_cache->put(key, key_len, val, val_len);
	}
	bool del(const void* key, size_t key_len)
	{
		return page_cache->del(key, key_len);
	}
	char* del(const void* key, size_t key_len, size_t& val_len)
	{
		return (char *)page_cache->del(key, key_len, val_len);
	}
	bool exist(const void* key, size_t key_len)
	{
		return page_cache->exist(key, key_len);
	}
	char* first_key(size_t& key_len)
	{
		return (char *)page_cache->first_key(key_len);
	}
	char* next_key(const void* key, size_t& key_len)
	{
		return (char *)page_cache->next_key(key, key_len);
	}
	char* last_key(size_t& key_len)
	{
		return (char *)page_cache->last_key(key_len);
	}
	char* prev_key(const void* key, size_t& key_len)
	{
		return (char *)page_cache->prev_key(key, key_len);
	}
	size_t count() const
	{
		return page_cache->record_count();
	}

	void walk(IQueryKey  *query) { page_cache->walk(query); }
	void walk(IQueryData *query) { page_cache->walk(query); }
	void walk(const void *key, size_t key_len, IQueryKey  *query) { page_cache->walk(key, key_len, query); }
	void walk(const void *key, size_t key_len, IQueryData *query) { page_cache->walk(key, key_len, query); }
};

class DB : public _db_interface
{
	size_t cache_high, cache_low;
	char *dbfile;
	Logger *logger;

	DB(const DB &);
	DB& operator=(const DB &);

public:
	DB(const char *file, size_t high = 4096, size_t low = 2048) : cache_high(high), cache_low(low), logger(NULL)
	{
		dbfile = (char *)malloc(strlen(file) + 1);
		strcpy(dbfile, file);
	}
	~DB()
	{
		free(dbfile);
		delete logger;
		delete page_cache;
	}
	bool init(bool safe = true)
	{
		page_cache = new PageCache(dbfile, cache_high, cache_low);
		if(safe)
			logger = new IntegrityLogger(page_cache);
		else
			logger = new NullLogger(page_cache);
		switch(logger->verify())
		{
			case LS_CORRUPT:
				fprintf(stderr, "log file corrupt!\n");
				return false;
			case LS_REDO:
				if(logger->redo(time32_now()) != LS_CLEAN)
				{
					fprintf(stderr, "log file corrupt!!\n");
					return false;
				}
				delete logger;
				delete page_cache;
				page_cache = new PageCache(dbfile, cache_high, cache_low);
				logger = new IntegrityLogger(page_cache);
				if(logger->verify() != LS_CLEAN)
				{
					fprintf(stderr, "log file corrupt!!!\n");
					return false;
				}
				break;
			default:
				break;
		}
		return true;
	}
	bool checkpoint()
	{
		page_cache->snapshot_create();
		bool r = (logger->prepare() == LS_PREPARED && logger->commit(time32_now()) == LS_CLEAN);
		page_cache->snapshot_release();
		return r;
	}

	size_t hashsize() const { return page_cache->hashsize(); }
	void dump_performance() const { page_cache->performance()->dump(); }
};

} //namespace lcore

#endif
