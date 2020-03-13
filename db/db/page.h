#ifndef _L_PAGE_H
#define _L_PAGE_H

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <algorithm>
#include "performance.h"
#include "aligned_allocator.h"
#ifdef _REENTRANT_
#include "mutex.h"
#endif

namespace lcore {

typedef unsigned int   page_index_t;
typedef unsigned short page_pos_t;
typedef unsigned char  byte_t;
typedef unsigned int   size32_t;
typedef unsigned int   time32_t;

const int PAGESIZE = 4096;
const int PAGESLOTSIZE = 1024;
const int PAGEPOOLSLOTSIZE = 4096;
const int WALKBUFFERSIZE = 16384;
const int FRAGSTEP = 8;
const int PAGEUSED = PAGESIZE - 8;
const ptrdiff_t PAGEMASK = ((ptrdiff_t)(PAGESIZE-1)); //0x0FFF

inline page_pos_t header_offset(void *p) { return (ptrdiff_t)p & PAGEMASK; }

class PageFile
{
	int	fd;
	char *name;
	size_t count_read, count_write, count_sync;
public:
	struct Exception { };
	PageFile(const char *path, int flags = O_CREAT|O_RDWR);
	~PageFile() { close(fd); free(name); }
	const char *identity() const { return name; }
	void read(page_index_t idx, void *data);
	void write(page_index_t idx, const void *data);
	void truncate(page_index_t idx);
	void sync();
	size_t read_counter()  const { return count_read;  }
	size_t write_counter() const { return count_write; }
	size_t sync_counter()  const { return count_sync;  }

	void set_magic(int magic);
	int get_magic();
};

/**
不考虑logger的话, 有4种Page: magic page, index page, data page, free page
magic page存放于文件头部
index page由多个index_hdr组成, index page被组织成b树, index_hdr中存储了key的部分值, 完整的key+value则存放于data page中
data page由至少1个data块和0或多个frag块组成, data块由三部分data_hdr、used_data、unused_data组成,
	frag块由两部分frag_hdr和frag_data组成, data块和frag块链成链表
key+value由至少1个data块组成, 它们被组织成链表
data_hdr和frag_hdr拥有两个相同的成员next和size, 且它们分别在data_hdr和frag_hdr中的相对位置也相同
需要data块时先从frag list查找有无合适frag块, 如有则将其转化为data块, 其容量可能大于实际需要的空间,
	size指示实际使用的空间, next可以指示容量; 如无合适的frag块, 则新分配一个页面, 取所需部分,
	如有剩余尝试查看其后是不是frag块, 如是则进行合并, 最后放入frag list中
一个占用整个页面的data块被释放时直接放到free page list中; 而一个小data块被释放时转化为frag块, size置为0,
	然后检测其前后是不是也是frag块, 如果也是则会合并它们, 因此需要从页面头部的data块/frag块依次找过来,
	合并之后形成的frag如果占用一个完整页面, 则将该页面放到free page list中, 否则放到frag list中
*/
#pragma pack(1)

struct magic_hdr
{
	page_index_t	free_page_list;
	page_index_t	root_index_idx;
	page_index_t	max_page_idx;
	time32_t		logger_last_check; //上次check的时间戳
	time32_t		logger_id; //log file文件名, NullLoger会置为0
	Performance		performance;
	//frag list headers, grouped by size
	//index:size 0:[0,8) 1:[8,16) 2:[16,24) ...
	//index 0 and index 1 are not used, see adjust_data_head
	page_index_t	frag_page_index[PAGEUSED/FRAGSTEP];
	page_pos_t		frag_page_pos[PAGEUSED/FRAGSTEP];
};

struct data_hdr
{
	page_pos_t   next; //next header pos
	page_pos_t   size; //size of data, not 0

	page_index_t next_page_index;
	page_pos_t   next_page_pos;
	page_pos_t   first_slice; //bool

	data_hdr*    next_header() { return next != PAGEUSED ? (data_hdr *)(((ptrdiff_t)this & ~PAGEMASK) + next) : NULL; }
	size_t       capacity() { return next - header_offset(this) - sizeof(data_hdr); }
	void         set_next(page_index_t idx, page_pos_t pos) { next_page_index = idx; next_page_pos = pos; }
};

struct frag_hdr
{
	page_pos_t		next; //next header pos
	page_pos_t		size; //must be 0

	page_index_t	next_page_index;
	page_index_t	prev_page_index;
	page_pos_t		next_page_pos;
	page_pos_t		prev_page_pos;

	void	set_next(page_index_t idx, page_pos_t pos) { next_page_index = idx; next_page_pos = pos; }
	void	set_prev(page_index_t idx, page_pos_t pos) { prev_page_index = idx; prev_page_pos = pos; }
};

struct index_hdr
{
	page_index_t	child_index;
	union
	{
		struct
		{
			page_index_t	page_index; //page index of data
			page_pos_t		page_pos; //pos of data in data page
			byte_t			key[9]; //start bytes of key
			byte_t			key_len;
		};

		struct
		{
			page_index_t	parent_index;
			page_pos_t		parent_pos;
			page_pos_t		l_pos;
			page_pos_t		r_pos;
		};
	};

