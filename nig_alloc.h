/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#ifndef _NIG_ALLOC_H_INCLUDED_
#define _NIG_ALLOC_H_INCLUDED_

#include "nig_header.h"
#include "nig_config.h"


//使用malloc分配内存空间
void *nig_alloc(size_t size);

//使用malloc分配内存空间，并且将空间内容初始化为0
void *nig_calloc(size_t num, size_t size);

extern nig_uint_t  nig_pagesize;
extern nig_uint_t  nig_pagesize_shift;
extern nig_uint_t  nig_cacheline_size;

#endif /* _NIG_ALLOC_H_INCLUDED_ */
