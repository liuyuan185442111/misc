#include "storage.h"
#include "conf.h"
#include "split.h"

namespace lcore {

#define DB_HOMEDIR_DEFAULT "./dbhome"
#define DB_DATADIR_DEFAULT "dbdata"
#define DB_LOGDIR_DEFAULT  "dblogs"

TABLE_MAP Storage::_table_map;
TABLE_LIST Storage::_table_list;
GlobalLogger *Storage::_logger = NULL;
Mutex Storage::_checkpoint_locker;
std::string Storage::_datadir;
std::string Storage::_logdir;

Table* Storage::get_table(const std::string &name, Transaction &txn)
{
	TABLE_MAP::iterator it = _table_map.find(name);
	return it != _table_map.end() ? txn.add_table(it->second) : NULL;
}

bool Storage::open(const std::string &conf_file)
{
	if(!Conf::load(conf_file.c_str())) return false;
	std::string homedir = Conf::find("storage", "homedir");
	_datadir = Conf::find("storage", "datadir");
	_logdir = Conf::find("storage", "logdir");
	if(homedir.empty()) homedir = DB_HOMEDIR_DEFAULT;
	if(_datadir.empty()) _datadir = DB_DATADIR_DEFAULT;
	if(_logdir.empty()) _logdir = DB_LOGDIR_DEFAULT;
	if(*homedir.rbegin() == '/') homedir.erase(homedir.end() - 1);
	if(*_datadir.rbegin() == '/') _datadir.erase(_datadir.end() - 1);
	if(*_logdir.rbegin() == '/') _logdir.erase(_logdir.end() - 1);
	_datadir = homedir + "/" + _datadir + "/";
	_logdir = homedir + "/" + _logdir + "/";

	size_t cache_high_default = atoi(Conf::find("storage", "cache_high_default").c_str());
	size_t cache_low_default = atoi(Conf::find("storage", "cache_low_default").c_str());
	std::vector<std::string> tables;
	SplitString(tables, Conf::find("storage", "tables").c_str(), ", ");
	std::sort(tables.begin(), tables.end());
	std::unique(tables.begin(), tables.end());
	for(size_t k=0; k<tables.size(); ++k)
	{
		size_t cache_high = cache_high_default;
		size_t cache_low = cache_low_default;
		std::string str_high = Conf::find("storage", tables[k] + "_cache_high");
		std::string str_low  = Conf::find("storage", tables[k] + "_cache_low");
		if(!str_high.empty()) cache_high = atoi(str_high.c_str());
		if(!str_low.empty())  cache_low  = atoi(str_low.c_str());
		if(0 == cache_high) cache_high = cache_high_default;
		if(0 == cache_low)  cache_low  = cache_low_default;
		Table *tmp;
		if(cache_low > 0 && cache_high > cache_low)
			tmp = new Table(_datadir + tables[k], cache_high, cache_low);
		else
			tmp = new Table(_datadir + tables[k]);
		_table_map[tables[k]] = tmp;
		_table_list.push_back(tmp);
		printf("create table %s addr %p cache_high %d cache_low %d\n",
				tables[k].c_str(), tmp, (int)cache_high, (int)cache_low);
	}
	return init();
}

bool Storage::init()
{
	system(("mkdir -p " + _datadir).c_str());
	system(("mkdir -p " + _logdir).c_str());
	load();
	LoggerState state = _logger->verify();
	if(state != LS_CLEAN)
	{
		fprintf(stderr, "verify log failed state = %d\n", state);
		if(state == LS_CORRUPT)
			abort();
		else if(state == LS_REDO)
			state = _logger->redo(time32_now());
		if(state == LS_CLEAN)
		{
			unload();
			load();
		}
	}
	return state == LS_CLEAN;
}

void Storage::close()
{
	Mutex::Scoped l(_checkpoint_locker);
	delete _logger;
	for(TABLE_LIST::iterator it = _table_list.begin(), ie = _table_list.end(); it != ie; ++it)
		delete *it;
	_table_map.clear();
	_table_list.clear();
}

void Storage::load()
{
	//这里需要排个序, checkpoint会依次对每个表加锁,
	//Transaction::lock也会对管理的表排序后依次加锁, 这样就不会死锁了
	std::sort(_table_list.begin(), _table_list.end());
	_logger = new GlobalLogger(_logdir.c_str());
	for(TABLE_LIST::iterator it = _table_list.begin(), ie = _table_list.end(); it != ie; ++it)
	{
		(*it)->open();
		_logger->add_log((*it)->getcache());
	}
}

void Storage::unload()
{
	std::for_each(_table_list.begin(), _table_list.end(), std::mem_fun(&Table::close));
	delete _logger;
}

bool Storage::checkpoint()
{
	try
	{
		Mutex::Scoped l(_checkpoint_locker);
		if(_table_list.empty()) return true;
		std::for_each(_table_list.begin(), _table_list.end(), std::mem_fun(&Table::_lock));
		std::for_each(_table_list.begin(), _table_list.end(), std::mem_fun(&Table::_snapshot_create));
		bool r = checkpoint_prepare() && checkpoint_commit();
		std::for_each(_table_list.begin(), _table_list.end(), std::mem_fun(&Table::_snapshot_release));
		return r;
	}
	catch(const PageFile::Exception &e)
	{
		fprintf(stderr, "StorageEnv::checkpoint DISK ERROR! PROGRAM ABORT!\n");
		abort();
	}
	return false;
}

#include <signal.h>
void *Storage::BackupThread(void *pParam)
{
	pthread_detach(pthread_self());

	sigset_t sigs;
	sigfillset(&sigs);
	pthread_sigmask(SIG_BLOCK, &sigs, NULL);

	time_t last_checked = time(NULL);
	while(true)
	{
		Conf::load();

		long cp_interval = atol(Conf::find("storage", "checkpoint_interval").c_str());
		int elapsed = time(NULL) - last_checked;
		if(elapsed < cp_interval)
			sleep(cp_interval - elapsed);
		last_checked = time(NULL);

		puts("checkpoint begin");
		if(!checkpoint())
		{
			fprintf(stderr, "Storage::checkpoint failed!\n");
		}
		puts("checkpoint end");

		if(0 == unlink(Conf::find("storage", "quit_file").c_str()))
		{
			puts("quit...");
			if(0 != kill(0, SIGUSR1)) exit(0);
			break;
		}
	}

	return NULL;
}

}
