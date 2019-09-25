#ifndef __ALLOC_H
#define __ALLOC_H

#include <stdlib.h>
#include <stddef.h>//for ptrdiff_t
#include "mutex.h"

namespace GNET
{

struct DefaultMemoryAllocator
{
	static void *alloc(size_t size) { return malloc(size); }
	static void free(void *p, size_t size) { ::free(p); }
};

/**
 提供固定大小内存块的内存池, BOUNDARY是每个内存块的大小, BLOCKS是每行包含的内存块的数量

 内存布局是这样的(BLOCKS=3):
 -- c
|   |
|   line1.list --> block1 --> block2 --> block3 --> NULL
|   |
 -- line2.list --> block1 --> block2 --> block3 --> NULL

 c和各line之间是一个双向链表:
 c.prev --> line1, c.next --> line3
 line1.prev --> c, line1.next --> line2
 line2.prev --> line1, line2.next --> c
 当这个链表太大的时候, free操作将耗费大量时间, 这时候可以考虑用一个map来组织各line.

 block之间是一个单向链表, 未使用的block的起始部分存放的下一个block的地址,
 初始情况下block1, block2, block3...是物理连续的, 但随着不断的alloc和free就不连续了.
 */

//BOUNDARY:size of memory block
//BLOCKS:amount of a memory line's blocks
template <int BOUNDARY, int BLOCKS, typename Allocator = DefaultMemoryAllocator>
class AlignedMemoryBlock
{
	void *list;
	size_t size;//used blocks in the memory line
	AlignedMemoryBlock *prev;
	AlignedMemoryBlock *next;
	static Thread::SpinLock locker;
	enum { ALIGN = 16 };//4,8,16,32...
public:
	AlignedMemoryBlock() : prev(this), next(this) { }
	void *alloc()
	{
		Thread::SpinLock::Scoped l(locker);
		for(AlignedMemoryBlock *c = next; c != this; c = c->next)
			if(void *p = c->list)
			{
				c->list = *(void **)p;
				++c->size;
				return p;
			}
		//need alloc a new line
		AlignedMemoryBlock *c = (AlignedMemoryBlock *)Allocator::alloc(sizeof(AlignedMemoryBlock) + BOUNDARY * BLOCKS + ALIGN);
		c->prev     = this;
		c->next     = next;
		c->size     = 1;
		next->prev  = c;
		next        = c;
		void *p     = (void *)(((ptrdiff_t)c + sizeof(AlignedMemoryBlock) + ALIGN - 1) & -ALIGN);
		void *q     = c->list = (char *)p + BOUNDARY;
		for(int i = BLOCKS - 1; --i; q = *(void **)q = (char *)q + BOUNDARY);
		*(void **)q = NULL;
		return p;
	}
	void free(void *p)
	{
		Thread::SpinLock::Scoped l(locker);
		for(AlignedMemoryBlock *c = next; c != this; c = c->next)
			if(p >= c && p < ((char *)c + sizeof(AlignedMemoryBlock) + BOUNDARY * BLOCKS + ALIGN))
			{
				*(void **)p = c->list;
				c->list     = p;
				if(!--c->size)
				{
					c->next->prev = c->prev;
					c->prev->next = c->next;
					Allocator::free(c, sizeof(AlignedMemoryBlock) + BOUNDARY * BLOCKS + ALIGN);
				}
				break;
			}
	}
};

template <int BOUNDARY, int BLOCKS, typename Allocator>
Thread::SpinLock AlignedMemoryBlock<BOUNDARY,BLOCKS,Allocator>::locker("AlignMemoryBlock");

}
#endif
