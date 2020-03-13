#include "table.h"

namespace lcore {

void Transaction::lock()
{
	if(_locked) return;
	std::sort(_tables.begin(), _tables.end());
	std::unique(_tables.begin(), _tables.end());
	std::for_each(_tables.begin(), _tables.end(), std::mem_fun(&Table::_lock));
	_locked = true;
	_canceled = false;
}

Transaction::~Transaction()
{
	if(!_locked) return;
	std::for_each(_tables.begin(), _tables.end(), std::bind2nd(std::mem_fun(&Table::_commit_and_unlock),_canceled));
	_locked = false;
}

void Transaction::commit()
{
	if(!_locked) return;
	std::for_each(_tables.begin(), _tables.end(), std::bind2nd(std::mem_fun(&Table::_commit),_canceled));
}

Table::Table(const std::string &name, size_t cache_high, size_t cache_low)
	: _manager(NULL), _name(name), _cache_high(cache_high), _cache_low(cache_low)
{
	_compressor = new SnappyCoder;
}

void Table::open()
{
	_manager = new PageCache(_name.c_str(), _cache_high, _cache_low);
}

void Table::close()
{
	delete _manager;
	delete _compressor;
}

bool Table::find(const Octets &key, Octets &val, Transaction &txn)
{
	if(key.size() == 0) throw DbException(DB_KEYSIZEZERO);
	try
	{
		txn.lock();
		size_t val_len;
		if(void *value = _manager->find(key.begin(), key.size(), val_len))
		{
			_compressor->Uncompress(Octets(value, val_len)).swap(val);
			free(value);
			return true;
		}
		return false;
	}
	catch(const DbException &e)
	{
		txn.abort();
		throw e;
	}
	catch(const PageFile::Exception &e)
	{
		fprintf(stderr, "Table::find DISK ERROR! PROGRAM ABORT!\n");
		abort();
	}
	return false;
}

Octets Table::find(const Octets &key, Transaction &txn)
{
	if(key.size() == 0) throw DbException(DB_KEYSIZEZERO);
	try
	{
		txn.lock();
		size_t val_len;
		if(void *value = _manager->find(key.begin(), key.size(), val_len))
		{
			Octets val = _compressor->Uncompress(Octets(value, val_len));
			free(value);
			return val;
		}
		throw DbException(DB_NOTFOUND);
	}
	catch(const DbException &e)
	{
		txn.abort();
		throw e;
	}
	catch(const PageFile::Exception &e)
	{
		fprintf(stderr, "Table::find DISK ERROR! PROGRAM ABORT!\n");
		abort();
	}
	return Octets();
}

void Table::insert(const Octets &key, const Octets &val, Transaction &txn, int flags)
{
	if(key.size() == 0) throw DbException(DB_KEYSIZEZERO);
	try
	{
		txn.lock();
		Octets com_val = _compressor->Compress(val);
		if(flags & DB_NOOVERWRITE)
		{
			//可以当做是新插入, 如果不是新插入直接抛异常
			if(!_manager->put(key.begin(), key.size(), com_val.begin(), com_val.size(), false))
				throw DbException(DB_OVERWRITE);
			_new_key.insert(key);
		}
		else if(_prepared.find(key) == _prepared.end() && _new_key.find(key) == _new_key.end())
		{
			size_t val_len = com_val.size();
			if(void *origin_val = _manager->put(key.begin(), key.size(), com_val.begin(), val_len))
				//overwrite, 保存被替换的数据
				_prepared.insert(std::make_pair(key, std::make_pair(origin_val, val_len)));
			else
				//new key
				_new_key.insert(key);
		}
		else
		{
			//一个事务内多次插入同一个key
			_manager->put(key.begin(), key.size(), com_val.begin(), com_val.size(), true);
		}
	}
	catch(const DbException &e)
	{
		txn.abort();
		throw e;
	}
	catch(const PageFile::Exception &e)
	{
		fprintf(stderr, "Table::insert DISK ERROR! PROGRAM ABORT!\n");
		abort();
	}
}

void Table::del(const Octets &key, Transaction& txn)
{
	if(key.size() == 0) throw DbException(DB_KEYSIZEZERO);
	try
	{
		txn.lock();
		if(_prepared.find(key) == _prepared.end())
		{
			NewKey::iterator it = _new_key.find(key);
			if(it != _new_key.end())
			{
				//新插入还未提交
				_new_key.erase(it);
				_manager->del(key.begin(), key.size());
			}
			else
			{
				//新删除
				size_t val_len;
				if(void *origin_val = _manager->del(key.begin(), key.size(), val_len))
					//保存被删除的数据
					_prepared.insert(std::make_pair(key, std::make_pair(origin_val, val_len)));
			}
		}
		else
		{
			//已被覆盖或被删除但未提交
			_manager->del(key.begin(), key.size());
		}
	}
	catch(const DbException &e)
	{
		txn.abort();
		throw e;
	}
	catch(const PageFile::Exception &e)
	{
		fprintf(stderr, "Table::del DISK ERROR! PROGRAM ABORT!\n");
		abort();
	}
}

void Table::walk(IQueryKey *query, Transaction &txn)
{
	try
	{
		txn.lock();
		_manager->walk(query);
	}
	catch(const DbException &e)
	{
		txn.abort();
		throw e;
	}
	catch(const PageFile::Exception &e)
	{
		fprintf(stderr, "Table::walk DISK ERROR! PROGRAM ABORT!\n");
		abort();
	}
}

void Table::_snapshot_create()
{
	_manager->snapshot_create();
	_locker.UNLock();
}

void Table::_snapshot_release()
{
	Mutex::Scoped l(_locker);
	_manager->snapshot_release();
}

void Table::__commit(bool cancel, bool unlock)
{
	try
	{
		if(cancel)
		{
			for(Prepared::const_iterator it = _prepared.begin(), ie = _prepared.end(); it != ie; ++it)
			{
				_manager->put(it->first.begin(), it->first.size(), it->second.first, it->second.second, true);
				free(it->second.first);
			}
			for(NewKey::const_iterator it = _new_key.begin(), ie = _new_key.end(); it != ie; ++it)
				_manager->del(it->begin(), it->size());
		}
		else
		{
			for(Prepared::const_iterator it = _prepared.begin(), ie = _prepared.end(); it != ie; ++it)
				free(it->second.first);
		}
		_prepared.clear();
		_new_key.clear();
		if(unlock) _locker.UNLock();
	}
	catch(const PageFile::Exception &e)
	{
		fprintf(stderr, "Table::_commit DISK ERROR! PROGRAM ABORT!\n");
		abort();
	}
}

}
