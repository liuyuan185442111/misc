/**
 * 2019-3-10
 * Boyer-Moort sring-search algorithm
 * reference: http://www.ruanyifeng.com/blog/2013/05/boyer-moore_string_search_algorithm.html
 */
#ifndef _BOYERMOORE_H
#define _BOYERMOORE_H
#include <string.h>
#include <unordered_map>
#include <algorithm>
#ifdef BM_DEBUG_SHOW
#include <iostream>
#include <string>
#endif

class BoyerMoore
{
	const bool need_free;
	const char *pattern;
	const int len;
	std::unordered_multimap<char, int> badtable;
#ifdef BM_ENABLE_GOOD_TABLE
	std::unordered_map<int, int> goodtable;
#endif

	int bad_forward(char bad_char, int bad_pos)
	{
		auto range = badtable.equal_range(bad_char);
		int forward = bad_pos + 1;
		for_each(range.first, range.second,
			[&forward,bad_pos](const std::pair<char,int>& x){if(bad_pos>x.second)forward=std::min(forward,bad_pos-x.second);});
		return forward;
	}
#ifdef BM_ENABLE_GOOD_TABLE
	int good_forward(int good_pos)
	{
		auto it = goodtable.find(good_pos);
		if(it != goodtable.end()) return it->second;
		return len;
	}
	void make_goodtable()
	{
		for(int i=len-1; i; --i)
		{
			for(int j=i-1; j>i-len; --j)
			{
				int m=i, n=j;
				for(; m<len; ++m, ++n)
				{
					if(n >= 0 && pattern[m] != pattern[n])
						break;
				}
				if(m == len)
				{
#ifdef BM_DEBUG_SHOW
					cout << "make_goodtable " << i << ":" << i - j << endl;
#endif
					goodtable.emplace(i, i - j);
				}
			}
		}
	}
#endif
public:
	//if pattern_str is nullptr, strlen would coredump
	BoyerMoore(const char *pattern_str, bool deep_copy = false) : len(strlen(pattern_str)), need_free(deep_copy)
	{
		if(!deep_copy)
			pattern = pattern_str;
		else
		{
#if _SVID_SOURCE || _BSD_SOURCE || _XOPEN_SOURCE >= 500
			pattern = strdup(pattern_str);
#else
			pattern = malloc(len + 1);
			strcpy(pattern, pattern_str);
#endif
		}

		for(int pos = 0; *pattern_str; ++pattern_str, ++pos)
			badtable.emplace(*pattern_str, pos);
#ifdef BM_ENABLE_GOOD_TABLE
		make_goodtable();
#endif
	}
	~BoyerMoore()
	{
		if(need_free) free((void *)pattern);
	}
	const char *findin(const char *text)
	{
		if(len == 0) return "";
		int text_len = strlen(text);
		while(text_len >= len)
		{
#ifdef BM_DEBUG_SHOW
			std::cout << endl;
			std::cout << "current text is " << text << endl;
			std::cout << "pattern      is " << pattern << endl;
#endif
			const char *a = text + len - 1;
			const char *b = pattern + len - 1;
			for(; b != pattern; --a, --b)
			{
#ifdef BM_DEBUG_SHOW
				std::cout << std::string(b-pattern, '-') << "*\n";
				std::cout << std::string(text, b-pattern) << a << endl;
				std::cout << std::string(pattern, b-pattern) << b << endl;
#endif
				if(*a != *b)
				{
					int pos = b-pattern;
					int step = bad_forward(*a, pos);
#ifdef BM_ENABLE_GOOD_TABLE
					if(pos < len - 1) step = std::max(step, good_forward(pos+1));
#endif
					text += step;
					text_len -= step;
#ifdef BM_DEBUG_SHOW
					std::cout << "bad_forward " << bad_forward(*a, pos) << endl;
#ifdef BM_ENABLE_GOOD_TABLE
					if(pos < len - 1)
						std::cout << "good_forward " << good_forward(pos+1) << endl;
#endif
#endif
					break;
				}
			}
			if(b == pattern) if(*a == *b) return a; else text += 1;
		}
		return "";
	}
};
#endif
