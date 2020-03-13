#ifndef _L_ALIGNED_ALLOCATOR_H
#define _L_ALIGNED_ALLOCATOR_H

#include <stdlib.h>
#include <stddef.h>//for ptrdiff_t

struct DefaultMemoryAllocator
{
	static void *alloc(size_t size) { return malloc(size); }
	static void free(void *p, size_t size) { ::free(p); }
};

/**
 提供地址对齐的固定大小内存块的内存池, BOUNDARY是每个内存块的大小, BLOCKS是每行包含的内存块的数量

 内存布局是这样的(BLOCKS=3,c1先分配,c2后分配):
 -- c
|   |
|   c2.list --> block1 --> block2 --> block3 --> NULL
|   |
 -- c1.list --> block1 --> block2 --> block3 --> NULL
 c和各行首之间的节点组成一个双向链表:
 c.prev --> c1, c.next --> c2
 c2.prev --> c, c2.next --> c1
 c1.prev --> c2, c1.next --> c

 block之间是一个单向链表, 未使用的block的起始部分存放的下一个block的地址,
 初始情况下block1, block2, block3...是物理连续的, 但随着不断的alloc和free就不连续了.
 */

template <int BOUNDARY, int BLOCKS, typename Allocator = DefaultMemoryAllocator>
class AlignedMemoryBlock
{
	void *list;
	union {
		size_t align; //每个block的对齐字节数, 可以是4,8,16,32..., 双向链表的head node用
		size_t size; //单向链表中已被使用的block的数量, 双向链表的other node用
	};
	AlignedMemoryBlock *prev;
	AlignedMemoryBlock *next;
public:
	//align为0表示不用考虑对齐
	AlignedMemoryBlock(size_t align_bytes = 0) : list(NULL), align(align_bytes), prev(this), next(this) { }
	void *alloc()
	{
		for(AlignedMemoryBlock *c = next; c != this; c = c->next)
		{
			if(void *p = c->list)
			{
				c->list = *(void **)p;
				++c->size;
				return p;
			}
		}
		//need alloc new blocks
		AlignedMemoryBlock *c = (AlignedMemoryBlock *)Allocator::alloc(sizeof(AlignedMemoryBlock) + BOUNDARY * BLOCKS + align);
		c->prev     = this;
		c->next     = next;
		c->size     = 1;
		next->prev  = c;
		next        = c;
		void *p     = align ? (void *)(((ptrdiff_t)c + sizeof(AlignedMemoryBlock) + align - 1) & -align) : (char *)c + sizeof(AlignedMemoryBlock);
		void *q     = c->list = (char *)p + BOUNDARY;
		for(int i   = BLOCKS - 1; --i; q = *(void **)q = (char *)q + BOUNDARY);
		*(void **)q = NULL;
		return p;
	}
	void free(void *p)
	{
		const size_t blocks_size = sizeof(AlignedMemoryBlock) + BOUNDARY * BLOCKS + align;
		for(AlignedMemoryBlock *c = next; c != this; c = c->next)
		{
			if(p > c && p < ((char *)c + blocks_size))
			{
				*(void **)p = c->list;
				c->list     = p;
				if(!--c->size)
				{
					c->next->prev = c->prev;
					c->prev->next = c->next;
					Allocator::free(c, blocks_size);
				}
				return;
			}
		}
		abort();
	}
};

#endif
