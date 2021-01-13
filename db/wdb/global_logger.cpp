#include "global_logger.h"

namespace lcore {

class LogInfo
{
	PageCache *page_cache;
	void *key_page;
	page_index_t key_page_index;

public:
	LogInfo(PageCache *cache) : page_cache(cache), key_page(PageMemory::alloc()) { }
	~LogInfo() { PageMemory::free(key_page); }
	size_t init(PageFile *logfile, page_index_t page_index)
	{
		memset(key_page, 0, PAGESIZE);
		size32_t *lp = (size32_t *)key_page;
		size_t len = strlen(identity()) + 1;
		*lp = (len + sizeof(size32_t) + 3) & ~3;
		memcpy(lp + 1, identity(), len);
		logfile->write(key_page_index = page_index, key_page);
		return *lp;
	}
	void load(page_index_t page_index, void *page)
	{
		memcpy(key_page, page, PAGESIZE);
		key_page_index = page_index;
	}

	logger_hdr* logger_item(page_index_t i) { return (logger_hdr *)((byte_t *)key_page + *(size32_t *)key_page) + i; }
	time32_t& logger_id() { return page_cache->magic_layout_ptr()->_magic.logger_id; }
	time32_t& logger_last_check() { return page_cache->magic_layout_ptr()->_magic.logger_last_check; }
	const char *identity() const { return page_cache->pagefile()->identity(); }

