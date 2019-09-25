/** 2018-10-2
*/
#ifndef __GCORE_MATCHER_H
#define __GCORE_MATCHER_H

#include <iconv.h>

#define __NO_PCRE_HEADER//debug

#ifdef __NO_PCRE_HEADER
#define PCRE_CASELESS       0x00000001
#define PCRE_UTF8           0x00000800
#define PCRE_NO_UTF8_CHECK  0x00002000
#ifdef __cplusplus
extern "C" {
#endif
struct pcre;
struct pcre_extra;
//I don't know why?!
#ifdef __cplusplus
void pcre_free(void *);
#else
void (*pcre_free)(void *);
#endif
struct pcre *pcre_compile(const char *, int, const char **, int *, const unsigned char *);
int pcre_exec(const struct pcre *, const struct pcre_extra *, const char *, int, int, int, int *, int);
#ifdef __cplusplus
}
#endif
#else
#include <pcre.h>
#endif

typedef struct pcre_matcher
{
	iconv_t cinput;
	iconv_t cvalid;
	char *inner_code, *check_code, *str_code;
	char *pattern;
	struct pcre *regexp;
} pcre_matcher;

#ifdef __cplusplus
extern "C" {
#endif
//inner_code: 所有的编码最终都转换为inner_code进行处理, 仅支持UTF8, 其他编码可能在文件中出现'\0', 影响字符串处理, NULL表示使用UTF8
//check_code: 要检测的str首先测试能否转化为check_code编码, NULL表示不进行检测
//default_input_code: 要检测的str的默认编码, NULL表示检测时需要提供字符编码
int pm_create(pcre_matcher **matcher, const char *inner_code, const char *check_code, const char *default_input_code);
//file_code表示pattern_file的文件格式, 可识别简体中文Windows记事本的四种编码, 此时可设置为NULL
int pm_load(pcre_matcher *matcher, const char *pattern_file, const char *file_code);
//str_code为NULL表示使用pm_create中设置的default_input_code, 如果二者都没设置则会出错
int pm_match(pcre_matcher *matcher, const char *str, size_t len, const char *str_code);
void pm_remove(pcre_matcher *matcher);
const char *pm_strerror(int err_code);
#ifdef __cplusplus
}
#endif

#endif
