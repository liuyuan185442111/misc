#include "page.h"

namespace lcore {

PageFile::PageFile(const char *path, int flags)
{
	if((fd = open(path, flags, 0600)) == -1)
	{
		fprintf(stderr, "open %s failed: %s\n", path, strerror(errno));
		errno = 0;
		throw Exception();
	}
	count_read = count_write = count_sync = 0;
	const char *p = strrchr(path, '/');
	name = strdup(p ? p + 1 : path);
}

void PageFile::read(page_index_t idx, void *data)
{
	count_read++;
	errno = 0; //重置一下以防搞错
	if(pread(fd, data, PAGESIZE, (off_t)idx * PAGESIZE) != PAGESIZE)
	{
		//如果要读取的位置超出文件范围, errno为0
		if(errno)
		{
			fprintf(stderr, "read page %d of %s failed: %s\n", idx, name, strerror(errno));
			errno = 0;
		}
		throw Exception();
	}
}

void PageFile::write(page_index_t idx, const void *data)
{
	count_write++;
	if(pwrite(fd, data, PAGESIZE, (off_t)idx * PAGESIZE) != PAGESIZE)
	{
		fprintf(stderr, "write page %d of %s failed: %s\n", idx, name, strerror(errno));
		errno = 0;
		throw Exception();
	}
}

void PageFile::truncate(page_index_t idx)
{
	if(ftruncate(fd, (off_t)idx * PAGESIZE) == -1)
	{
		fprintf(stderr, "truncate %s to %d pages failed: %s\n", name, idx, strerror(errno));
		errno = 0;
		throw Exception();
	}
}

void PageFile::sync()
{
	count_sync++;
	if(fsync(fd) == -1)
	{
		fprintf(stderr, "sync %s failed: %s\n", name, strerror(errno));
		errno = 0;
		throw Exception();
	}
}

void PageFile::set_magic(int magic)
{
	char data[sizeof(magic)];
	memcpy(data, &magic, sizeof(magic));
	lseek(fd, 0, SEEK_END);
	if(::write(fd, data, sizeof(data)) != sizeof(data))
	{
		fprintf(stderr, "write magic of %s failed: %s\n", name, strerror(errno));
		errno = 0;
		throw Exception();
	}
}

int PageFile::get_magic()
{
	int magic;
	char data[sizeof(magic)];
	lseek(fd, -sizeof(magic), SEEK_END);
	if(::read(fd, data, sizeof(magic)) != sizeof(magic))
	{
		fprintf(stderr, "read magic of %s failed: %s\n", name, strerror(errno));
		errno = 0;
		throw Exception();
	}
	memcpy(&magic, data, sizeof(magic));
	return magic;
}

PageMemory::PageMemoryType PageMemory::mem(PAGESIZE);
#ifdef _REENTRANT_
SpinLock PageMemory::locker;
#endif

bool Page::set_snapshot()
{
	if(dirty)
	{
		PageLayout *tmp = (PageLayout *)PageMemory::alloc();
		memcpy(tmp, layout, PAGESIZE);
		*(page_index_t *)(tmp + 1) = idx;
		*((page_index_t *)(tmp + 1) + 1) = type;
		snapshot = tmp;
		dirty = false;
		return true;
	}
	return false;
}

void Page::set_frag_page_position(int i, page_index_t page_index, page_pos_t page_pos)
{
	set_dirty();
	layout->_magic.frag_page_index[i] = page_index;
	layout->_magic.frag_page_pos[i] = page_pos;
}

void Page::set_parent(page_index_t parent_index, page_pos_t parent_pos)
{
	set_dirty();
	index_hdr *end = index_end();
	end->parent_index = parent_index;
	end->parent_pos = parent_pos;
}

class PagePool {
	typedef AlignedMemoryBlock<sizeof(Page), PAGEPOOLSLOTSIZE> PagePoolType;
	static PagePoolType pool;
#ifdef _REENTRANT_
	static SpinLock locker;
public:
	static void *alloc() { SpinLock::Scoped l(locker); return pool.alloc(); }
	static void free(void *p) { SpinLock::Scoped l(locker); return pool.free(p); }
#else
public:
	static void *alloc() { return pool.alloc(); }
	static void free(void *p) { return pool.free(p); }
#endif
};
PagePool::PagePoolType PagePool::pool;
#ifdef _REENTRANT_
SpinLock PagePool::locker;
#endif

void* Page::operator new(size_t size) { return PagePool::alloc(); }
void Page::operator delete(void *p) { return PagePool::free(p); }

} //namespace lcore