	page_index_t prepare(PageFile *logfile, page_index_t page_index, page_index_t rec_index)
	{
		size_t snapshot_size;
		Page** snapshot = page_cache->snapshot_reference(snapshot_size);
		logger_item(rec_index)->logger_first_idx = page_index;
		logger_item(rec_index)->logger_last_idx  = page_index = Logger::save_page(snapshot, snapshot + snapshot_size, logfile, page_index);
		//没有脏页面的情况下, first_idx等于last_idx
		logfile->write(key_page_index, key_page);
		return page_index;
	}
	void commit(PageFile *logfile, time32_t _logger_id, time32_t timestamp, page_index_t rec_index)
	{
		PageFile *pagefile = page_cache->pagefile();
		PageLayout *layout = (PageLayout *)PageMemory::alloc();
		//没有脏页面的情况下, it比ie大1
		page_index_t it = logger_item(rec_index)->logger_first_idx;
		page_index_t ie = logger_item(rec_index)->logger_last_idx - 1;
		try { pagefile->read(0, layout); } catch(const PageFile::Exception &e) { } //new talbe's first commit would throw exception
		layout->_magic.logger_id = logger_id() = _logger_id; //set the magic page in file and the one in memory
		if(it <= ie)
		{
			pagefile->write(0, layout);
			pagefile->sync();
			Logger::restore_page(logfile, it, ie, pagefile);
			//snapshot最后一页是magic页
			logfile->read(ie, layout);
			layout->_magic.logger_id = _logger_id;
		}
		layout->_magic.logger_last_check = logger_last_check() = timestamp;
		pagefile->write(0, layout);
		pagefile->sync();
		PageMemory::free(layout);
	}
};
static bool compare_loginfo(const LogInfo *lhs, const LogInfo *rhs) { return strcmp(lhs->identity(), rhs->identity()) < 0; }

GlobalLogger::GlobalLogger(const char *log_dir, size_t pages) : logdir(strdup(log_dir)), log_pages(pages), logfile(NULL), need_rotate(false)
{
	timestamp = time32_now();
	logmagic = (PageLayout *)PageMemory::alloc();
}

GlobalLogger::~GlobalLogger()
{
	for(LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end(); it != ie; ++it)
		delete *it;
	delete logfile;
	free((void *)logdir);
	PageMemory::free(logmagic);
}

void GlobalLogger::add_log(PageCache *cache)
{
	loginfolist.push_back(new LogInfo(cache));
}

LoggerState GlobalLogger::verify()
{
	try
	{
		std::sort(loginfolist.begin(), loginfolist.end(), compare_loginfo);
		time32_t max_logger_id = 0;
		for(LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end(); it != ie; ++it)
			max_logger_id = std::max(max_logger_id, (*it)->logger_id());
		if(max_logger_id == 0)
		{
			new_logfile(time32_now());
			return LS_CLEAN;
		}
		for(LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end(); it != ie; ++it)
		{
			time32_t cur_logger_id = (*it)->logger_id();
			if(cur_logger_id == 0) // new table, or logged by NullLogger
				need_rotate = true;
			else if(cur_logger_id < max_logger_id) // after logrotate, prepare succeed, but commit fail
				(*it)->logger_id() = max_logger_id;
		}
		if(!load_logfile(max_logger_id))
			return LS_CORRUPT; // some dbfile miss

		page_index_t rec_index = logger_rec_cur();
		switch(logger_item(rec_index)->check_result)
		{
			case LCR_NULL:
				if(logger_item(rec_index)->logger_first_idx)
				{
					//prepare fail
					logger_item(rec_index)->check_result = LCR_ABORT;
					logger_rec_cur() ++;
					sync_magic();
				}
				return LS_CLEAN;
			case LCR_PREPARED:
				//prepare succeed, commit fail
				return LS_REDO;
			default:
				return LS_CORRUPT; // 这种情况不会出现
		}
	}
	catch(const PageFile::Exception &e) { }
	return LS_CORRUPT;
}

LoggerState GlobalLogger::prepare()
{
	try
	{
		page_index_t page_index = logger_check_rotate();
		page_index_t rec_index  = logger_rec_cur();
		logger_item(rec_index)->logger_first_idx = page_index;
		sync_magic();
		for(LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end(); it != ie; ++it)
			page_index = (*it)->prepare(logfile, page_index, rec_index);
		logger_item(rec_index)->logger_last_idx = logger_page_last() = page_index;
		logger_item(rec_index)->check_result    = LCR_PREPARED;
		sync_magic();
		return LS_PREPARED;
	}
	catch(const PageFile::Exception &e) { }
	return LS_CORRUPT;
}

LoggerState GlobalLogger::redo(time32_t new_timestamp)
{
	try
	{
		timestamp = (new_timestamp <= timestamp) ? (timestamp + 1) : new_timestamp;
		page_index_t rec_index = logger_rec_cur();
		time32_t logger_id = get_logger_id();
		for(LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end(); it != ie; ++it)
			if((*it)->logger_id()) // 表示LogInfo::commit中途执行失败
				(*it)->commit(logfile, logger_id, timestamp, rec_index);
		logger_item(rec_index)->check_result = LCR_COMMIT;
		logger_item(rec_index)->logger_check_timestamp = timestamp;
		logger_rec_cur() ++;
		sync_magic();
		return LS_CLEAN;
	}
	catch(const PageFile::Exception &e) { }
	return LS_CORRUPT;
}

LoggerState GlobalLogger::commit(time32_t new_timestamp)
{
	try
	{
		timestamp = (new_timestamp <= timestamp) ? (timestamp + 1) : new_timestamp;
		page_index_t rec_index = logger_rec_cur();
		time32_t logger_id = get_logger_id();
		for(LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end(); it != ie; ++it)
			(*it)->commit(logfile, logger_id, timestamp, rec_index);
		logger_item(rec_index)->check_result = LCR_COMMIT;
		logger_item(rec_index)->logger_check_timestamp = timestamp;
		logger_rec_cur() ++;
		sync_magic();
		return LS_CLEAN;
	}
	catch(const PageFile::Exception &e) { }
	return LS_CORRUPT;
}

time32_t GlobalLogger::get_logger_id() const
{
	time32_t ts;
	sscanf(logfile->identity(), "log.%x", &ts);
	return ts;
}

void GlobalLogger::open_logfile(time32_t timestamp)
{
	delete logfile;
	char *name = (char *)malloc(strlen(logdir) + 32);
	sprintf(name, "%slog.%08x", logdir, timestamp);
	logfile = new PageFile(name);
	free(name);
}

//create new log file
void GlobalLogger::new_logfile(time32_t new_timestamp)
{
	if(timestamp >= new_timestamp)
		new_timestamp = timestamp + 1;
	if(logfile)
	{
		logger_chain() = new_timestamp;
		sync_magic();
	}
	open_logfile(new_timestamp);
	memset(logmagic, 0, PAGESIZE);
	logger_page_first() = logger_page_last() = loginfolist.size() + 1;
	timestamp = new_timestamp;
	size_t used_max = (byte_t *)logmagic->_logger_magic.logger_head - (byte_t *)logmagic;
	page_index_t page_index = 1;
	for(LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end(); it != ie; ++it)
		used_max = std::max(used_max, (*it)->init(logfile, page_index++));
	logmagic->_logger_magic.logger_rec_max = (PAGESIZE - used_max) / sizeof(logger_hdr);
	sync_magic();
}

//load existing log file
bool GlobalLogger::load_logfile(time32_t new_timestamp)
{
	bool r = true;
	open_logfile(timestamp = new_timestamp);
	logfile->read(0, logmagic);
	char *page = (char *)PageMemory::alloc();
	for(page_index_t page_index = 1; page_index < logger_page_first(); page_index++)
	{
		logfile->read(page_index, page);
		LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end();
		for( ; it != ie; ++it)
		{
			if(strcmp((*it)->identity(), page + sizeof(size32_t)) == 0)
			{
				(*it)->load(page_index, page);
				break;
			}
		}
		if(it == ie) r = false; //db file miss
	}
	PageMemory::free(page);
	return r;
}

page_index_t GlobalLogger::logger_check_rotate()
{
	if(need_rotate || logger_rec_cur() >= logger_rec_max() || logger_page_last() > log_pages)
		new_logfile(time32_now());
	return logger_page_last();
}

std::vector<time32_t> GlobalLogger::check_version()
{
	std::vector<time32_t> r;
	LogInfoList::iterator ii = loginfolist.begin();
	time32_t logger_last_check = (*ii)->logger_last_check();
	time32_t logger_id         = (*ii)->logger_id();
	logger_hdr *it = logger_item(0);
	logger_hdr *ie = logger_item(logger_rec_cur());
	while(it != ie && (*it).logger_check_timestamp <= logger_last_check) ++it;
	do
	{
		for( ; it != ie; ++it)
			if((*it).check_result == LCR_COMMIT)
				r.push_back((*it).logger_check_timestamp);
	}
	while(logger_chain() && load_logfile(logger_chain()) && (it = logger_item(0), ie = logger_item(logger_rec_cur()), true));
	load_logfile(logger_id);
	return r;
}

bool GlobalLogger::restore(time32_t timestamp, page_index_t rec_index)
{
	try
	{
		time32_t logger_id = get_logger_id();
		for(LogInfoList::iterator it = loginfolist.begin(), ie = loginfolist.end(); it != ie; ++it)
			(*it)->commit(logfile, logger_id, timestamp, rec_index);
		return true;
	}
	catch(const PageFile::Exception &e) { }
	return false;
}

std::vector<time32_t> GlobalLogger::restore(time32_t timestamp)
{
	std::vector<time32_t> r;
	LogInfoList::iterator ii = loginfolist.begin();
	time32_t logger_id         = (*ii)->logger_id();
	logger_hdr *it = logger_item(0);
	logger_hdr *ie = logger_item(logger_rec_cur());
	while(it != ie && (*it).logger_check_timestamp <= timestamp) ++it;
	logger_hdr *ib = it;
	do
	{
		for( ; it != ie; ++it)
		{
			if((*it).check_result == LCR_COMMIT)
			{
				if(restore((*it).logger_check_timestamp, it - ib))
					r.push_back((*it).logger_check_timestamp);
				else
					goto end;
			}
		}
	}
	while(logger_chain() && load_logfile(logger_chain()) && (it = ib = logger_item(0), ie = logger_item(logger_rec_cur()), true));
end:
	load_logfile(logger_id);
	return r;
}

}
