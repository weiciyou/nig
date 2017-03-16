/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#ifndef _NIG_STRING_H_INCLUDED_
#define _NIG_STRING_H_INCLUDED_

#include "nig_header.h"
#include "nig_config.h"

typedef struct {
    char * str;
    unsigned int len;
} nig_str_t;

extern void *nig_alloc(size_t size);

NIG_API nig_str_t *nig_string( char * str );

NIG_API void nig_free_string( nig_str_t **cstr );

// 将str前n个字符转换为小写
NIG_API void strtolow(char *str, int n);

// 判断str起始位置开始是否包含"content-type"
NIG_API int is_contype(char *str);

// 判断str起始位置开始是否包含"content-length"
NIG_API int is_conlength(char *str);


#endif    /*end ifndef*/
