/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#include "nig_mempool.h"

static void *_palloc_block(pool_t *pool, size_t size);

pool_t *nig_create_pool(size_t size)
{
    pool_t *p;
    p = nig_memalign(POOL_ALIGNMENT, size);
    if(p == NULL)
    {
        return p;
    }

    p->d.last = (u_char *) p + sizeof(pool_t);
    p->d.end = (u_char *)p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    p->max = p->d.end - p->d.last;
    p->current = p;

    return p;
}


void *nig_palloc(pool_t *pool, size_t size)
{
    void *m;
    pool_t *p = pool->current;

    if(size >= p->max)
    {
         return NULL;
     }

     do{
         if(p->d.end - p->d.last > size)
         {
             m = p->d.last;
             p->d.last += size;
             return m;
         }
          p = p->d.next;
    } while(p);

    return _palloc_block(pool, size);
}


static void *_palloc_block(pool_t *pool, size_t size)
{
	pool_t *new_pool;
	pool_t *p;
	pool_t *current;

	size_t pool_size = pool->d.end - (u_char *)pool;

	new_pool = nig_memalign(POOL_ALIGNMENT, pool_size);
	if(new_pool == NULL)
	{
		return NULL;
	}

    //init new_pool
	new_pool->d.last = (u_char *)new_pool + sizeof(pool_t);
	new_pool->d.end = (u_char *)new_pool + pool_size;
	new_pool->d.next = NULL;
	new_pool->d.failed = 0;
    new_pool->max = new_pool->d.end - new_pool->d.last;

    //因为原来的pool_t不能分配才到这个函数，所以从current开始的pool_t块都failed加1
    //当pool->current指向的块的failed超过4时，current指向下一个块
    //其实前面的块失败次数永远  >= 后面的块，所以当一个块failed < 4时，最后的块也不会改变current了
	current = pool->current;
	for(p = current; p->d.next; p = p->d.next)
	{
		if(p->d.failed++ > 4)
		{
			current = p->d.next;
		}
	}

    //无论如何，new_pool都是在单链表的尾部，上面的p沿current开始直到尾部
    p->d.next = new_pool;

    //如果当前的current指向了原本链表最后一块的d.next，那么
    pool->current = current ? current : new_pool;

    void * m = new_pool->d.last;
	new_pool->d.last += size;
	return m;
}


void nig_status_pool(pool_t *pool)
{
    int n = 0;
    pool_t *p = pool;

    printf("**********************************************************************\n");
    for(; p; p = p->d.next, n++)
    {
        printf("pool:%d  ", n);
        printf("max:%d  left:%d\n",
        		p->max,
        		p->d.end - p->d.last);
    }
    printf("**********************************************************************\n");
}

int nig_reset_pool(pool_t *pool)
{
    pool_t *pp = pool;
    pool->current = pool;
    for(; pp ; )
    {
        pp->d.last = (u_char *)pp + sizeof(pool_t);
        pp->d.failed = 0;

        pp = pp->d.next;
    }
    return 0;
}

int nig_destory_pool(pool_t *pool)
{
    pool_t *pp,  *p;

    pp = pool;
    p = pp->d.next;
    for(; pp;)
    {
        pp->d.next = NULL;
        free(pp);
        if(p == NULL)
        {
            break;
        }

        pp = p;
        p = pp->d.next;
    }
    return 0;
}
