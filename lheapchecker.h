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
	lmap<lstring, int> call_counter;
	lmap<void*, int*> addr_caller;
public:
	void* call(void *addr)
	{
		void* buffer[3];
		int stack_num = backtrace(buffer, 3);
		if(stack_num < 3)
		{
			puts("error");
			return NULL;
		}

		char ** stacktrace = backtrace_symbols(buffer, stack_num);
		lstring caller(stacktrace[2]);
		free(stacktrace);
		//应用于动态库会崩溃, 不知为何
		if(caller.find(".so(") != std::string::npos)
			return addr;
		caller.erase(caller.find_first_of(' '));

		int *pnum = &call_counter[caller];
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

	void checkto(int fd = 1)
	{
		for(lmap<lstring, int>::iterator it = call_counter.begin(), ie = call_counter.end(); it != ie; ++it)
		{
			if(it->second != 0)
			{
				write(fd, it->first.data(), it->first.length());
				char buffer[16];
				sprintf(buffer, ":%d\n", it->second);
				write(fd, buffer, strlen(buffer));
			}
		}
		fsync(fd);
	}
};

#endif
