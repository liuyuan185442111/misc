#ifndef LHEAPCHECKER
#define LHEAPCHECKER

/** 一个简单的内存检测工具 需要gcc使用-rdynamic参数链接
 * 重载全局的perator new和perator delete
 * 定义一个全局的lheapchecker对象, 程序结束前调用其check方法, 即可得到未释放的内存
lheapchecker checker;
void *operator new(size_t n)
{
	return checker.call(malloc(n));
}
void operator delete(void *p)
{
	checker.uncall(p);
	free(p);
}
*/

#include <execinfo.h>
#include "lalloc.h"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

class lheapchecker
{
	lmap<void*, int> call_counter;
	lmap<void*, int*> addr_caller;
	lmap<void*, lstring> cache;
	bool isopen;

public:
	lheapchecker(bool open = true) : isopen(open){}
	void open() { isopen = true; }
	void close() { isopen = false; }
	void* call(void *addr)
	{
		if(!isopen) return addr;
		void* buffer[3];
		int stack_num = backtrace(buffer, 3);
		if(stack_num < 3)
		{
			puts("lheapchecker backtrace error");
			return NULL;
		}

		/*
		if(cache.find(buffer[2]) == cache.end())
		{
			void* temp[1] = {buffer[2]};
			char ** stacktrace = backtrace_symbols(temp, 1);
			//应用于部分动态库会崩溃, 不知为何
			if(strstr(stacktrace[0], ".so("))
			{
				free(stacktrace);
				return addr;
			}
			cache[buffer[2]] = stacktrace[0];
			free(stacktrace);
		}
		*/

		int *pnum = &call_counter[buffer[2]];
		++*pnum;
		addr_caller[addr] = pnum;

		return addr;
	}
	void uncall(void *addr)
	{
		lmap<void*, int*>::iterator iter = addr_caller.find(addr);
		if(iter != addr_caller.end())
			--*iter->second;
	}

	void symbols()
	{
		for(lmap<void*, int>::iterator it = call_counter.begin(), ie = call_counter.end(); it != ie; ++it)
		{
			if(cache.find(it->first) == cache.end())
			{
				void* temp[1] = {it->first};
				char ** stacktrace = backtrace_symbols(temp, 1);
				//应用于部分动态库会崩溃, 不知为何
				if(strstr(stacktrace[0], ".so("))
				{
					free(stacktrace);
					cache[it->first] = "so";
					break;
				}
				cache[it->first] = stacktrace[0];
				free(stacktrace);
			}
		}
	}
	void dumpto(int fd = 1)
	{
		symbols();
		for(lmap<void*, int>::iterator it = call_counter.begin(), ie = call_counter.end(); it != ie; ++it)
		{
			if(it->second != 0)
			{
				write(fd, cache[it->first].data(), cache[it->first].length());
				char buffer[16];
				sprintf(buffer, ":%d\n", it->second);
				write(fd, buffer, strlen(buffer));
			}
		}
		fsync(fd);
	}
};

#endif
