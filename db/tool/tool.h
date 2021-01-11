#ifndef _L_TOOL_H
#define _L_TOOL_H

#include "logger.h"
#include <deque>
#ifdef DB_USE_MMAP
#include <sys/mman.h>
#endif

namespace lcore {

class PageMonitor
{
	PageFile *page_file;
	PageLayout *layout;
	PageLayout *init_layout;
	PageLayout *last_layout;
	Performance perf_from_init;
	Performance perf_from_last;
	time32_t    time_init;
	time32_t    time_last;
	time32_t    time_new;

public:
	PageMonitor(const char *dbfile) : page_file(new PageFile(dbfile, O_RDONLY)), layout((PageLayout *)PageMemory::alloc()),
		init_layout((PageLayout *)PageMemory::alloc()), last_layout((PageLayout *)PageMemory::alloc())
	{
		page_file->read(0, layout);
		memcpy(init_layout, layout, PAGESIZE);
		memcpy(last_layout, layout, PAGESIZE);
		time_new = time_last = time_init = init_layout->_magic.logger_last_check;
	}
	~PageMonitor()
	{
		delete page_file;
		PageMemory::free(layout);
		PageMemory::free(init_layout);
		PageMemory::free(last_layout);
	}
	bool monitor()
	{
		page_file->read(0, layout);
		if(layout->_magic.logger_last_check <= last_layout->_magic.logger_last_check)
			return false;
		perf_from_last  = layout->_magic.performance;
		perf_from_last -= last_layout->_magic.performance;
		time_last = last_layout->_magic.logger_last_check;
		time_new  = layout->_magic.logger_last_check;
		memcpy(last_layout, layout, PAGESIZE);
		return true;
	}
	const Performance *performance_init(time_t *last_check = NULL)
	{
		if(last_check) *last_check = time_last;
		return &init_layout->_magic.performance;
	}
	const Performance *performance_from_begin(time_t *t_init, time_t *t_new)
	{
		perf_from_init  = last_layout->_magic.performance;
		perf_from_init -= init_layout->_magic.performance;
		*t_init = time_init;
		*t_new = time_new;
		return &perf_from_init;
	}
	const Performance *performance_from_checkpoint(time_t *t_last, time_t *t_new) const
	{
		*t_last = time_last;
		*t_new = time_new;
		return &perf_from_last;
	}
};

class PageBrowser
{
#ifdef DB_USE_MMAP
	byte_t *file_ptr;
	size_t file_len;
#else
	PageFile	*file;
	PageHash	hash;
#endif

	data_hdr* load(page_index_t page_index)
	{
#ifdef DB_USE_MMAP
		size_t offset = (size_t)page_index * PAGESIZE;
		if(offset >= file_len) throw PageFile::Exception();
		byte_t *dst = file_ptr + offset;
		if(*(page_index_t *)(dst + PAGEUSED + 4) == DATA_PAGE)
			return (data_hdr *)dst;
		return NULL;
#else
		Page *page = hash.find(page_index);
		if(!page) throw PageFile::Exception();
		if(page->get_type() != DATA_PAGE) return NULL;
		return page->layout_ptr()->_data;
#endif
	}

	data_hdr* next_data_head(data_hdr *hdr)
	{
		if(hdr->next_page_index)
		{
			try
			{
				if(data_hdr *next = load(hdr->next_page_index))
					return (data_hdr *)((byte_t *)next + hdr->next_page_pos);
			}
			catch(const PageFile::Exception &e) { }
		}
		return NULL;
	}

	void* extract_data(data_hdr *hdr)
	{
		size_t size = *(size32_t *)(hdr + 1);
		byte_t *data = (byte_t *)malloc(size);
		for(byte_t *p = data; hdr; p += hdr->size, hdr = next_data_head(hdr))
		{
			if(size < hdr->size)
			{
				free(data);
				return NULL;
			}
			memcpy(p, hdr + 1, hdr->size);
			size -= hdr->size;
		}
		if(size)
		{
			free(data);
			return NULL;
		}
		return data;
	}

public:
#ifdef DB_USE_MMAP
	~PageBrowser() { munmap(file_ptr, file_len); }
	PageBrowser(const char *filename, size_t=0) : file_ptr(NULL)
	{
		struct stat statbuf;
		int fd = open(filename, O_RDONLY);
		void *file;
		if(fd < 0)
		{
			fprintf(stderr, "open %s failed: %s\n", filename, strerror(errno));
			goto fail;
		}
		if(fstat(fd, &statbuf) < 0)
		{
			fprintf(stderr, "stat %s failed: %s\n", filename, strerror(errno));
			goto fail;
		}
		file = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if(file == MAP_FAILED)
		{
			fprintf(stderr, "mmap %s failed: %s\n", filename, strerror(errno));
			goto fail;
		}
		file_ptr = (byte_t *)file;
		file_len = statbuf.st_size;
		return;
fail:
		throw PageFile::Exception();
	}
#else
	~PageBrowser() { delete file; }
	PageBrowser(const char *filename, size_t max = 1024) : file(new PageFile(filename, O_RDONLY))
	{
		hash.init(file, max, max/2);
	}
#endif

