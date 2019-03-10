#ifndef _BOYERMOORE_H
#define _BOYERMOORE_H
#include <string.h>
#include <unordered_map>
#include <algorithm>
#ifdef BM_DEBUG_SHOW
#include <iostream>
#endif

class BoyerMoore
{
	const bool need_free;
	const char *pattern;
	const size_t len;
	std::unordered_multimap<char, int> badtable;

	int bad_forward(char bad_char, int bad_pos)
	{
		auto range = badtable.equal_range(bad_char);
		int forward = bad_pos + 1;
		for_each(range.first, range.second,
			[&forward,bad_pos](const std::pair<char,int>& x){if(bad_pos>x.second)forward=std::min(forward,bad_pos-x.second);});
		return forward;
	}
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
	}
	~BoyerMoore()
	{
		if(need_free) free((void *)pattern);
	}
	const char *findin(const char *text)
	{
		if(len == 0) return "";
		size_t text_len = strlen(text);
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
				for(int i=b-pattern; i; --i) std::cout << '-';
				std::cout << "*\n";
				std::cout << string(text, b-pattern) << a << endl;
				std::cout << string(pattern, b-pattern) << b << endl;
#endif
				if(*a != *b)
				{
					int forward = bad_forward(*a, b-pattern);
					text += forward;
					text_len -= forward;
#ifdef BM_DEBUG_SHOW
					std::cout << "forward " << forward << endl;
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
