#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "pcre_matcher.h"

//实现字符串相加: buf=buf+str+addon, 自动处理占用内存, 返回最新的buf大小
//*buf为NULL表示需要程序为str分配存储空间, 此时buf_size应为0
//buf和str不可为NULL
size_t pm_addstr(char **buf, size_t buf_size, const char *str, const char *addon)
{
	if(!buf || !str) return buf_size;
	if(!*buf && buf_size) return 0;
	size_t buf_len = *buf ? strlen(*buf) : 0;
	size_t str_len = strlen(str);
	size_t addon_len = addon ? strlen(addon) : 0;
	size_t need_size = buf_len + str_len + addon_len + 1;
	if(need_size > buf_size)
	{
		//内存增长策略是取1.5倍原内存和实际所需内存的最大值
		size_t new_size = (buf_size * 3) >> 1;
		if(new_size < need_size) new_size = need_size;
		char *tmp = (char *)malloc(new_size);
		if(!tmp) return buf_size;
		if(*buf)
		{
			memcpy(tmp, *buf, buf_len + 1);
			free(*buf);
		}
		*buf = tmp;
		buf_size = new_size;
	}
	memcpy(*buf + buf_len, str, str_len);
	if(addon) memcpy(*buf + buf_len + str_len, addon, addon_len + 1);
	else (*buf)[buf_len + str_len] = 0;
	return buf_size;
}