	size_t action(IQueryData *query)
	{
		int corrupt_count = 0;
		try
		{
			for(page_index_t page_index = 1; ; page_index++)
			{
				if(data_hdr *hdr = load(page_index))
				{
					do
					{
						if(hdr->size && hdr->first_slice)
						{
							if(void *data = extract_data(hdr))
							{
								size_t key_len = *((size32_t *)data + 1);
								size_t val_len = *(size32_t *)data - key_len - sizeof(size32_t) - sizeof(size32_t);
								void *key = (byte_t *)data + sizeof(size32_t) + sizeof(size32_t);
								void *val = (byte_t *)key + key_len;
								query->update(key, key_len, val, val_len);
								free(data);
							}
							else
								corrupt_count++;
						}
					}
					while((hdr = hdr->next_header()));
				}
#ifndef DB_USE_MMAP
				hash.cleanup();
#endif
			}
		}
		catch(const PageFile::Exception &e) { }
		return corrupt_count;
	}
};

class IndexBrowser
{
	PageFile *file;
	int extract_key(index_hdr *hdr, char key_buf[])
	{
		if(hdr->key_len)
		{
			memcpy(key_buf, hdr->key, hdr->key_len);
		}
		//暂不支持大key
		return hdr->key_len;
	}
public:
	~IndexBrowser() { delete file; }
	IndexBrowser(const char *filename) : file(new PageFile(filename, O_RDONLY)) { }
	int action(IQueryKey *query)
	{
		Page cur_page(0);
		file->read(0, cur_page.layout_ptr());
		if(cur_page.layout_ptr()->_magic.performance.record_count() == 0) return -1;
		std::deque<page_index_t> pages;
		bool reach_leaf = false;
		int big_key_count = 0;
		char key_buf[16];
		if(cur_page.get_root_index_idx())
			pages.push_back(cur_page.get_root_index_idx());
		while(!pages.empty())
		{
			file->read(pages.front(), cur_page.layout_ptr());
			pages.pop_front();
			index_hdr *it = cur_page.index_left();
			for(index_hdr *ie = cur_page.index_right(); it != ie; ++it)
			{
				if(!reach_leaf && !it->l_child()) reach_leaf = true;
				if(!reach_leaf) pages.push_back(it->l_child());
				if(int key_len = extract_key(it, key_buf))
					query->update(key_buf, key_len);
				else
					big_key_count++;
			}
			if(!reach_leaf) pages.push_back(it->l_child());
		}
		return big_key_count;
	}
};

class PageRebuild
{
	class PageWriter : public IQueryData
	{
		PageCache *cache;
		Logger    *logger;
		size_t	  count;
		size_t    cache_max;
	public:
		~PageWriter()
		{
			cache->snapshot_create(); logger->prepare(); logger->commit(time32_now()); cache->snapshot_release();
			delete cache;
			delete logger;
		}

		PageWriter(const char *dst, size_t max) : 
			cache(new PageCache(dst, max, max / 2)), logger(new NullLogger(cache)), count(0), cache_max(max)
		{
		}

		bool update(const void *key, size_t key_len, const void *val, size_t val_len)
		{
			cache->put(key, key_len, val, val_len);
			if(++count % cache_max == 0)
			{
				cache->snapshot_create(); logger->prepare(); logger->commit(time32_now()); cache->snapshot_release();
			}
			return true;
		}

		size_t counter() const { return count; }
	};

	PageBrowser *browser;
	PageWriter  *writer;

public:
	~PageRebuild()
	{
		delete writer;
		delete browser;
	}
	PageRebuild(const char *dst, const char *src, size_t max = 1024) : browser(new PageBrowser(src, max)), writer(new PageWriter(dst, max)) { }

	size_t action(size_t *corrupt_count)
	{
		size_t c = browser->action(writer);
		if(corrupt_count) *corrupt_count = c;
		return writer->counter();
	}
};

}

#endif
