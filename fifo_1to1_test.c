/*
** 2016-12-15 19:59:20 zhougang
** 测试fifo_1to1实现
** gcc -Wall -Wno-unused-function -lpthread fifo_1to1.c fifo_1to1_test.c  -o fifo_1to1_test
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "fifo_1to1.h"

#define BUFFER_SIZE 1023

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

#define USER_NUM 1000000

static user_info produce_backup[USER_NUM];
static int pb_count = 0;
pthread_mutex_t pb_lock = PTHREAD_MUTEX_INITIALIZER;

static user_info consume_backup[USER_NUM];
static int cb_count = 0;
pthread_mutex_t cb_lock = PTHREAD_MUTEX_INITIALIZER;

void* produce_task(void *arg)
{
	//time_t cur_time;
	fifo_1to1 *fifo = (fifo_1to1 *)arg;
	unsigned int tid = (unsigned int)pthread_self();
	for(;;)
	{
		unsigned int uid;
		unsigned int lv;
		user_info *info;
		int put_num;

		//检查是否结束了
		if (PRODUCE_THREAD_NUM > 1)
			pthread_mutex_lock(&pb_lock);
		if (pb_count >= USER_NUM)
		{
			if (PRODUCE_THREAD_NUM > 1)
				pthread_mutex_unlock(&pb_lock);
			return((void *)0);
		}
		if (PRODUCE_THREAD_NUM > 1)
			pthread_mutex_unlock(&pb_lock);

		/*
		time(&cur_time);
		srand(cur_time);
		*/
		
		info = (user_info*)malloc(sizeof(user_info));
		if (info == 0)
		{
			fprintf(stderr,"produce malloc fail:%u, reason:%s\n", errno, strerror(errno));
			return((void *)0);
		}

		uid = rand() + tid;
		lv = rand() % 100;
		info->uid = uid; 
		info->lv = lv;

		put_num = PUT_DATA(fifo, (TYPE *)&info, 1);
		if (put_num <= 0)
		{
			continue;
		}
		
		//生成的数据备份
		if (PRODUCE_THREAD_NUM > 1)
			pthread_mutex_lock(&pb_lock);
		if (pb_count >= USER_NUM)
		{
			if (PRODUCE_THREAD_NUM > 1) 
				pthread_mutex_unlock(&pb_lock);
			return((void *)0);
		}
		if ((PRODUCE_THREAD_NUM == 1) && (CONSUME_THREAD_NUM == 1))
		{
			produce_backup[pb_count].uid = uid;
			produce_backup[pb_count].lv = lv;
		}
		pb_count++;
		if (PRODUCE_THREAD_NUM > 1) 
			pthread_mutex_unlock(&pb_lock);
		usleep(10);
	}

	return((void *)0);
}

void* consume_task(void *arg)
{
	fifo_1to1 *fifo = (fifo_1to1 *)arg;
	//unsigned int tid = (unsigned int)pthread_self();
	for(;;)
	{
		user_info *info;
		unsigned int uid;
		unsigned int lv;
		int get_num;

		//检查是否结束了
		if (CONSUME_THREAD_NUM > 1)
			pthread_mutex_lock(&cb_lock);
		if (cb_count >= USER_NUM) //TODO: 可能fifo队列中有元素还free
		{
			if (CONSUME_THREAD_NUM > 1)
				pthread_mutex_unlock(&cb_lock);
			return((void *)0);
		}
		if (CONSUME_THREAD_NUM > 1)
			pthread_mutex_unlock(&cb_lock);

		get_num = GET_DATA(fifo, (TYPE *)&info, 1);
		if (get_num <= 0)
		{
			continue;
		}

		uid = info->uid;
		lv = info->lv;
		free(info);
		//消费的数据备份
		if (CONSUME_THREAD_NUM > 1)
			pthread_mutex_lock(&cb_lock);
		if (cb_count >= USER_NUM)
		{
			if (CONSUME_THREAD_NUM > 1)
				pthread_mutex_unlock(&cb_lock);
			return((void *)0);
		}
		if ((PRODUCE_THREAD_NUM == 1) && (CONSUME_THREAD_NUM == 1))
		{
			consume_backup[cb_count].uid = uid;
			consume_backup[cb_count].lv = lv;
		}
		cb_count++;
		if (CONSUME_THREAD_NUM > 1)
			pthread_mutex_unlock(&cb_lock);
		usleep(10);
	}
	return((void *)0);
}


int main()
{
	int i;
	fifo_1to1* fifo;
	pthread_t p_tids[PRODUCE_THREAD_NUM];
	pthread_t c_tids[CONSUME_THREAD_NUM];
	
	fifo = fifo_1to1_alloc(BUFFER_SIZE);
	if (fifo == NULL)
	{
		fprintf(stderr,"fail init fifo:%u, reason:%s\n", errno, strerror(errno));
		exit(1);
	}

	printf("start create produce thread.\n");
	for(i = 0; i < PRODUCE_THREAD_NUM; i ++)
	{
		int err = pthread_create(&(p_tids[i]), NULL, produce_task, (void *)fifo);
		if (err != 0)
		{
			printf("can't create thread: %s\n", strerror(err));
			exit(1);
		}
	}
	printf("end create produce thread.\n");

	printf("start create consume thread.\n");
	for(i = 0; i < CONSUME_THREAD_NUM; i ++)
	{
		int err = pthread_create(&(c_tids[i]), NULL, consume_task, (void *)fifo);
		if (err != 0)
		{
			printf("can't create thread: %s\n", strerror(err));
			exit(1);
		}
	}
	printf("end create consume thread.\n");

	printf("produce thread join.\n");
	for(i = 0; i < PRODUCE_THREAD_NUM; i ++)
	{
		int err = pthread_join(p_tids[i], NULL);
		if (err != 0)
		{
			printf("can't join thread: %s\n", strerror(err));
			exit(1);
		}
	}
	printf("produce thread end.\n");

	printf("consume thread join.\n");
	for(i = 0; i < CONSUME_THREAD_NUM; i ++)
	{
		int err = pthread_join(c_tids[i], NULL);
		if (err != 0)
		{
			printf("can't join thread: %s\n", strerror(err));
			exit(1);
		}
	}
	printf("consume thread end.\n");

	fifo_1to1_free(fifo);	

	//比较数据是否相同
	if (pb_count != cb_count)
	{
		printf("produce count is not equal consume count errro: %d %d\n", pb_count, cb_count);
		exit(1);
	}

	if ((PRODUCE_THREAD_NUM == 1) && (CONSUME_THREAD_NUM == 1))
	{
		printf("start compare produce data order and consume data order.\n");
		for(i = 0; i < pb_count; i ++)
		{
			if ((produce_backup[i].uid != consume_backup[i].uid) ||
					(produce_backup[i].lv != consume_backup[i].lv))
			{
				printf("produce count is not equal consume count errro idx %d: %u %u %u %u\n",
											  i, produce_backup[i].uid, consume_backup[i].uid,
											  produce_backup[i].lv, consume_backup[i].lv);
				exit(1);
			}
		}
		printf("end compare produce data order and consume data order.\n");
	}

	printf("test end.\n");

	return 0;
}
