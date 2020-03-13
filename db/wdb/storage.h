#ifndef _L_STORAGE_H
#define _L_STORAGE_H

#include "table.h"
#include "global_logger.h"

namespace lcore {

class Storage
{
	static TABLE_MAP _table_map;
	static TABLE_LIST _table_list;
	static GlobalLogger *_logger;
	static Mutex _checkpoint_locker;

	static std::string _datadir;
	static std::string _logdir;

	static void load();
	static void unload();
	static bool checkpoint_prepare() { return _logger->prepare() == LS_PREPARED; }
	static bool checkpoint_commit() { return _logger->commit(time32_now()) == LS_CLEAN; }

public:
	static bool open(const std::string &conf_file);
	static bool init();
	static void close();
	static Table* get_table(const std::string &name, Transaction &txn);
	static bool checkpoint();

public:
	static void *BackupThread(void *pParam);
};


}

#endif
