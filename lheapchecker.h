#ifndef LHEAPCHECKER
#define LHEAPCHECKER

/** 一个简单的内存检测工具 需要gcc使用-rdynamic参数链接
 * 重载全局的operator new和operator delete,
 * 定义一个全局的lheapchecker对象, 程序结束前调用其dumpto方法,
 * 即可得到未释放的内存是在何处分配的, 例如:
//checker不定义成全局变量是因为, 其他全局变量如果更早的构造,
//且调用了new, 则会使用未初始化的checker
lheapchecker &checker()
{
	static lheapchecker checker;
	return checker;
}
void *operator new(size_t n)
{
	return checker().call(malloc(n));
}
void operator delete(void *p)
{
	checker().uncall(p);
	free(p);
}
*/

#include "lalloc.h"
#include <execinfo.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

class lheapchecker
{
public:
	lheapchecker(bool open = true, bool loney = true)
		: isopen(open), get_symbol_lonely(loney), symbols_per_time(0) {}
	void open() { isopen = true; }
	void close() { isopen = false; }
	void set_symbols_per_time(size_t n) { symbols_per_time = n; }

private:
	lmap<void*, int> call_counter;//function address, counter
	lmap<void*, int*> addr_caller;//memory, counter pointer
	lmap<void*, lstring> cache;//function address, function address string
	bool isopen;
	bool get_symbol_lonely;//即时获取symbol
	size_t symbols_per_time;//一次获取几个symbols

	//一个一个的获取所有symbols
	void symbols()
	{
		for(lmap<void*, int>::iterator it = call_counter.begin(), ie = call_counter.end(); it != ie; ++it)
		{
			if(cache.find(it->first) == cache.end())
			{
				void* temp[1] = {it->first};
				char ** stacktrace = backtrace_symbols(temp, 1);
				cache[it->first] = stacktrace[0];
				free(stacktrace);
			}
		}
	}
	//获取n个symbols, 返回是否已经处理完毕
	bool _symbol(size_t n)
	{
		if(n == 0) n=1;
		std::vector<void*> temp;
		temp.reserve(n);
		for(lmap<void*, int>::iterator it = call_counter.begin(), ie = call_counter.end(); n > 0 && it != ie; ++it)
		{
			if(cache.find(it->first) == cache.end())
			{
				--n;
				temp.push_back(it->first);
			}
		}
		if(!temp.empty())
		{
			char ** stacktrace = backtrace_symbols(&temp[0], temp.size());
			for(int i=temp.size()-1; i>=0; --i)
			{
				cache[temp[i]] = stacktrace[i];
			}
			free(stacktrace);
		}
		//碰到恰好处理完毕call_counter的情况,那就再调用一次该函数
		return n>0;
	}
	//n个n个的获取所有symbols
	void symbols(size_t n)
	{
		bool status = isopen;
		isopen = false;
		while(!_symbol(n));
		isopen = status;
	}

public:
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

		if(get_symbol_lonely)
		{
			if(cache.find(buffer[2]) == cache.end())
			{
				void* temp[1] = {buffer[2]};
				char ** stacktrace = backtrace_symbols(temp, 1);
				cache[buffer[2]] = stacktrace[0];
				free(stacktrace);
			}
		}

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
	void dumpto(int fd = 1)
	{
		if(!get_symbol_lonely)
		{
			if(symbols_per_time < 2)
				symbols();
			else
				symbols(symbols_per_time);
		}
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
