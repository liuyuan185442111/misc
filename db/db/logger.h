#ifndef _L_LOGGER_H
#define _L_LOGGER_H

#include <dirent.h>
#include <vector>
#include "pagecache.h"

namespace lcore {

inline time32_t time32_now() { return time(NULL) - 0; }

enum LoggerState { LS_CLEAN, LS_PREPARED, LS_REDO, LS_CORRUPT };
enum LoggerCommitResult { LCR_NULL, LCR_PREPARED, LCR_COMMIT, LCR_ABORT };

struct Logger
{
	virtual ~Logger() { }
	virtual LoggerState verify() { return LS_CLEAN; }
	virtual LoggerState redo(time32_t timestamp) { return LS_CLEAN; }
	virtual LoggerState prepare() { return LS_PREPARED; }
	virtual LoggerState commit(time32_t timestamp) = 0;

	static void save_page(PageFile *page_file, const Page *page, page_index_t page_index)
	{
		page_file->write(page_index, page->snapshot_ptr());
	}

	static void save_page(Page **it, Page **ie, PageFile *page_file)
	{
		for( ; it != ie; ++it)
			save_page(page_file, *it, (*it)->index());
	}

	static page_index_t save_page(Page **it, Page **ie, PageFile *log_file, page_index_t page_index)
	{
		for( ; it != ie; ++it)
			save_page(log_file, *it, page_index++);
		return page_index;
	}

	static void restore_page(PageFile *log_file, page_index_t it, page_index_t ie, PageFile *page_file)
	{
		PageLayout *layout = (PageLayout *)PageMemory::alloc();
		for( ; it != ie; ++it)
		{
			log_file->read(it, layout);
			page_file->write(*(page_index_t *)(layout + 1), layout);
		}
		PageMemory::free(layout);
	}
};

class NullLogger : public Logger
{
	PageCache *page_cache;
public:
	NullLogger(PageCache *cache) : page_cache(cache) { }
	LoggerState commit(time32_t timestamp)
	{
		page_cache->magic_layout_ptr()->_magic.logger_last_check = timestamp;
		page_cache->magic_layout_ptr()->_magic.logger_id         = 0;
		size_t snapshot_size;
		Page** snapshot = page_cache->snapshot_reference(snapshot_size);
		try
		{
			PageFile *page_file = page_cache->pagefile();
			save_page(snapshot, snapshot + snapshot_size, page_file);
			page_file->sync();
			return LS_CLEAN;
		}
		catch(const PageFile::Exception &e) { }
		return LS_CORRUPT;
	}
};

//echo lcore::DB | md5sum
const int log_magic_num = 0xdd9dc9ec;
//防止在checkpoint过程中的崩溃损坏整个数据库
class IntegrityLogger : public Logger
{
	bool has_log;
	char *file_name; //只是文件名, 没有路径
	page_index_t last_page;
	PageFile *log_file;
	PageCache *page_cache;
public:
	IntegrityLogger(PageCache *cache) : log_file(NULL), page_cache(cache)
	{
		const char *dbfile = page_cache->pagefile()->identity();
		file_name = (char *)malloc(strlen(dbfile) + 10);
		strcpy(file_name, dbfile);
		strcat(file_name, ".okintlog");
		has_log = (0 == access(file_name, F_OK));
		//access会设置errno的值!!!
		errno = 0;
	}
	~IntegrityLogger() { delete log_file; free(file_name); }
	LoggerState verify()
	{
		if(has_log)
		{
			struct stat buf;
			stat(file_name, &buf);
			if(buf.st_size % PAGESIZE != sizeof(int)) return LS_CORRUPT;
			last_page = buf.st_size / PAGESIZE;
			return LS_REDO;
		}
		return LS_CLEAN;
	}
	LoggerState redo(time32_t timestamp)
	{
		if(!has_log) return LS_CLEAN;
		try
		{
			log_file = new PageFile(file_name, O_RDWR);
			if(log_file->get_magic() != log_magic_num) return LS_CORRUPT;
			return commit(timestamp);
		}
		catch(const PageFile::Exception &e) { }
		return LS_CORRUPT;
	}
	LoggerState prepare()
	{
		size_t snapshot_size;
		Page** snapshot = page_cache->snapshot_reference(snapshot_size);
		try
		{
			log_file = new PageFile(file_name, O_CREAT|O_RDWR|O_TRUNC);
			last_page = save_page(snapshot, snapshot + snapshot_size, log_file, 0);
			log_file->sync();
			log_file->set_magic(log_magic_num);
			log_file->sync();
			return LS_PREPARED;
		}
		catch(const PageFile::Exception &e) { }
		return LS_CORRUPT;
	}
	LoggerState commit(time32_t timestamp)
	{
		page_cache->magic_layout_ptr()->_magic.logger_last_check = timestamp;
		page_cache->magic_layout_ptr()->_magic.logger_id         = 0;
		try
		{
			PageFile *page_file = page_cache->pagefile();
			restore_page(log_file, 0, last_page, page_file);
			delete log_file;
			log_file = NULL;
			page_file->sync();
			if(has_log)
			{
				has_log = false;
				PageLayout magic;
				page_file->read(0, &magic);
				magic._magic.logger_last_check = timestamp;
				magic._magic.logger_id         = 0;
				page_file->write(0, &magic);
			}
			else
			{
				page_file->write(0, page_cache->magic_layout_ptr());
			}
			page_file->sync();
			unlink(file_name);
			return LS_CLEAN;
		}
		catch(const PageFile::Exception &e) { }
		return LS_CORRUPT;
	}
};

} //namespace lcore

#endif
