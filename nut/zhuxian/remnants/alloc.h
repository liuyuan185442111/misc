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
 �ṩ�̶���С�ڴ����ڴ��, BOUNDARY��ÿ���ڴ��Ĵ�С, BLOCKS��ÿ�а������ڴ�������

 �ڴ沼����������(BLOCKS=3):
 -- c
|   |
|   line1.list --> block1 --> block2 --> block3 --> NULL
|   |
 -- line2.list --> block1 --> block2 --> block3 --> NULL

 c�͸�line֮����һ��˫������:
 c.prev --> line1, c.next --> line3
 line1.prev --> c, line1.next --> line2
 line2.prev --> line1, line2.next --> c
 ���������̫���ʱ��, free�������ķѴ���ʱ��, ��ʱ����Կ�����һ��map����֯��line.

 block֮����һ����������, δʹ�õ�block����ʼ���ִ�ŵ���һ��block�ĵ�ַ,
 ��ʼ�����block1, block2, block3...������������, �����Ų��ϵ�alloc��free�Ͳ�������.
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
