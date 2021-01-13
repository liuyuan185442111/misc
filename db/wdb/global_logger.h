#ifndef _L_GLOBAL_LOGGER_H
#define _L_GLOBAL_LOGGER_H

#include "logger.h"

namespace lcore {

class LogInfo;
class GlobalLogger : public Logger
{
	typedef std::vector<LogInfo *> LogInfoList;

	const char *logdir;
	size_t log_pages; //maximum amount of pages of a log file
	PageFile *logfile;
	time32_t timestamp; //of last operation
	PageLayout *logmagic; //1st page of log file
	LogInfoList loginfolist;
	bool need_rotate; //need a new log file

public:
	GlobalLogger(const char *log_dir, size_t max_pages_per_logfile = 4096);
	~GlobalLogger();
	void add_log(PageCache *cache);
	LoggerState verify();
	LoggerState redo(time32_t new_timestamp);
	LoggerState prepare();
	LoggerState commit(time32_t new_timestamp);
	time32_t get_logger_id() const;

private:
	page_index_t& logger_page_first() { return logmagic->_logger_magic.logger_page_first; }
	page_index_t& logger_page_last()  { return logmagic->_logger_magic.logger_page_last;  }
	page_index_t& logger_rec_max()    { return logmagic->_logger_magic.logger_rec_max;    }
	page_index_t& logger_rec_cur()    { return logmagic->_logger_magic.logger_rec_cur;    }
	time32_t&     logger_chain()      { return logmagic->_logger_magic.logger_chain;      }
	logger_hdr*   logger_item(page_index_t i) { return logmagic->_logger_magic.logger_head + i; }

	void sync_magic() { logfile->write(0, logmagic); logfile->sync(); }
	page_index_t logger_check_rotate();
	void open_logfile(time32_t timestamp);
	void new_logfile(time32_t new_timestamp);
	bool load_logfile(time32_t new_timestamp);
	bool restore(time32_t timestamp, page_index_t rec_index);

public:
	//使用增量备份恢复
	//找到比现在的数据库版本更新的所有版本
	std::vector<time32_t> check_version();
	//将数据恢复到timestamp时刻, 返回成功恢复的版本
	std::vector<time32_t> restore(time32_t timestamp);
};

}

#endif
