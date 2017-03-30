#ifndef _NIG__THRPOOL_H_INCLUDED_
#define _NIG__THRPOOL_H_INCLUDED_

#include <pthread.h>

#include "nig_header.h"
#include "nig_config.h"

typedef int (*thrpool_taskfn)(void *param);

void * thrpool_create(int workers);
void thrpool_destroy(void * pool, int wait);
int thrpool_add_task(void * pool, thrpool_taskfn taskfn, void *param);
void thrpool_wait(void * pool);

#endif /* !_NIG__THRPOOL_H_INCLUDED_ */
