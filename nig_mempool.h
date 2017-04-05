/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
    * 使用案例 :

	typedef struct
	{
	    int a[10];
	} data_t;

	pool_t *pool;
	data_t *  p[20];

	size_t size = 128;
	pool = nig_create_pool(size);

	nig_status_pool(pool);

    int i;
	for(i = 0; i < 10 ;i++)
	{
		p[i] = nig_palloc(pool, sizeof(data_t));
		p[i]->a[0] = i;
		p[i]->a[1] = i+1;
		p[i]->a[2] = i+2;
		p[i]->a[3] = i+3;
	}

	nig_status_pool(pool);

	nig_reset_pool(pool);

	nig_status_pool(pool);

	nig_destory_pool(pool);
	pool = NULL;
	nig_status_pool(pool);

	return 1;
 ************************************************************************/

#ifndef _NIG_MEMPOOL_H_INCLUDED_
#define _NIG_MEMPOOL_H_INCLUDED_

#include "nig_header.h"
#include "nig_config.h"
#include "nig_alloc.h"

#define POOL_ALIGNMENT 16  /*32位机器8  64位机器16*/

typedef  unsigned char u_char;
typedef struct pool_data_s pool_data_t;
typedef struct pool_s pool_t;

struct pool_data_s
{
    u_char *last; //每一小块的结尾位置
    u_char *end;  //最后一小块内存的位置
    pool_t *next;
    int failed;  //错误次数
};

struct pool_s
{
    pool_data_t    d;      //pool_data_s 结构体
    size_t         max;    //整块内存长度
    pool_t        *current;//指向自己
};

/**
 * 创建一块内存 (让pool_s 结构体类型指针 指向它)
 * @access public
 * @param size_t size
 * @return pool_t 结构体 (pool_s)
 */
pool_t *nig_create_pool(size_t size);

/**
 * 分出一块指定大小的内存
 * @access public
 * @param pool_t * pool
 * @param size_t size
 * @return (void *)
 */
void *nig_palloc(pool_t *pool, size_t size);

/**
 * 清空大块内存
 * @access public
 * @param pool_t * pool
 * @return int
 */
int nig_destory_pool(pool_t *pool);

/**
 * 重置大块内存
 * @access public
 * @param pool_t * pool
 * @return int
 */
int nig_reset_pool(pool_t *pool);

/**
 * 当前内存状态
 * @access public
 * @param pool_t * pool
 * @return int
 */
void nig_status_pool(pool_t *pool);

#endif /*_NIG_MEMPOOL_H_INCLUDED_*/
