#ifndef _L_PAGECACHE_H
#define _L_PAGECACHE_H

#include "page.h"
#include "pagehash.h"

namespace lcore {

struct IQueryKey {
	virtual ~IQueryKey() { }
	virtual bool update(const void *key, size_t key_len) = 0;
};

struct IQueryData {
	virtual ~IQueryData() { }
	virtual bool update(const void *key, size_t key_len, const void *val, size_t val_len) = 0;
};

class PageCache
{
	Page      magic;
	PageFile *file;
	PageHash  hash; //for lru, snapshot, and read pages

public:
	PageCache(const char *file_name, size_t cache_high, size_t cache_low);
	~PageCache() { delete file;}
	size_t hashsize() const { return hash.size(); }
	PageFile* pagefile() { return file; }
	PageLayout* magic_layout_ptr() { return magic.layout_ptr(); }
	Performance* performance() { return &magic.layout_ptr()->_magic.performance; }

	//key_len cannot be 0, val_len can be 0
	bool put(const void *key, size_t key_len, const void *val, size_t val_len, bool replace);
	//need free return pointer
	void* put(const void *key, size_t key_len, const void *val, size_t& val_len);

	//need free return pointer
	void* find(const void *key, size_t key_len, size_t &val_len);
	//also put value in val_buf if val_buf is sufficient
	//need free return pointer if it not equals val_buf
	void* find(const void *key, size_t key_len, size_t &val_len, void *val_buf);

	//need free return pointer
	void* del(const void *key, size_t key_len, size_t& val_len);
	bool del(const void *key, size_t key_len);

	bool exist(const void *key, size_t key_len) { return find(key, key_len); }
	size_t record_count() { return performance()->record_count(); }

	//need free return pointer
	void* first_key(size_t &key_len);
	//need free return pointer
	void* next_key(const void *key, size_t &key_len);

	template <typename T> void walk(T *query) { walk(left_most(root_index_page()), query); }
	template <typename T> void walk(const void *key, size_t key_len, T *query) { walk(great_equal_key(key, key_len), query); }

	Page** snapshot_reference(size_t &snapshot_size) { return hash.snapshot_reference(snapshot_size); }
	void snapshot_create() { performance()->set_dirty_peak(hash.snapshot_create(&magic)); }
	void snapshot_release();

private:
	static inline Page* Header2Page(void *p) { return *(Page **)((PageLayout *)((ptrdiff_t)p & ~PAGEMASK) + 1); }
	static inline page_index_t Header2PageIndex(void *p) { return Header2Page(p)->index(); }
	//不复制child_index, 叶子节点不会上升变成非叶子的child_index始终是0
	static inline void copy_key(index_hdr *dst, index_hdr *src)
	{
		memcpy((page_index_t *)dst+1, (page_index_t *)src+1, sizeof(index_hdr)-sizeof(page_index_t));
	}
	static inline void copy_key_and_left(index_hdr *dst, index_hdr *src)
	{
		memcpy(dst, src, sizeof(index_hdr));
	}
	static inline void copy_key_and_right(index_hdr *dst, index_hdr *src)
	{
		memcpy((page_index_t *)dst+1, (page_index_t *)src+1, sizeof(index_hdr));
	}

	//----------page
	Page *Rload_page(page_index_t idx) { return hash.find(idx); }
	Page *Wload_page(page_index_t idx) { return hash.find(idx)->set_dirty(); }
	void *Rload_hdr(page_index_t idx, page_pos_t pos) { return (byte_t *)(Rload_page(idx)->layout_ptr()) + pos; }
	void *Wload_hdr(page_index_t idx, page_pos_t pos) { return (byte_t *)(Wload_page(idx)->layout_ptr()) + pos; }

	data_hdr* Rnext_page_header(data_hdr *hdr) { return (data_hdr *)Rload_hdr(hdr->next_page_index, hdr->next_page_pos); }
	data_hdr* Wnext_page_header(data_hdr *hdr) { return (data_hdr *)Wload_hdr(hdr->next_page_index, hdr->next_page_pos); }
	frag_hdr* Wnext_page_header(frag_hdr *hdr) { return (frag_hdr *)Wload_hdr(hdr->next_page_index, hdr->next_page_pos); }
	frag_hdr* Wprev_page_header(frag_hdr *hdr) { return (frag_hdr *)Wload_hdr(hdr->prev_page_index, hdr->prev_page_pos); }
	data_hdr* Rindex_hdr2data_hdr(index_hdr *hdr) { return (data_hdr *)Rload_hdr(hdr->page_index, hdr->page_pos); }
	data_hdr* Windex_hdr2data_hdr(index_hdr *hdr) { return (data_hdr *)Wload_hdr(hdr->page_index, hdr->page_pos); }

