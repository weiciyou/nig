/*************************************************************************
    * File: 1-1.c
    * Brief:
    * Author: luoyuxiang
    * Mail: 269724033@qq.com
    * Created Time: Sat 17 Dec 2016 08:00:39 PM CST
 ************************************************************************/

#include "nig_thrpool.h"

struct thrpool_task {
	thrpool_taskfn taskfn;
	void *param;
	struct thrpool_task *next;
};

struct thrpool_internal {
	int min_workers;
	pthread_cond_t worker_c;//线程条件变量A
	pthread_mutex_t worker_m;//线程互斥锁 1
	pthread_cond_t task_c;//线程条件变量B
	pthread_cond_t task_0_c;//线程条件变量C
	pthread_mutex_t task_m;//线程互斥锁  2
	volatile int destroy;
	volatile int running_workers;
	volatile int wait_task_0;
	struct thrpool_task *task_front;
	struct thrpool_task *task_rear;
};

static void _worker_cleanup(void * param)
{
	struct thrpool_internal *ti;

	ti = (struct thrpool_internal *)param;
	pthread_mutex_lock(&ti->worker_m);
	ti->running_workers--;
	pthread_mutex_unlock(&ti->worker_m);
	pthread_cond_signal(&ti->worker_c);
}

static void _task_enq(struct thrpool_internal *ti, struct thrpool_task *t)
{
	pthread_mutex_lock(&ti->task_m); //互斥锁  start
	if (ti->task_front == NULL) {
		ti->task_front = t;
	} else {
		ti->task_rear->next = t;
	}
	ti->task_rear = t;
	pthread_mutex_unlock(&ti->task_m); //互斥锁 end
	pthread_cond_signal(&ti->task_c);
}

static void *_worker(void *param)
{
	struct thrpool_internal *ti = (struct thrpool_internal *)param;
	struct thrpool_task *t;

	pthread_cleanup_push(_worker_cleanup, ti); //注册在退出控制流时调用的函数
	for (;;)
	{
		t = NULL;

		pthread_mutex_lock(&ti->task_m); // 任务锁 start
		while (ti->task_front == NULL)
		{
			if (ti->wait_task_0)
			{
				ti->wait_task_0 = 0;
				pthread_cond_broadcast(&ti->task_0_c); // 通知所有的线程
			}

			if (ti->destroy)
			{
				break;
			}
			pthread_cond_wait(&ti->task_c, &ti->task_m);////在此处阻塞  等待被通知
		}

		if (ti->task_front != NULL)
		{
			t = ti->task_front;
			ti->task_front = t->next;
		}
		pthread_mutex_unlock(&ti->task_m);// 任务锁 end


		if (t == NULL) {
			break;
		}
		if (t->taskfn(t->param)) {
			t->next = NULL;
			_task_enq(ti, t);
		} else {
			free(t);
		}
	}
	pthread_cleanup_pop(1);
	return NULL;
}

void * thrpool_create(int workers)
{
	int r;
	int i;
	pthread_t t;//用于返回成功创建新线程的线程 ID

	if (workers < 0) {
		return NULL;
	}

	struct thrpool_internal *ti = (struct thrpool_internal *)malloc(sizeof(struct thrpool_internal));
	memset(ti, 0, sizeof(struct thrpool_internal));
	ti->min_workers = workers;
	pthread_cond_init(&ti->worker_c, NULL);//参数2为空指针等价于ti->worker_c中的属性为缺省属性
	pthread_mutex_init(&ti->worker_m, NULL);
	pthread_cond_init(&ti->task_c, NULL); //线程条件变量初始化
	pthread_cond_init(&ti->task_0_c, NULL);
	pthread_mutex_init(&ti->task_m, NULL);//动态的创建锁
	r = 0;
	for (i = 0; i < workers; i++) {
		if (pthread_create(&t, NULL, _worker, ti) != 0) {
			r = 1;
			break;
		}
//joinable状态，当线程函数自己返回退出时或pthread_exit时都不会释放线程所占用堆栈和线程描述符（总计8K多）
//unjoinable状态的线程，这些资源在线程函数退出时或pthread_exit时自动会被释放。

       //pthread_detach(pthread_self())，将状态改为unjoinable状态，确保资源的释放。
		pthread_detach(t);
		ti->running_workers++;
	}

	if (r == 1) {
		thrpool_destroy((void * )ti, 0);
		ti = NULL;
	}
	return (void * )ti;
}

void thrpool_destroy(void * pool, int wait)
{
	struct thrpool_internal *ti;
	struct thrpool_task *t;
	struct thrpool_task *next;

	if (pool == NULL) {
		return;
	}
	ti = (struct thrpool_internal *)pool;
	if (ti->destroy) {
		return;
	}
	if (!wait) {
		pthread_mutex_lock(&ti->task_m);
		t = ti->task_front;
		ti->task_front = NULL;
		pthread_mutex_unlock(&ti->task_m);
		while (t != NULL) {
			next = t->next;
			free(t);
			t = next;
		}
	}
	ti->destroy = 1;
	pthread_mutex_lock(&ti->worker_m);
	while (ti->running_workers > 0) {
		pthread_cond_signal(&ti->task_c);
		pthread_cond_wait(&ti->worker_c, &ti->worker_m);
	}
	pthread_mutex_unlock(&ti->worker_m);
	pthread_cond_destroy(&ti->worker_c);
	pthread_mutex_destroy(&ti->worker_m);
	pthread_cond_destroy(&ti->task_c);
	pthread_cond_destroy(&ti->task_0_c);
	pthread_mutex_destroy(&ti->task_m);
	free(ti);
}



int thrpool_add_task(void * pool, thrpool_taskfn taskfn, void *param)
{
	struct thrpool_internal *ti;
	struct thrpool_task *t;
	
	if (pool == NULL || taskfn == NULL) {
		return 1;
	}
	ti = (struct thrpool_internal *)pool;
	if (ti->destroy) {
		return 1;
	}
	t = (struct thrpool_task *)malloc(sizeof(struct thrpool_task));
	t->taskfn = taskfn;
	t->param = param;
	t->next = NULL;
	_task_enq(ti, t);
	return 0;
}

void thrpool_wait(void * pool)
{
	struct thrpool_internal *ti;
	
	if (pool == NULL) {
		return;
	}
	ti = (struct thrpool_internal *)pool;
	if (ti->destroy) {
		return;
	}
	pthread_mutex_lock(&ti->task_m);
	if (ti->task_front != NULL) {
		ti->wait_task_0 = 1;
		pthread_cond_wait(&ti->task_0_c, &ti->task_m);
	}
	pthread_mutex_unlock(&ti->task_m);
}
