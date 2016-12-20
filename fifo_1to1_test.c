/*
** 2016-12-15 19:59:20 zhougang
** 测试fifo_1to1实现
*/

#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>

#include "fifo_1to1.h"

#define BUFFER_SIZE (1024 * 1024)

#define PRODUCE_THREAD_NUM 1
#define CONSUME_THREAD_NUM 1

#if PRODUCE_THREAD_NUM > 1
# define PUT_DATA(fifo, element, elem_num) fifo_1to1_put(fifo, element, elem_num)
#else
# define PUT_DATA(fifo, element, elem_num) __fifo_1to1_put(fifo, element, elem_num)
#endif

#if CONSUME_THREAD_NUM > 1
# define GET_DATA(fifo, element, elem_num) fifo_1to1_get(fifo, element, elem_num)
#else
# define GET_DATA(fifo, element, elem_num) __fifo_1to1_get(fifo, element, elem_num)
#endif

typedef struct user_info {
	unsigned int uid;
	unsigned int lv;
} user_info;

#define USER_NUM 10000

static user_info produce_backup[USER_NUM];
static int pb_count = 0;
pthread_mutex_t pb_lock = PTHREAD_MUTEX_INITIALIZER;

static user_info consume_backup[USER_NUM];
static int cb_count = 0;
pthread_mutex_t cb_lock = PTHREAD_MUTEX_INITIALIZER;

void* produce_task(void *arg)
{
	time_t cur_time;
	fifo_1to1 *fifo = (fifo_1to1 *)arg;
	for(;;)
	{
		user_info *info;
		//检查是否结束了
		pthread_mutex_lock(&pb_lock);
		if (pb_count >= USER_NUM)
		{
			pthread_mutex_unlock(&pb_lock);
			return((void *)0);
		}
		pthread_mutex_unlock(&pb_lock);

		time(&cur_time);
		srand(cur_time);
		
		info = (user_info*)malloc(user_info);
		if (info == 0)
		{
			fprintf(stderr,"produce malloc fail:%u, reason:%s\n", errno, strerror(errno));
			return((void *)0);
		}
		info->uid = rand();
		info->lv = (rand() % 100);

		PUT_DATA(fifo, info, 1);
		
		//生成的数据备份
		pthread_mutex_lock(&pb_lock);
		if (pb_count >= USER_NUM)
		{
			pthread_mutex_unlock(&pb_lock);
			return((void *)0);
		}
		produce_backup[pb_count] = {info->uid, info->lv};
		pb_count++;
		pthread_mutex_unlock(&pb_lock);

	}

	return((void *)0);
}

void* consume_task(void *arg)
{
	fifo_1to1 *fifo = (fifo_1to1 *)arg;
	for(;;)
	{
		user_info info;
		//检查是否结束了
		pthread_mutex_lock(&cb_lock);
		if (cb_count >= USER_NUM)
		{
			pthread_mutex_unlock(&cb_lock);
			return((void *)0);
		}
		pthread_mutex_unlock(&cb_lock);

		GET_DATA(fifo, &info, 1);

		//消费的数据备份
	}
	return((void *)0);
}


int main()
{
	fifo_1to1* fifo;
	pthread_t p_tids[PRODUCE_THREAD_NUM];
	pthread_t c_tids[CONSUME_THREAD_NUM];
	
	fifo = fifo_1to1_alloc(BUFFER_SIZE);
	if (buff == NULL)
	{
		fprintf(stderr,"fail init fifo:%u, reason:%s\n", errno, strerror(errno));
	}

	for(i = 0; i < PRODUCE_THREAD_NUM; i ++)
	{
		err = pthread_create(&(p_tids[i]), NULL, produce_task, (void *)fifo);
		if (err != 0)
		{
			printf("can't create thread: %s\n", strerror(err));
			exit(1);
		}
	}

	for(i = 0; i < CONSUME_THREAD_NUM; i ++)
	{
		err = pthread_create(&(c_tids[i]), NULL, consume_task, (void *)fifo);
		if (err != 0)
		{
			printf("can't create thread: %s\n", strerror(err));
			exit(1);
		}
	}


	for(i = 0; i < PRODUCE_THREAD_NUM; i ++)
	{
		err = pthread_join(p_tids[i], NULL);
		if (err != 0)
		{
			printf("can't join thread: %s\n", strerror(err));
			exit(1);
		}
	}

	for(i = 0; i < CONSUME_THREAD_NUM; i ++)
	{
		err = pthread_join(c_tids[i], NULL);
		if (err != 0)
		{
			printf("can't join thread: %s\n", strerror(err));
			exit(1);
		}
	}

	fifo_1to1_free(fifo);	

	return 0;
}