enum {
	FAIL = -1,
	SUCCESS = 0,
	INVALID_POINTER_TO_MATCHER,
	INVALID_INNER_CODE,
	MALLOC_FAILED,
	INVALID_MATCHER,
	INVALID_PATTERN_FILE,
	STAT_PATTERN_FILE_FAILED,
	OPEN_PATTERN_FILE_FAILED,
	MMAP_FAILED,
	CANNOT_TRANS_PATTERN_FILE_CODE,
	TRANS_PATTERN_FILE_CODE_FAILED,
	EMPTY_PATTERN_FILE,
	PCRE_COMPILE_FAILED,
	INVALID_REGEXP,
	INVALID_STR,
	INVALID_STR_CODE,
	CHECK_ERROR,
};
#define CDINVALID ((iconv_t)-1)
int pm_create(pcre_matcher **pmatcher, const char *inner_code, const char *check_code, const char *default_input_code)
{
	if(!pmatcher) return INVALID_POINTER_TO_MATCHER;
	if(inner_code && strcmp(inner_code, "UTF8")) return INVALID_INNER_CODE;
	if((*pmatcher = (pcre_matcher*)malloc(sizeof(pcre_matcher))) == NULL) return MALLOC_FAILED;
	(*pmatcher)->inner_code = inner_code ? strdup(inner_code) : strdup("UTF8");
	(*pmatcher)->check_code = check_code ? strdup(check_code) : NULL;
	(*pmatcher)->str_code = default_input_code ? strdup(default_input_code) : NULL;
	(*pmatcher)->pattern = NULL;
	(*pmatcher)->regexp = NULL;
	if(check_code) (*pmatcher)->cvalid = iconv_open((*pmatcher)->inner_code, check_code);
	if(default_input_code) (*pmatcher)->cinput = iconv_open((*pmatcher)->inner_code, default_input_code);
	return SUCCESS;
}
int pm_load(pcre_matcher *matcher, const char *pattern_file, const char *file_code)
{
	if(!matcher) return INVALID_MATCHER;
	if(!pattern_file || !pattern_file[0]) return INVALID_PATTERN_FILE;
	if(!file_code) file_code = "GB2312";
	struct stat file_stat;
	if(stat(pattern_file, &file_stat) != 0) return STAT_PATTERN_FILE_FAILED;
	int file_fd = open(pattern_file, O_RDONLY);
	if(file_fd == -1) return OPEN_PATTERN_FILE_FAILED;
	char *pmap = (char *)mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
	close(file_fd); 
	if(!pmap) return MMAP_FAILED;

	char *in = pmap;
	size_t ilen = file_stat.st_size;
	if(ilen >= 2 && *(unsigned char*)in==0xFF && *(unsigned char*)(in+1)==0xFE)
	{
		in += 2;
		ilen -= 2;
		file_code = "UTF16";
	}
	else if(ilen >= 2 && *(unsigned char*)in==0xFE && *(unsigned char*)(in+1)==0xFF)
	{
		in += 2;
		ilen -= 2;
		file_code = "UTF16BE";
	}
	else if(ilen >= 3 && *(unsigned char*)in==0xEF && *(unsigned char*)(in+1)==0xBB && *(unsigned char*)(in+2)==0xBF)
	{
		in += 3;
		ilen -= 3;
		file_code = "UTF8";
	}
	if(ilen == 0)
	{
		munmap(pmap, file_stat.st_size);
		return EMPTY_PATTERN_FILE;
	}

	char *filebuf;
	if(strcmp(matcher->inner_code, file_code))
	{
		iconv_t cv = iconv_open(matcher->inner_code, file_code);
		if(cv == CDINVALID)
		{
			munmap(pmap, file_stat.st_size);
			return CANNOT_TRANS_PATTERN_FILE_CODE;
		}
		size_t buflen = 4*ilen + 1;
		if((filebuf = (char *)malloc(buflen)) == NULL)
		{
			iconv_close(cv);
			munmap(pmap, file_stat.st_size);
			return MALLOC_FAILED;
		}
		char *out = filebuf;
		size_t olen = buflen - 1;
		if(iconv(cv,&in,&ilen,&out,&olen) == (size_t)(-1))
		{
			free(filebuf);
			iconv_close(cv);
			munmap(pmap, file_stat.st_size);
			return TRANS_PATTERN_FILE_CODE_FAILED;
		}
		iconv_close(cv);
		filebuf[buflen - 1 - olen] = 0;
	}
	else
	{
		filebuf = (char *)malloc(ilen + 1);
		memcpy(filebuf, in, ilen);
		filebuf[ilen] = 0;
	}
	munmap(pmap, file_stat.st_size);

	size_t pattern_size = strlen(filebuf) + 3;
	char *pattern;
	char *sentry = strtok(filebuf, "\r\n");
	if(sentry)
	{
		if((pattern = (char *)malloc(pattern_size)) == NULL)
		{
			free(filebuf);
			return MALLOC_FAILED;
		}
		pattern[0] = '(';
		pattern[1] = '?';
		pattern[2] = ':';
		pattern[3] = 0;
	}
	else return EMPTY_PATTERN_FILE;
	while(sentry)
	{
		pattern_size = pm_addstr(&pattern, pattern_size, sentry, ")|(?:");
		sentry = strtok(NULL, "\r\n");
	}
	free(filebuf);

	pattern[strlen(pattern) - 4] = 0;
	const char *error;
	int erroffset;
	matcher->regexp = pcre_compile(pattern,PCRE_CASELESS|PCRE_UTF8,&error,&erroffset,0);
	if(!matcher->regexp) return PCRE_COMPILE_FAILED;
	matcher->pattern = pattern;
	return SUCCESS;
}
int pm_match(pcre_matcher *matcher, const char *str, size_t len, const char *str_code)
{
	if(!str) return INVALID_STR;
	if(len == 0) return SUCCESS;
	if(!matcher) return INVALID_MATCHER;
	if(!matcher->regexp) return INVALID_REGEXP;
	if(!matcher->str_code && !str_code) return INVALID_STR_CODE;

	iconv_t cvalid = CDINVALID;
	iconv_t cinput = CDINVALID;
	if(!str_code)
	{
		cvalid = matcher->cvalid;
		cinput = matcher->cinput;
	}
	else if(!matcher->str_code || strcmp(str_code, matcher->str_code))
	{
		if(matcher->check_code && strcmp(str_code, matcher->check_code))
			cvalid = iconv_open(matcher->check_code, str_code);
		if(strcmp(str_code, matcher->inner_code))
			cinput = iconv_open(matcher->inner_code, str_code);
	}
	if(cvalid != CDINVALID || cinput != CDINVALID)
	{
		char buf[4*len];
		char *in, *out;
		size_t ilen, olen;

		if(cvalid != CDINVALID)
		{
			in = (char *)str;
			ilen = len;
			out = buf;
			olen = 4*len;
			if(iconv(cvalid,&in,&ilen,&out,&olen) == (size_t)(-1))
				return CHECK_ERROR;
		}

		if(cinput != CDINVALID)
		{
			in = (char *)str;
			ilen = len;
			out = buf;
			olen = 4*len;
			iconv(cinput,&in,&ilen,&out,&olen);//转UTF8应该都能成功吧
			return (pcre_exec(matcher->regexp, NULL, buf, 4*len-olen, 0, PCRE_NO_UTF8_CHECK, NULL, 0) >= 0 ? SUCCESS : FAIL);
		}
	}
	//我不太清除为什么pcre_exec总是返回0或-1,有时又会返回1,或许需要额外设置?
	//https://stackoverflow.com/questions/48704002/what-is-the-perl-equivalent-of-pcres-pcre-partial
	//https://stackoverflow.com/questions/1421785/how-can-i-use-pcre-to-get-all-match-groups
	return (pcre_exec(matcher->regexp, NULL, str, len, 0, PCRE_NO_UTF8_CHECK, NULL, 0) >= 0 ? SUCCESS : FAIL);
}
void pm_remove(pcre_matcher *matcher)
{
	if(matcher)
	{
		if(matcher->inner_code) free(matcher->inner_code);
		if(matcher->check_code) free(matcher->check_code);
		if(matcher->str_code) free(matcher->str_code);
		if(matcher->pattern) free(matcher->pattern);
		if(matcher->regexp) pcre_free(matcher->regexp);
		free(matcher);
	}
}
const char *pm_strerror(int err_code)
{
	switch(err_code)
	{
		case FAIL:
			return "MATCH FAIL";
		case SUCCESS:
			return "SUCCESS";
		case INVALID_POINTER_TO_MATCHER:
			return "INVALID_POINTER_TO_MATCHER";
		case INVALID_INNER_CODE:
			return "INVALID_INNER_CODE";
		case MALLOC_FAILED:
			return "MALLOC_FAILED";
		case INVALID_MATCHER:
			return "INVALID_MATCHER";
		case INVALID_PATTERN_FILE:
			return "INVALID_PATTERN_FILE";
		case STAT_PATTERN_FILE_FAILED:
			return "STAT_PATTERN_FILE_FAILED";
		case OPEN_PATTERN_FILE_FAILED:
			return "OPEN_PATTERN_FILE_FAILED";
		case MMAP_FAILED:
			return "MMAP_FAILED";
		case CANNOT_TRANS_PATTERN_FILE_CODE:
			return "CANNOT_TRANS_PATTERN_FILE_CODE";
		case TRANS_PATTERN_FILE_CODE_FAILED:
			return "TRANS_PATTERN_FILE_CODE_FAILED";
		case EMPTY_PATTERN_FILE:
			return "EMPTY_PATTERN_FILE";
		case PCRE_COMPILE_FAILED:
			return "PCRE_COMPILE_FAILED";
		case INVALID_REGEXP:
			return "INVALID_REGEXP";
		case INVALID_STR:
			return "INVALID_STR";
		case INVALID_STR_CODE:
			return "INVALID_STR_CODE";
		case CHECK_ERROR:
			return "CHECK_ERROR";
		default:
			return "unknown code";
	}
}
