/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/


#include "nig_string.h"

NIG_API nig_str_t *nig_string( char * str )
{
	unsigned int len;
    nig_str_t *ptr = ( nig_str_t * ) nig_alloc( sizeof( nig_str_t ) );
    if ( str == NULL )
    {
    	return NULL;
    }

    len = (size_t) strlen( str );
    ptr->str = ( char * ) nig_alloc( len );
    if ( ptr->str == NULL )
    {
        free( ptr );
        return NULL;
    }
    //将字符串复制到堆。
    memcpy( ptr->str, str, len );
    ptr->len = len;
    return ptr;
}


NIG_API void nig_free_string( nig_str_t **cstr )
{
    if ( cstr == NULL )
    {
    	return;
    }

    if ( *cstr != NULL )
    {
        free( (*cstr)->str );
        (*cstr)->str = NULL;

        free( *cstr );
        *cstr = NULL;
    }
}



/*
 * 将str前n个字符转换为小写
 */
NIG_API void strtolow(char *str, int n)
{
    char *cur = str;
    while (n > 0) {
        *cur = tolower(*cur);
        cur++;
        n--;
    }
}

/*
 * 判断str起始位置开始是否包含"content-type"
 * 包含返回1
 * 不包含返回0
 */
NIG_API int is_contype(char *str)
{
    char *cur = str;
    char *cmp = "content-type";

    // 删除开始的空格
    while (*cur == ' ') {
        cur++;
    }

    for (; *cmp != '\0' && tolower(*cur) == *cmp; cur++,cmp++)
        ;

    if (*cmp == '\0') { // cmp字符串以0结束
        return 1;
    }

    return 0;
}

/*
 * 判断str起始位置开始是否包含"content-length"
 * 包含返回1
 * 不包含返回0
 */
NIG_API int is_conlength(char *str)
{
    char *cur = str;
    char *cmp = "content-length";

    // 删除开始的空格
    while (*cur == ' ') {
        cur++;
    }

    for (; *cmp != '\0' && tolower(*cur) == *cmp; cur++,cmp++)
        ;

    if (*cmp == '\0') { // cmp字符串以0结束
        return 1;
    }

    return 0;
}