	Page *alloc_page(bool index_page = true);
	void free_page(Page *page);
	Page *alloc_root_index_page();
	Page *root_index_page() { return Rload_page(magic.get_root_index_idx()); }

	//----------fragment
	void set_fragment(frag_hdr *hdr);
	void clr_fragment(frag_hdr *hdr);

	//if hdr->capacity() is too large, adjust it
	//all data_dhr and frag_hdr are aligned by 16 bytes
	void adjust_data_head(data_hdr *hdr);

	//----------data_hdr
	data_hdr* __alloc_data_head(size_t size);
	data_hdr* alloc_data_head(index_hdr &idx, size_t size);
	data_hdr* alloc_data_head(data_hdr *pdr, size_t size);
	void create_node(index_hdr& index, const void *key, size_t key_len, const void *val, size_t val_len);
	void free_data(data_hdr *hdr);

	//----------extract
	void extract_key_copyer(data_hdr *hdr, byte_t *dest, size_t copy_len);
	void extract_val_copyer(data_hdr *hdr, byte_t *dest, size_t pass_len);
	void* extract_data(data_hdr *hdr, size_t &size, void *data_buf);
	void* extract_key (data_hdr *hdr, size_t &key_len);
	void* extract_key (data_hdr *hdr, size_t &key_len, void *key_buf);
	void* extract_val (data_hdr *hdr, size_t &val_len, void *val_buf);
	void* extract_val (data_hdr *hdr, size_t &val_len);
	void* extract_data(index_hdr *hdr, size_t &size, void *data_buf) { return extract_data(Rindex_hdr2data_hdr(hdr), size, data_buf); }
	void* extract_key(index_hdr *hdr, size_t &key_len);
	void* extract_key(index_hdr *hdr, size_t &key_len, void *key_buf);
	void* extract_val(index_hdr *hdr, size_t &val_len) { return extract_val(Rindex_hdr2data_hdr(hdr), val_len); }
	void* extract_val(index_hdr *hdr, size_t &val_len, void *val_buf) { return extract_val(Rindex_hdr2data_hdr(hdr), val_len, val_buf); }

	//----------find
	//>0:key is greater  <0:key is less  0:equal
	int compare_key(index_hdr *hdr, const void *key, size_t key_len);
	/*
	@return   &pos  result
	NULL      N/A   NOT FOUND: empty
	NOT NULL  0     FOUND:   equals key
	NOT NULL  >0    GREATER: the greater after key or the index_right()
	NOT NULL  <0    GREATER: the greater after key, key is less than all keys in table
	*/
	//in the last 2 situations, the return value must be at leaf node
	index_hdr* find_key(const void *key, size_t key_len, int& pos);
	index_hdr* find(const void *key, size_t key_len);
	index_hdr* left_most(Page *page);
	index_hdr* next_index_header(index_hdr *cur);
	index_hdr* greater_key(const void *key, size_t key_len);
	index_hdr* great_equal_key(const void *key, size_t key_len);

	//----------walk
	void walk(index_hdr *hdr, IQueryKey *query);
	void walk(index_hdr *hdr, IQueryData *query);

	//----------insert
	index_hdr* insert_init_node();
	//插入一定插在叶子节点
	index_hdr* put_new_node(const void *key, size_t key_len, const void *val, size_t val_len);
	void* put_replace(index_hdr* found, const void *key, size_t key_len, const void *val, size_t& save_val_len, bool need_origin);
	void insert_leaf(index_hdr *pos, index_hdr *val);
	//page中的元素数目达到了INDEXCOUNT
	//分裂成两个子节点, 并拿出中间的元素来做为父元素
	void split(Page *page);
	//put mid in front of pos
	void insert_internal(Page *page, index_hdr *pos, index_hdr *mid, Page *l_child, Page *r_child);

	//----------remove
	void remove(index_hdr *pos);
	void remove_leaf(index_hdr *pos);
	void remove_internal(index_hdr *pos);
	void merge_sibling(Page *left, index_hdr *parent_hdr, Page *right);
	void r_shrink_internal(Page *page, index_hdr *parent_hdr);
	//将右孩子(sibling)的数据拿一个给左孩子(page)
	//此时左孩子有INDEXMINC-1个元素
	void r_shrink_leaf(Page *page, index_hdr *parent_hdr);
	void l_shrink_internal(Page *page, index_hdr *parent_hdr);
	//将左孩子(sibling)的元素拿一个给右孩子(page)
	//此时右孩子有INDEXMINC-1个元素
	void l_shrink_leaf(Page *page, index_hdr *parent_hdr);
};

} //namespace lcore

#endif
