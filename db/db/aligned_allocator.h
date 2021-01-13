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
 �ṩ��ַ����Ĺ̶���С�ڴ����ڴ��, BOUNDARY��ÿ���ڴ��Ĵ�С, BLOCKS��ÿ�а������ڴ�������

 �ڴ沼����������(BLOCKS=3,c1�ȷ���,c2�����):
 -- c
|   |
|   c2.list --> block1 --> block2 --> block3 --> NULL
|   |
 -- c1.list --> block1 --> block2 --> block3 --> NULL
 c�͸�����֮��Ľڵ����һ��˫������:
 c.prev --> c1, c.next --> c2
 c2.prev --> c, c2.next --> c1
 c1.prev --> c2, c1.next --> c

 block֮����һ����������, δʹ�õ�block����ʼ���ִ�ŵ���һ��block�ĵ�ַ,
 ��ʼ�����block1, block2, block3...������������, �����Ų��ϵ�alloc��free�Ͳ�������.
 */

template <int BOUNDARY, int BLOCKS, typename Allocator = DefaultMemoryAllocator>
class AlignedMemoryBlock
{
	void *list;
	union {
		size_t align; //ÿ��block�Ķ����ֽ���, ������4,8,16,32..., ˫�������head node��
		size_t size; //�����������ѱ�ʹ�õ�block������, ˫�������other node��
	};
	AlignedMemoryBlock *prev;
	AlignedMemoryBlock *next;
public:
	//alignΪ0��ʾ���ÿ��Ƕ���
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
