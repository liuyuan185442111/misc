#include "pagecache.h"

namespace lcore {

PageCache::PageCache(const char *file_name, size_t cache_high, size_t cache_low) : magic(0)
{
	file = new PageFile(file_name);
	hash.init(file, cache_high, cache_low);
	try { file->read(0, magic.layout_ptr()); *(Page **)(magic.layout_ptr() + 1) = &magic; }
	catch(const PageFile::Exception &e) { magic.init_magic(); magic.set_root_index_idx(alloc_page()->index()); }
	Performance *perf = performance();
	perf->set_cache_high(cache_high);
	perf->set_cache_low(cache_low);
	perf->reset_peak();
	perf->set_page_read(0);
	perf->set_page_write(0);
	perf->set_page_sync(0);
}

void* PageCache::extract_data(data_hdr *hdr, size_t &size, void *data_buf)
{
	size_t copy_len = *(size32_t *)(hdr + 1);
	byte_t *data = (byte_t *)(copy_len <= size ? data_buf : malloc(copy_len));
	size = copy_len;
	byte_t *p = data;
	while(true)
	{
		memcpy(p, hdr + 1, hdr->size);
		p += hdr->size;
		if(!hdr->next_page_index)
			break;
		hdr = Rnext_page_header(hdr);
	}
	return data;
}

void PageCache::extract_key_copyer(data_hdr *hdr, byte_t *dest, size_t copy_len)
{
	size_t min = hdr->size - sizeof(size32_t) - sizeof(size32_t);
	if(copy_len <= min)
		memcpy(dest, (size32_t *)(hdr + 1) + 2, copy_len);
	else
	{
		memcpy(dest, (size32_t *)(hdr + 1) + 2, min);
		dest     += min;
		copy_len -= min;
		while(copy_len)
		{
			hdr       = Rnext_page_header(hdr);
			min       = copy_len < hdr->size ? copy_len : hdr->size;
			memcpy(dest, hdr + 1, min);
			dest     += min;
			copy_len -= min;
		}
	}
}

void* PageCache::extract_key(data_hdr *hdr, size_t &key_len)
{
	key_len = *((size32_t *)(hdr + 1) + 1);
	byte_t *key = (byte_t *)malloc(key_len);
	extract_key_copyer(hdr, key, key_len);
	return key;
}

void* PageCache::extract_key(data_hdr *hdr, size_t &key_len, void *key_buf)
{
	size_t copy_len = *((size32_t *)(hdr + 1) + 1);
	byte_t *key = (byte_t *)(copy_len <= key_len ? key_buf : malloc(copy_len));
	key_len = copy_len;
	extract_key_copyer(hdr, key, key_len);
	return key;
}

void* PageCache::extract_key(index_hdr *hdr, size_t &key_len)
{
	if(hdr->key_len)
	{
		key_len = hdr->key_len;
		void *key = malloc(key_len);
		memcpy(key, hdr->key, key_len);
		return key;
	}
	return extract_key(Rindex_hdr2data_hdr(hdr), key_len);
}

void* PageCache::extract_key(index_hdr *hdr, size_t &key_len, void *key_buf)
{
	if(hdr->key_len)
	{
		void *key = hdr->key_len <= key_len ? key_buf : malloc(hdr->key_len);
		key_len = hdr->key_len;
		memcpy(key, hdr->key, key_len);
		return key;
	}
	return extract_key(Rindex_hdr2data_hdr(hdr), key_len, key_buf);
}

void PageCache::extract_val_copyer(data_hdr *hdr, byte_t *dest, size_t pass_len)
{
	for( ; pass_len > hdr->size; hdr = Rnext_page_header(hdr))
		pass_len -= hdr->size;
	memcpy(dest, (byte_t *)(hdr + 1) + pass_len, hdr->size - pass_len);
	dest += hdr->size - pass_len;
	for( ; hdr->next_page_index; dest += hdr->size)
	{
		hdr = Rnext_page_header(hdr);
		memcpy(dest, hdr + 1, hdr->size);
	}
}

void* PageCache::extract_val(data_hdr *hdr, size_t &val_len)
{
	size_t size    = *(size32_t *)(hdr + 1);
	size_t key_len = *((size32_t *)(hdr + 1) + 1);
	byte_t *val    = (byte_t *)malloc(val_len = size - key_len - sizeof(size32_t) - sizeof(size32_t));
	extract_val_copyer(hdr, val, key_len + sizeof(size32_t) + sizeof(size32_t));
	return val;
}

void* PageCache::extract_val(data_hdr *hdr, size_t &val_len, void *val_buf)
{
	size_t size     = *(size32_t *)(hdr + 1);
	size_t key_len  = *((size32_t *)(hdr + 1) + 1);
	size_t copy_len = size - key_len - sizeof(size32_t) - sizeof(size32_t);
	byte_t *val     = (byte_t *)(val_buf && copy_len <= val_len ? val_buf :  malloc(copy_len));
	val_len         = copy_len;
	extract_val_copyer(hdr, val, key_len + sizeof(size32_t) + sizeof(size32_t));
	return val;
}

void PageCache::free_data(data_hdr *hdr)
{
	PageCache *cache = this;
	do
	{
		hdr->size = 0;
		data_hdr *next, *cur = (data_hdr *)((ptrdiff_t)hdr & ~PAGEMASK);
		if(hdr != cur)
		{
			for(next = cur->next_header(); next != hdr; next = next->next_header())
				cur = next;
			if(cur->size == 0)
			{
				cache->clr_fragment((frag_hdr *)cur);
				cur->next = next->next;
			}
			else
				cur = next;
		}
		if((next = cur->next_header()) && next->size == 0)
		{
			cache->clr_fragment((frag_hdr *)next);
			cur->next = next->next;
		}
		hdr = hdr->next_page_index ? Wnext_page_header(hdr) : NULL;
		cache->set_fragment((frag_hdr *)cur);
	} while(hdr);
}

int PageCache::compare_key(index_hdr *hdr, const void *key, size_t key_len)
{
	if(size_t i_key_len = hdr->key_len)
	{
		if(int comp = memcmp(key, hdr->key, std::min(key_len, i_key_len)))
			return comp;
		return key_len - i_key_len;
	}
	if(key_len <= sizeof(hdr->key))
	{
		if(int comp = memcmp(key, hdr->key, key_len))
			return comp;
		return -1;
	}
	if(int comp = memcmp(key, hdr->key, sizeof(hdr->key)))
		return comp;

	data_hdr *ddr    = Rindex_hdr2data_hdr(hdr);
	size_t i_key_len = *((size32_t *)(ddr + 1) + 1);
	size_t cmp_len   = std::min(key_len, i_key_len);
	size_t min       = ddr->size - sizeof(size32_t) - sizeof(size32_t);
	byte_t *i_key    = (byte_t *)(ddr + 1) + sizeof(size32_t) + sizeof(size32_t);
	if(cmp_len <= min)
	{
		if(int comp = memcmp(key, i_key, cmp_len))
			return comp;
		return key_len - i_key_len;
	}
	if(int comp = memcmp(key, i_key, min))
		return comp;
	while(cmp_len -= min)
	{
		key = (byte_t *)key + min;
		ddr = Rnext_page_header(ddr);
		min = cmp_len < ddr->size ? cmp_len : ddr->size;
		if(int comp = memcmp(key, ddr + 1, min))
			return comp;
	}
	return key_len - i_key_len;
}

bool PageCache::put(const void *key, size_t key_len, const void *val, size_t val_len, bool replace)
{
	if(index_hdr *found = put_new_node(key, key_len, val, val_len))
	{
		if(!replace)
		{
			performance()->insert_reject();
			return false;
		}
		put_replace(found, key, key_len, val, val_len, false);
	}
	return true; 
}

void* PageCache::put(const void *key, size_t key_len, const void *val, size_t& val_len)
{
	if(index_hdr *found = put_new_node(key, key_len, val, val_len))
		return put_replace(found, key, key_len, val, val_len, true);
	return NULL; 
}

void* PageCache::find(const void *key, size_t key_len, size_t &val_len)
{
	index_hdr *hdr = find(key, key_len);
	return hdr ? extract_val(hdr, val_len) : NULL;
}

void* PageCache::find(const void *key, size_t key_len, size_t &val_len, void *val_buf)
{
	index_hdr *hdr = find(key, key_len);
	return hdr ? extract_val(hdr, val_len, val_buf) : NULL;
}

void* PageCache::del(const void *key, size_t key_len, size_t& val_len)
{
	if(index_hdr *hdr = find(key, key_len))
	{
		performance()->remove_found();
		data_hdr *origin_hdr = Windex_hdr2data_hdr(hdr);
		void     *origin_val = extract_val(origin_hdr, val_len);
		free_data(origin_hdr);
		remove(hdr);
		return origin_val;
	}
	performance()->remove_not_found();
	return NULL;
}

bool PageCache::del(const void *key, size_t key_len)
{
	if(index_hdr *hdr = find(key, key_len))
	{
		performance()->remove_found();
		free_data(Windex_hdr2data_hdr(hdr));
		remove(hdr);
		return true;
	}
	performance()->remove_not_found();
	return false;
}

void* PageCache::first_key(size_t &key_len)
{
	index_hdr *hdr = left_most(root_index_page());
	return hdr ? extract_key(hdr, key_len) : NULL;
}

void* PageCache::next_key(const void *key, size_t &key_len)
{
	if(index_hdr *hdr = greater_key(key, key_len))
		return extract_key(hdr, key_len);
	return NULL;
}

void PageCache::snapshot_release()
{
	Performance *perf = performance();
	PageFile *pagefile = hash.pagefile();
	perf->set_page_read (pagefile->read_counter());
	perf->set_page_write(pagefile->write_counter());
	perf->set_page_sync (pagefile->sync_counter());
	perf->set_cache_peak(hash.size());
	hash.snapshot_release();
}

void PageCache::walk(index_hdr *hdr, IQueryKey *query)
{
	page_index_t lastPageIndex = 0;
	byte_t key_buf[WALKBUFFERSIZE];
	for(bool r = true; r && hdr; hdr = next_index_header(hdr))
	{
		page_index_t curPageIndex = Header2PageIndex(hdr);
		if(curPageIndex != lastPageIndex)
		{
			lastPageIndex = curPageIndex;
			hash.cleanup();
		}

		size_t key_len = sizeof(key_buf);
		void*  key     = extract_key(hdr, key_len, key_buf);
		r = query->update(key, key_len);
		if(key != key_buf) ::free(key);
	}
}

void PageCache::walk(index_hdr *hdr, IQueryData *query)
{
	page_index_t lastPageIndex = 0;
	byte_t data_buf[WALKBUFFERSIZE];
	for(bool r = true; r && hdr; hdr = next_index_header(hdr))
	{
		page_index_t curPageIndex = Header2PageIndex(hdr);
		if(curPageIndex != lastPageIndex)
		{
			lastPageIndex = curPageIndex;
			hash.cleanup();
		}

		//不要在query->update中直接或间接调用hash.cleanup(), 比如checkpoint-->clr_snapshot-->cleanup这个路径,
		//如果data_len太大读入Page会非常多, 可能会导致当前hdr所在的Page被清理掉
		size_t data_len = sizeof(data_buf);
		void*  data     = extract_data(hdr, data_len, data_buf);
		size_t key_len  = *((size32_t *)data + 1);
		size_t val_len  = *(size32_t *)data - key_len - sizeof(size32_t) - sizeof(size32_t);
		void*  key      = (byte_t *)data + sizeof(size32_t) + sizeof(size32_t);
		void*  val      = (byte_t *)key + key_len;
		r = query->update(key, key_len, val, val_len);
		if(data != data_buf) ::free(data);
	}
}

index_hdr* PageCache::find_key(const void *key, size_t key_len, int& pos)
{
	performance()->find_key();
	Page *page = root_index_page();
	if(size_t size = page->index_size())
		while(true)
		{
			index_hdr *hdr = page->index_left();
			do
			{
				int half = size >> 1;
				pos = compare_key(hdr + half, key, key_len);
				if(pos > 0)
				{
					size -= half + 1;
					hdr  += half + 1;
				}
				else if(pos < 0)
					size  = half;
				else
					return hdr + half;
			} while(size);
			if(page_index_t page_index = hdr->l_child())
			{
				page = Rload_page(page_index);
				size = page->index_size();
			}
			else
				return hdr;
		}
	return NULL;
}

index_hdr* PageCache::find(const void *key, size_t key_len)
{
	int pos;
	index_hdr *hdr = find_key(key, key_len, pos);
	return (hdr && !pos) ? hdr : NULL;
}

index_hdr* PageCache::left_most(Page *page)
{
	index_hdr *bgn = page->index_left();
	while(bgn->l_child())
		bgn = Rload_page(bgn->l_child())->index_left();
	return bgn < Header2Page(bgn)->index_right() ? bgn : NULL;
}

void PageCache::clr_fragment(frag_hdr *hdr)
{
	if(hdr->prev_page_index)
	{
		frag_hdr *frag_prev = Wprev_page_header(hdr);
		if(hdr->next_page_index)
		{
			frag_hdr *frag_next = Wnext_page_header(hdr);
			frag_prev->set_next(Header2PageIndex(frag_next), header_offset(frag_next));
			frag_next->set_prev(Header2PageIndex(frag_prev), header_offset(frag_prev));
		}
		else
			frag_prev->set_next(0, 0);
	}
	else
	{
		int i = ((data_hdr *)hdr)->capacity() / FRAGSTEP;
		if(hdr->next_page_index)
		{
			frag_hdr *frag_next = Wnext_page_header(hdr);
			magic.set_frag_page_position(i, Header2PageIndex(frag_next), header_offset(frag_next));
			frag_next->set_prev(0, 0);
		}
		else
			magic.set_frag_page_position(i, 0, 0);
	}
}

void PageCache::adjust_data_head(data_hdr *hdr)
{
	//a frag at least needs 16 bytes capacity, because data_hdr.size is at least 9(sizeof(size32_t)+sizeof(size32_t)+1)
	if(hdr->capacity() >= ((sizeof(data_hdr) + hdr->size + 15) & ~15) + sizeof(data_hdr) + 16)
	{
		data_hdr *ndr = (data_hdr *)((ptrdiff_t)((byte_t *)(hdr + 1) + hdr->size + 15) & ~(ptrdiff_t)15);
		data_hdr *next = hdr->next_header();
		if(next && next->size == 0)
		{
			clr_fragment((frag_hdr *)next);
			ndr->next = next->next;
		}
		else
			ndr->next = hdr->next;
		ndr->size = 0;
		set_fragment((frag_hdr *)ndr);
		hdr->next = header_offset(ndr);
	}
}

Page *PageCache::alloc_page(bool index_page)
{
	performance()->page_alloc();
	Page *page;
	if(page_index_t idx = magic.get_free_page_list())
	{
		page = hash.find(idx);
		magic.set_free_page_list(page->get_free_page_list());
	}
	else
		page = hash.insert(new Page(magic.extend_page()));
	memset(page->layout_ptr(), 0, sizeof(PageLayout));
	page->set_type(index_page ? INDEX_PAGE : DATA_PAGE);
	return page->set_dirty();
}

void PageCache::free_page(Page *page)
{
	performance()->page_free();
	page->set_type(FREE_PAGE);
	page->set_free_page_list(magic.get_free_page_list());
	magic.set_free_page_list(page->index());
}

Page *PageCache::alloc_root_index_page()
{
	Page *page = alloc_page();
	magic.set_root_index_idx(page->index());
	return page;
}

void PageCache::set_fragment(frag_hdr *hdr)
{
	page_pos_t capacity = ((data_hdr *)hdr)->capacity();
	if(capacity == PAGEMAXSPARE)
	{
		free_page(Header2Page(hdr));
		return;
	}
	int i = capacity / FRAGSTEP;
	page_index_t next_page_index = magic.get_frag_page_index(i);
	page_pos_t   next_page_pos   = magic.get_frag_page_pos(i);
	page_index_t curr_page_index = Header2PageIndex(hdr);
	page_pos_t   curr_page_pos   = header_offset(hdr);
	if(next_page_index)
		((frag_hdr*)Wload_hdr(next_page_index, next_page_pos))->set_prev(curr_page_index, curr_page_pos);
	hdr->set_next(next_page_index, next_page_pos);
	hdr->set_prev(0, 0);
	magic.set_frag_page_position(i, curr_page_index, curr_page_pos);
}

data_hdr* PageCache::__alloc_data_head(size_t size)
{
	struct {
		data_hdr* operator()(PageCache *_cache, data_hdr *_hdr, size_t _hdr_size)
		{
			if(_hdr_size < PAGEMAXSPARE)
			{
				_hdr->size = _hdr_size;
				_cache->adjust_data_head(_hdr);
			}
			else
				_hdr->size = PAGEMAXSPARE;
			return _hdr;
		}
	} adjuster;
	if(size < PAGEMAXSPARE)
	{
		for(int i = (size + FRAGSTEP - 1) / FRAGSTEP, e = PAGEUSED / FRAGSTEP; i < e; i++)
		{
			if(page_index_t next_page_index = magic.get_frag_page_index(i))
			{
				frag_hdr *hdr = (frag_hdr *)Wload_hdr(next_page_index, magic.get_frag_page_pos(i));
				magic.set_frag_page_position(i, next_page_index = hdr->next_page_index, hdr->next_page_pos);
				if(next_page_index)
					((frag_hdr *)Wload_hdr(next_page_index, hdr->next_page_pos))->set_prev(0, 0);
				return adjuster(this, (data_hdr *)hdr, size);
			}
		}
	}
	data_hdr *hdr = alloc_page(false)->layout_ptr()->_data;
	hdr->next     = PAGEUSED;
	return adjuster(this, hdr, size);
}

data_hdr* PageCache::alloc_data_head(index_hdr &idx, size_t size)
{
	data_hdr *hdr = __alloc_data_head(size);
	hdr->first_slice = 1;
	idx.set_data_position(Header2PageIndex(hdr), header_offset(hdr));
	return hdr;
}

data_hdr* PageCache::alloc_data_head(data_hdr *pdr, size_t size)
{
	data_hdr *hdr = __alloc_data_head(size);
	hdr->first_slice = 0;
	pdr->set_next(Header2PageIndex(hdr), header_offset(hdr));
	return hdr;
}

void PageCache::create_node(index_hdr& index, const void *key, size_t key_len, const void *val, size_t val_len)
{
	if(key_len > sizeof(index.key))
	{
		memcpy(index.key, key, sizeof(index.key));
		index.key_len = 0;
	}
	else
	{
		memcpy(index.key, key, key_len);
		index.key_len = key_len;
	}
	size_t    size = key_len + val_len + sizeof(size32_t) + sizeof(size32_t);
	data_hdr  *hdr = alloc_data_head(index, size);
	byte_t      *p = (byte_t *)(hdr + 1);
	*(size32_t *)p = size;    p += sizeof(size32_t);
	*(size32_t *)p = key_len; p += sizeof(size32_t);
	if(hdr->size < size)
	{
		size -= sizeof(size32_t) + sizeof(size32_t);
		size_t capacity = hdr->size - sizeof(size32_t) - sizeof(size32_t);
		while(capacity < key_len)
		{
			memcpy(p, key, capacity);
			key      = (byte_t *)key + capacity;
			key_len -= capacity;
			size    -= capacity;
			hdr      = alloc_data_head(hdr, size);
			capacity = hdr->size;
			p        = (byte_t *)(hdr + 1);
		}
		memcpy(p, key, key_len);
		p        += key_len;
		capacity -= key_len;
		while(capacity < val_len)
		{
			memcpy(p, val, capacity);
			val      = (byte_t *)val + capacity;
			val_len -= capacity;
			hdr      = alloc_data_head(hdr, val_len);
			capacity = hdr->size;
			p        = (byte_t *)(hdr + 1);
		}
	}
	else
	{
		memcpy(p, key, key_len); p += key_len;
	}
	memcpy(p, val, val_len);
	hdr->set_next(0, 0);
}

index_hdr* PageCache::insert_init_node()
{
	Page *root_page = root_index_page();
	root_page->set_dirty();
	root_page->index_l_pos() = INDEXMINC;
	root_page->index_r_pos() = INDEXMINC + 1;
	index_hdr *bgn = root_page->index_left();
	bgn->l_child() = bgn->r_child() = 0;
	return bgn;
}

index_hdr* PageCache::put_new_node(const void *key, size_t key_len, const void *val, size_t val_len)
{
	int pos;
	if(index_hdr *found = find_key(key, key_len, pos))
	{
		if(pos == 0) return found;
		index_hdr index;
		create_node(index, key, key_len, val, val_len);
		insert_leaf(found, &index);
	}
	else
		create_node(*insert_init_node(), key, key_len, val, val_len);
	performance()->insert();
	return NULL;
}

void* PageCache::put_replace(index_hdr* found, const void *key, size_t key_len, const void *val, size_t& save_val_len, bool need_origin)
{
	performance()->insert_replace();
	data_hdr *hdr    = Windex_hdr2data_hdr(found);
	size_t val_len   = save_val_len;
	void* origin_val = need_origin ? extract_val(hdr, save_val_len) : NULL;
	size_t pass_len  = key_len + sizeof(size32_t) + sizeof(size32_t);
	size_t size      = pass_len + val_len;
	size_t capacity  = hdr->capacity();
	if(capacity < size && capacity < PAGEMAXSPARE)
	{
		Header2Page(found)->set_dirty();
		free_data(hdr);
		create_node(*found, key, key_len, val, val_len);
		return origin_val;
	}
	*(size32_t *)(hdr + 1) = size;
	for( ; pass_len > hdr->size; hdr = Rnext_page_header(hdr)) pass_len -= hdr->size;
	Header2Page(hdr)->set_dirty();
	byte_t *p     = (byte_t *)(hdr + 1) + pass_len;
	data_hdr *ndr = NULL;
	capacity      = hdr->capacity() - pass_len;
	if(val_len <= capacity)
	{
		memcpy(p, val, val_len);
		hdr->size = pass_len + val_len;
		if(hdr->next_page_index) ndr = Wnext_page_header(hdr);
		adjust_data_head(hdr);
	}
	else
	{
		memcpy(p, val, capacity);
		hdr->size = pass_len + capacity;
		val       = (byte_t *)val + capacity;
		val_len  -= capacity;
		while(hdr->next_page_index && (capacity = (ndr = Wnext_page_header(hdr))->size) < val_len && capacity == PAGEMAXSPARE)
		{
			hdr      = ndr;
			memcpy(hdr + 1, val, capacity);
			val      = (byte_t *)val + capacity;
			val_len -= capacity;
		}
		if(hdr->next_page_index == 0) ndr = NULL;
		if(ndr && (capacity = ndr->capacity()) >= val_len)
		{
			hdr       = ndr;
			ndr       = hdr->next_page_index ? Wnext_page_header(hdr) : NULL;
			memcpy(hdr + 1, val, val_len);
			hdr->size = val_len;
			adjust_data_head(hdr);
		}
		else while(val_len)
		{
			hdr       = alloc_data_head(hdr, val_len);
			capacity  = hdr->size;
			memcpy(hdr + 1, val, capacity);
			val       = (byte_t *)val + capacity;
			val_len  -= capacity;
		}
	}
	if(ndr) free_data(ndr);
	hdr->set_next(0, 0);
	return origin_val;
}

void PageCache::insert_leaf(index_hdr *pos, index_hdr *val)
{
	Page *page = Header2Page(pos);
	page->set_dirty();
	//左边到头了 or (pos在中间位置的右边 and 右边没到头)  此时往右移的话移动的元素较少
	if(page->index_l_pos() == 0 || (page->index_m_pos() < page->index_offset(pos) && page->index_r_pos() < INDEXCOUNT))
	{
		performance()->insert_leaf_adjust_right();
		for(index_hdr *tail = page->index_right(); tail > pos; tail--)
			copy_key(tail, tail - 1);
		page->index_r_pos() ++;
	}
	else
	{
		performance()->insert_leaf_adjust_left();
		for(index_hdr *bgn = page->index_left(); bgn < pos; bgn++)
			copy_key(bgn - 1, bgn);
		pos--;
		page->index_l_pos() --;
	}
	copy_key(pos, val);
	if(page->index_size() == INDEXCOUNT)
		split(page);
}

void PageCache::split(Page *page)
{
	performance()->split();
	page_pos_t left_count = (INDEXCOUNT >> 1);
	page_pos_t right_count = INDEXCOUNT - left_count - 1;
	index_hdr *mid = page->index_pos(left_count);
	page->index_r_pos() = left_count;

	Page *parent = page->parent_index() ? Wload_page(page->parent_index()) : alloc_root_index_page();
	Page *sibling = alloc_page();
	sibling->index_l_pos() = SHRINK_SHIFT;
	sibling->index_r_pos() = SHRINK_SHIFT + right_count;
	index_hdr *sibling_bgn = sibling->index_pos(SHRINK_SHIFT);
	memcpy(sibling_bgn, mid + 1, right_count * sizeof(index_hdr) + sizeof(page_index_t));
	if(sibling_bgn->l_child())
	{
		page_index_t sibling_index = sibling->index();
		for(page_pos_t i = 0; i <= right_count; i++)
			Wload_page((sibling_bgn + i)->l_child())->set_parent(sibling_index, SHRINK_SHIFT + i);
	}
	insert_internal(parent, parent->index_pos(page->parent_pos()), mid, page, sibling);
}

void PageCache::remove(index_hdr *pos)
{
	if(pos->l_child()) remove_internal(pos);
	else remove_leaf(pos);
}

void PageCache::remove_internal(index_hdr *pos)
{
	Header2Page(pos)->set_dirty();
	performance()->remove_internal();
	//right_most of left child
	index_hdr *right_most;
	for(right_most = Rload_page(pos->l_child())->index_right() - 1; right_most->r_child(); right_most = Rload_page(right_most->r_child())->index_right() - 1);
	copy_key(pos, right_most);
	remove_leaf(right_most);
}

void PageCache::remove_leaf(index_hdr *pos)
{
	Page *page = Header2Page(pos);
	page->set_dirty();
	if(page->index_m_pos() < page->index_offset(pos))
	{
		for(index_hdr *tail = page->index_right() - 1; pos < tail; pos++)
			copy_key(pos, pos + 1);
		page->index_r_pos() --;
	}
	else
	{
		for(index_hdr *bgn = page->index_left(); pos > bgn; pos--)
			copy_key(pos, pos - 1);
		page->index_l_pos() ++;
	}
	if(page->index_size() < INDEXMINC && page->parent_index())
	{
		Page *parent = Wload_page(page->parent_index());
		index_hdr *parent_hdr = parent->index_pos(page->parent_pos());
		if(parent->index_r_pos() == page->parent_pos())//page have no right sibling
			l_shrink_leaf(page, parent_hdr - 1);//try steal one key from left sibling
		else
			r_shrink_leaf(page, parent_hdr);//try steal one key from right sibling
	}
}

index_hdr* PageCache::next_index_header(index_hdr *cur)
{
	//右子树的最小值
	if(cur->r_child())
		return left_most(Rload_page(cur->r_child()));
	//处于叶子节点, 右边的元素
	Page *page = Header2Page(cur);
	if(++cur < page->index_right())
		return cur;
	//处于叶子节点最右边
	index_hdr *end = page->index_end();
	while(end->parent_index)
	{
		Page *parent = Rload_page(end->parent_index);
		if(end->parent_pos < parent->index_r_pos())
			return parent->index_pos(end->parent_pos);
		end = parent->index_end();
	}
	return NULL;
}

index_hdr* PageCache::greater_key(const void *key, size_t key_len)
{
	int pos = 0;
	if(index_hdr *hdr = find_key(key, key_len, pos))
	{
		if(pos == 0) // equals found.
			return next_index_header(hdr); // return NULL if the key is last. see next_index_header.
		if(hdr != Header2Page(hdr)->index_right()) // greater found. maybe tail. see great_equal_key.
			return hdr;
	}
	return NULL;
}

index_hdr* PageCache::great_equal_key(const void *key, size_t key_len)
{
	int pos;
	index_hdr* hdr = find_key(key, key_len, pos); 
	return (hdr && hdr == Header2Page(hdr)->index_right()) ? NULL : hdr;
}

void PageCache::merge_sibling(Page *left, index_hdr *parent_hdr, Page *right)
{
	performance()->merge_sibling();
	index_hdr *sibling_bgn  = right->index_left();
	page_pos_t sibling_size = right->index_size();
	//left所有元素左移
	if(INDEXCOUNT < left->index_r_pos() + sibling_size + 1)
	{
		performance()->merge_sibling_adjust();
		index_hdr *cur = left->index_pos(0);
		index_hdr *bgn = left->index_left();
		if(bgn->l_child())
		{
			page_pos_t page_pos = 0;
			for(index_hdr *tail = left->index_right(); bgn < tail; cur++, bgn++)
			{
				copy_key_and_left(cur, bgn);
				Wload_page(cur->l_child())->parent_pos() = page_pos++;
			}
			Wload_page(cur->l_child() = bgn->l_child())->parent_pos() = page_pos;
		}
		else
			for(index_hdr *tail = left->index_right(); bgn < tail; copy_key(cur++, bgn++));
		left->index_r_pos() = left->index_size();
		left->index_l_pos() = 0;
	}
	Page *parent = Header2Page(parent_hdr);
	index_hdr *tail = left->index_right();
	copy_key(tail, parent_hdr);
	memcpy(tail + 1, sibling_bgn, sibling_size * sizeof(index_hdr) + sizeof(page_index_t));
	left->index_r_pos() += sibling_size + 1;
	free_page(right);
	if((++tail)->l_child())
	{
		page_index_t page_index = left->index();
		page_pos_t   page_pos   = left->index_offset(tail);
		for(index_hdr *new_tail = left->index_right() + 1; tail < new_tail; tail++)
			Wload_page(tail->l_child())->set_parent(page_index, page_pos++);
	}

	if(parent->index_m_pos() < parent->index_offset(parent_hdr))
	{
		for(index_hdr *parent_tail = parent->index_right() - 1; parent_hdr < parent_tail; parent_hdr++)
		{
			copy_key_and_right(parent_hdr, parent_hdr + 1);
			Wload_page(parent_hdr->r_child())->parent_pos() --;
		}
		parent->index_r_pos() --;
	}
	else
	{
		for(index_hdr *parent_bgn = parent->index_left(); parent_hdr > parent_bgn; parent_hdr--)
		{
			copy_key_and_right(parent_hdr, parent_hdr - 1);
			Wload_page(parent_hdr->l_child())->parent_pos() ++;
		}
		Wload_page(parent_hdr->r_child() = parent_hdr->l_child())->parent_pos() ++;
		parent->index_l_pos() ++;
	}

	if(parent->index_size() < INDEXMINC)
	{
		if(page_index_t ancestry_index = parent->parent_index())
		{
			performance()->merge_sibling_nest();
			Page *ancestry = Wload_page(ancestry_index);
			if(ancestry->index_r_pos() == parent->parent_pos())
				l_shrink_internal(parent, ancestry->index_pos(parent->parent_pos()) - 1);
			else
				r_shrink_internal(parent, ancestry->index_pos(parent->parent_pos()));
		}
		else if(parent->index_size() == 0)
		{
			left->set_parent(0, 0);
			magic.set_root_index_idx(left->index());
			free_page(parent);
		}
	}
}

void PageCache::insert_internal(Page *page, index_hdr *pos, index_hdr *mid, Page *l_child, Page *r_child)
{
	if(page->index_l_pos() == 0 || (page->index_m_pos() < page->index_offset(pos) && page->index_r_pos() < INDEXCOUNT))
	{
		performance()->insert_internal_adjust_right();
		for(index_hdr *tail = page->index_right(); tail > pos; tail--)
		{
			Wload_page(tail->l_child())->parent_pos() ++;
			copy_key_and_right(tail, tail - 1);
		}
		page->index_r_pos() ++;
	}
	else
	{
		performance()->insert_internal_adjust_left();
		for(index_hdr *bgn = page->index_left(); bgn < pos; bgn++)
		{
			Wload_page(bgn->l_child())->parent_pos() --;
			copy_key_and_left(bgn - 1, bgn);
		}
		pos--;
		page->index_l_pos() --;
	}
	copy_key(pos, mid);
	pos->l_child() = l_child->index();
	pos->r_child() = r_child->index();
	l_child->set_parent(page->index(), page->index_offset(pos));
	r_child->set_parent(page->index(), page->index_offset(pos) + 1);
	if(page->index_size() == INDEXCOUNT)
		split(page);
}

void PageCache::r_shrink_internal(Page *page, index_hdr *parent_hdr)
{
	performance()->r_shrink_internal();
	Page *sibling = Wload_page(parent_hdr->r_child());

	if(sibling->index_size() > INDEXMINC)
	{
		index_hdr *sibling_bgn = sibling->index_left();
		index_hdr *tail        = page->index_right();
		if(page->index_r_pos() == INDEXCOUNT)
		{
			performance()->r_shrink_internal_adjust();
			index_hdr *cur = page->index_pos(SHRINK_SHIFT);
			index_hdr *bgn = page->index_left();
			for( ;bgn < tail; bgn++, cur++)
			{
				copy_key_and_left(cur, bgn);
				Wload_page(cur->l_child())->parent_pos() = page->index_offset(cur);
			}
			Wload_page(cur->l_child() = bgn->l_child())->parent_pos() = page->index_offset(cur);
			page->index_l_pos() = SHRINK_SHIFT;
			page->index_r_pos() = SHRINK_SHIFT + INDEXMINC - 1;
			tail = page->index_right();
		}
		copy_key(tail, parent_hdr);
		copy_key(parent_hdr, sibling_bgn);
		Wload_page(tail->r_child() = sibling_bgn->l_child())->set_parent(page->index(), page->index_offset(tail) + 1); 
		page->index_r_pos() ++;
		sibling->index_l_pos() ++;
	}
	else
		merge_sibling(page, parent_hdr, sibling);
}

void PageCache::r_shrink_leaf(Page *page, index_hdr *parent_hdr)
{
	performance()->r_shrink_leaf();
	Page *sibling = Wload_page(parent_hdr->r_child());

	if(sibling->index_size() > INDEXMINC)
	{
		if(page->index_r_pos() == INDEXCOUNT)
		{
			performance()->r_shrink_leaf_adjust();
			for(index_hdr *cur = page->index_pos(SHRINK_SHIFT), *bgn = page->index_left(), *tail = page->index_right(); bgn < tail; copy_key(cur++, bgn++));
			page->index_l_pos() = SHRINK_SHIFT;
			page->index_r_pos() = SHRINK_SHIFT + INDEXMINC - 1;
		}
		copy_key(page->index_right(), parent_hdr);
		copy_key(parent_hdr, sibling->index_left());
		page->index_r_pos() ++;
		sibling->index_l_pos() ++;
	}
	else
		merge_sibling(page, parent_hdr, sibling);
}

void PageCache::l_shrink_internal(Page *page, index_hdr *parent_hdr)
{
	performance()->l_shrink_internal();
	Page *sibling = Wload_page(parent_hdr->l_child());

	if(sibling->index_size() > INDEXMINC)
	{
		if(page->index_l_pos() == 0)
		{
			performance()->l_shrink_internal_adjust();
			index_hdr *tail = page->index_right();
			index_hdr *cur  = tail + SHRINK_SHIFT;
			for(index_hdr *bgn = page->index_left(); tail > bgn;)
			{
				copy_key_and_right(--cur, --tail);
				Wload_page(cur->r_child())->parent_pos() += SHRINK_SHIFT;
			}
			Wload_page(cur->l_child() = tail->l_child())->parent_pos() += SHRINK_SHIFT;
			page->index_l_pos() = SHRINK_SHIFT - 1;
			page->index_r_pos() = SHRINK_SHIFT + INDEXMINC - 1;
		}
		else
			page->index_l_pos() --;
		sibling->index_r_pos() --;
		index_hdr *sibling_tail = sibling->index_right();
		index_hdr *bgn          = page->index_left();
		copy_key(bgn, parent_hdr);
		copy_key(parent_hdr, sibling_tail);
		Wload_page(bgn->l_child() = sibling_tail->r_child())->set_parent(page->index(), page->index_offset(bgn));
	}
	else
		merge_sibling(sibling, parent_hdr, page);
}

void PageCache::l_shrink_leaf(Page *page, index_hdr *parent_hdr)
{
	performance()->l_shrink_leaf();
	Page *sibling = Wload_page(parent_hdr->l_child());

	if(sibling->index_size() > INDEXMINC)
	{
		if(page->index_l_pos() == 0)
		{
			performance()->l_shrink_leaf_adjust();
			for(index_hdr *tail = page->index_right(), *bgn = page->index_left(), *cur = tail + SHRINK_SHIFT; tail > bgn; copy_key(--cur, --tail));
			page->index_l_pos() = SHRINK_SHIFT - 1;
			page->index_r_pos() = SHRINK_SHIFT + INDEXMINC - 1;
		}
		else
			page->index_l_pos() --;
		copy_key(page->index_left(), parent_hdr);
		sibling->index_r_pos() --;
		copy_key(parent_hdr, sibling->index_right());
	}
	else
		//兄弟也没有余粮了
		merge_sibling(sibling, parent_hdr, page);
}

} // namespace lcore