	page_index_t& l_child() { return child_index; }
	page_index_t& r_child() { return (this+1)->child_index; }
	void set_data_position(page_index_t idx, page_pos_t pos) { page_index = idx; page_pos = pos; }
};

struct logger_hdr
{
	page_index_t	logger_first_idx;
	page_index_t	logger_last_idx; // 下一个要使用的page的序号
	page_index_t	check_result;
	time32_t		logger_check_timestamp;
};

struct logger_magic_hdr
{
	page_index_t	logger_page_first; // log file中第一个page的序号
	page_index_t	logger_page_last; // log file中下一个要使用的page的序号
	page_index_t	logger_rec_max; // log file的magic page所能容纳的logger_head最多数量
	page_index_t	logger_rec_cur; // 下一个要使用的logger_head索引
	time32_t		logger_chain; // 下一个log file的logger_id
	logger_hdr		logger_head[1];
};

const size_t PAGEMAXSPARE = PAGEUSED - sizeof(data_hdr);
//每个节点最多包含的元素数目
const page_pos_t INDEXCOUNT = (PAGEUSED / sizeof(index_hdr)) - 1;
//每个非根节点最少包含的元素数目
const page_pos_t INDEXMINC = (INDEXCOUNT - 1) / 2;
const page_pos_t SHRINK_SHIFT = INDEXCOUNT / 4;

/** 为什么有8字节没有用
1. 最后的8字节用来放指向Page的指针, 从而使得各header能找得到Page
2. set_snapshot的时候前4字节用来放Page的序号, 后4字节用来放Page的类型,
   前者使得Logger的prepare阶段可以同时保存脏页面和脏页面的序号,
   后者使得PageBrowser可以轻松的判断出页面的类型
*/
union PageLayout
{
	char _used[PAGEUSED];
	magic_hdr _magic;
	index_hdr _index[INDEXCOUNT + 1];
	data_hdr _data[1];
	logger_magic_hdr _logger_magic;
};

#pragma pack()

class PageMemory {
	typedef AlignedMemoryBlock<PAGESIZE, PAGESLOTSIZE> PageMemoryType;
	static PageMemoryType mem;
#ifdef _REENTRANT_
	static SpinLock locker;
public:
	static void *alloc() { SpinLock::Scoped l(locker); return mem.alloc(); }
	static void free(void *p) { SpinLock::Scoped l(locker); mem.free(p); }
#else
public:
	static void *alloc() { return mem.alloc(); }
	static void free(void *p) { mem.free(p); }
#endif
};

enum { FREE_PAGE,INDEX_PAGE,DATA_PAGE };
class Page
{
	const page_index_t idx;
	page_index_t type;
	size_t lru;
	Page *next_hash_equal;
	PageLayout *layout;
	PageLayout *snapshot;
	bool dirty;

	struct lruless { bool operator()(Page * p1, Page * p2) const { return p1->lru < p2->lru; } };
public:
	static void sort_lru(Page **it, Page **ie) { std::sort(it, ie, lruless()); }

	Page(page_index_t index) : idx(index), type(0), lru(0), next_hash_equal(NULL),
		layout((PageLayout *)PageMemory::alloc()), snapshot(NULL), dirty(false) { *(Page **)(layout + 1) = this; }
	~Page() { PageMemory::free(layout); }

	page_index_t index() const { return idx; }
	void set_type(page_index_t page_type) { type = page_type; }
	page_index_t get_type() const { return type; }
	PageLayout *layout_ptr() { return layout; }
	const PageLayout *layout_ptr() const { return layout; }

	//lru
	void set_lru(size_t l) { lru = l; }
	Page *get_next_hash_equal() const { return next_hash_equal; }
	void set_next_hash_equal(Page *page) { next_hash_equal = page; }

	//snapshot
	const PageLayout* snapshot_ptr() const { return snapshot; }
	bool set_snapshot();
	void clr_snapshot() { PageMemory::free(snapshot); snapshot = NULL; }
	Page* set_dirty() { dirty = true; return this; }
	bool is_clean() const { return !dirty; }

	//magic page
	void init_magic() { memset(layout, 0, sizeof(PageLayout)); }
	page_index_t get_root_index_idx() const { return layout->_magic.root_index_idx; }
	void set_root_index_idx(page_index_t idx) { set_dirty(); layout->_magic.root_index_idx = idx; }
	page_index_t get_free_page_list() const { return layout->_magic.free_page_list; }
	void set_free_page_list(page_index_t idx) { set_dirty(); layout->_magic.free_page_list = idx; }
	page_index_t get_frag_page_index(int i) const { return layout->_magic.frag_page_index[i]; }
	page_pos_t   get_frag_page_pos  (int i) const { return layout->_magic.frag_page_pos[i];   }
	void set_frag_page_position(int i, page_index_t page_index, page_pos_t page_pos);
	page_index_t extend_page() { set_dirty(); return ++layout->_magic.max_page_idx; }

	//index page
	index_hdr*    index_end()    { return layout->_index + INDEXCOUNT; }
	index_hdr*    index_left()   { return layout->_index + index_end()->l_pos; }
	index_hdr*    index_right()  { return layout->_index + index_end()->r_pos; }
	index_hdr*    index_pos(page_pos_t page_pos) { return layout->_index + page_pos; }
	page_pos_t&   index_l_pos()  { return index_end()->l_pos; }
	page_pos_t&   index_r_pos()  { return index_end()->r_pos; }
	page_pos_t    index_m_pos()  { index_hdr *end = index_end(); return (end->r_pos + end->l_pos) >> 1; }
	page_pos_t    index_size()   { index_hdr *end = index_end(); return end->r_pos - end->l_pos; }
	static page_pos_t index_offset(index_hdr *hdr) { return header_offset(hdr) / sizeof(index_hdr); }
	page_index_t& parent_index() { return index_end()->parent_index; }
	page_pos_t&   parent_pos()   { return index_end()->parent_pos; }
	void set_parent(page_index_t parent_index, page_pos_t parent_pos);

	static void* operator new(size_t size);
	static void operator delete(void *p);
};

} //namespace lcore

#endif
