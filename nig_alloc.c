/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/


#include "nig_alloc.h"

nig_uint_t  nig_pagesize;
nig_uint_t  nig_pagesize_shift;
nig_uint_t  nig_cacheline_size;


void *
nig_alloc(size_t size)
{
    void  *p;
    p = malloc(size);
    if (p == NULL) {
        LOGS("malloc(%uz) failed", size);
    }
    return p;
}

void *
nig_calloc(size_t num, size_t size)
{
    void  *p;
    p = calloc(num, size);
    //calloc() 在动态分配完内存后，自动初始化该内存空间为零，而 malloc() 不初始化，里边数据是未知的垃圾数据
    if (p == NULL) {
        LOGS("malloc(%uz) failed", size);
    }
    return p;
}


