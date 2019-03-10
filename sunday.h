/**
 * 2019-3-10
 * Sunday sring-search algorithm
 * reference: https://www.jianshu.com/p/2e6eb7386cd3
 */
#ifndef _SUNDAY_H
#define _SUNDAY_H
#include <string.h>
#include <unordered_map>
class Sunday
{
	const bool need_free;
	const char *pattern;
	const int len;
	std::unordered_map<char, int> move_table;
	int forward(char next_char)
	{
		auto it = move_table.find(next_char);
		if(it != move_table.end()) return it->second;
		return len + 1;
	}
public:
	Sunday(const char *pattern_str, bool deep_copy=false) : len(strlen(pattern_str)), need_free(deep_copy)
	{
		if(deep_copy) pattern = strdup(pattern_str);
		else pattern = pattern_str;
		for(int i=0; i<len; ++i, ++pattern_str)
			move_table[*pattern_str] = len - i;
	}
	~Sunday()
	{
		if(need_free) free((void *)pattern);
	}
	const char *findin(const char *text)
	{
		if(len == 0) return "";
		int text_len = strlen(text);
		while(text_len >= len)
		{
			int i = 0;
			for(; i<len; ++i)
			{
				if(text[i] != pattern[i])
				{
					if(text_len == len) return ""; // reach end
					int step = forward(text[len]);
					text += step;
					text_len -= step;
					break;
				}
			}
			if(i == len) return text;
		}
		return "";
	}
};
#endif
