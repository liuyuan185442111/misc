#include "pagehash.h"

namespace lcore {

Page* PageHash::load_page(page_index_t idx)
{
	Page *page = new Page(idx);
	try
	{
		page_file->read(idx, page->layout_ptr());
		//assert(idx == *(page_index_t *)(page->layout_ptr() + 1));
		page->set_type(*((page_index_t *)(page->layout_ptr() + 1) + 1));
		*(Page **)(page->layout_ptr() + 1) = page;
	}
	catch(const PageFile::Exception &e)
	{
		delete page;
		return NULL;
	}
	return insert(page);
}

void PageHash::erase_page(Page *page)
{
	Page **it = bucket + (page->index() & bucket_mask);
	if(page == *it) *it = page->get_next_hash_equal();
	else for(Page *next_next, *next = *it; next; next = next_next)
	{
		if((next_next = next->get_next_hash_equal()) == page)
		{
			next->set_next_hash_equal(page->get_next_hash_equal());
			break;
		}
	}
	count--;
	delete page;
}

Page* PageHash::lru_adjust(Page *page)
{
	page->set_lru(lru);
	if(++lru == 0)
	{
		Page **tmp = (Page **)malloc(count * sizeof(Page *)), **it = tmp, **ii = tmp;
		for(Page** it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it)
			for(Page *page = *it ; page ; page = page->get_next_hash_equal())
				*ii++ = page;
		Page::sort_lru(it, ii);
		for(lru = 0; it != ii; ++it)
			(*it)->set_lru(lru++);
		free(tmp);
	}
	return page;
}

void PageHash::cleanup()
{
	if(count <= cache_high) return;
	if(count <= cache_low) return;
	Page **tmp = (Page **)malloc(count * sizeof(Page *)), **it = tmp, **ii = tmp;
	for(Page** it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it)
		for(Page *page = *it ; page ; page = page->get_next_hash_equal())
			*ii++ = page;
	Page::sort_lru(it, ii);
	for(lru = 0; count > cache_low && it != ii; ++it)
	{
		if((*it)->is_clean()) erase_page(*it);
		else (*it)->set_lru(lru++);
	}
	for(; it != ii; ++it) (*it)->set_lru(lru++);
	free(tmp);
}

PageHash::~PageHash()
{
	clear();
	free(bucket);
	free(snapshot);
}

void PageHash::init(PageFile *file, size_t high, size_t low)
{
	page_file = file;
	cache_high = high;
	cache_low = low;
	if(cache_high > 1<<24) exit(1); 
	size_t bucket_size = bucket_mask + 1;
	while(bucket_size < cache_high) bucket_size <<= 1;
	bucket_mask = bucket_size - 1;
	bucket = (Page **)malloc(bucket_size * sizeof(Page *));
	memset(bucket, 0, bucket_size * sizeof(Page *));
}

size_t PageHash::snapshot_create(Page *magic)
{
	if(snapshot_capacity < count + 1)
	{
		free(snapshot);
		snapshot = (Page **)malloc((snapshot_capacity = count + 1) * sizeof(Page *));
	}
	snapshot_size = 0;
	for(Page **it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it)
		for(Page *page = *it ; page ; page = page->get_next_hash_equal())
			if(page->set_snapshot())
				snapshot[snapshot_size++] = page;
	if(snapshot_size)
		magic->set_dirty();
	if(magic->set_snapshot())
		snapshot[snapshot_size++] = magic;
	return snapshot_size;
}

void PageHash::snapshot_release()
{
	for(Page **it = snapshot, **ie = snapshot + snapshot_size; it != ie; ++it)
		(*it)->clr_snapshot();
	cleanup();
}

Page* PageHash::insert(Page *page)
{
	struct {
		void operator()(Page *_page, Page** _bucket, size_t _mask)
		{
			Page** it = _bucket + (_page->index() & _mask);
			_page->set_next_hash_equal(*it);
			*it = _page;
		}
	} inserter;
	inserter(page, bucket, bucket_mask);
	if(count++ == bucket_mask)
	{
		size_t tmp_mask = ((bucket_mask + 1) << 1) - 1;
		Page** tmp = (Page **)malloc((tmp_mask + 1) * sizeof(Page *));
		memset(tmp, 0, (tmp_mask + 1) * sizeof(Page *));
		for(Page **it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it)
		{
			for(Page *next, *page = *it ; page ; page = next)
			{
				next = page->get_next_hash_equal();
				inserter(page, tmp, tmp_mask);
			}
		}
		free(bucket);
		bucket      = tmp;
		bucket_mask = tmp_mask;
	}
	return lru_adjust(page);
}

Page* PageHash::find(page_index_t page_index)
{
	for(Page *page = bucket[page_index & bucket_mask]; page; page = page->get_next_hash_equal())
		if(page->index() == page_index)
			return lru_adjust(page);
	return load_page(page_index);
}

void PageHash::clear()
{
	for(Page **it = bucket, **ie = bucket + bucket_mask + 1; it != ie; ++it)
	{
		if(Page *page = *it)
		{
			*it = NULL;
			Page *next;
			do {
				next = page->get_next_hash_equal();
				delete page;
			} while((page = next));
		}
	}
	count = 0;
}

} //namespace lcore
