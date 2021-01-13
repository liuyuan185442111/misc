#ifndef _L_PAGEHASH_H
#define _L_PAGEHASH_H

#include "page.h"

namespace lcore {

class PageHash
{
	size_t	lru; //next lru sequence number
	size_t	count; //current element count
	size_t	bucket_mask; //bucket_size - 1
	size_t	snapshot_size, snapshot_capacity;
	size_t	cache_high, cache_low;
	Page**	bucket;
	Page**	snapshot;
	PageFile *page_file;

	Page* load_page(page_index_t idx);
	void erase_page(Page *page);
	Page* lru_adjust(Page *page);

public:
	PageHash() : lru(0), count(0), bucket_mask(15), snapshot_size(0), snapshot_capacity(0), bucket(NULL), snapshot(NULL) { }
	~PageHash();
	void init(PageFile *file, size_t high, size_t low);
	PageFile *pagefile() { return page_file; }
	size_t size() const { return count; }
	Page** snapshot_reference(size_t& size) { size = snapshot_size; return snapshot; };
	size_t snapshot_create(Page *magic);
	void snapshot_release();
	Page* insert(Page *page);
	Page* find(page_index_t page_index);
	void cleanup();
	void clear();
};

} //namespace lcore

#endif
