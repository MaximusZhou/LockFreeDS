/*
** 2016-12-15 19:59:20 zhougang
** 测试fifo_1to1.c fifo_1to1.h 加锁版本与非加锁版本性能评测
** gcc -Wall -Wno-unused-function -lpthread fifo_1to1.c fifo_1to1_benchmark.c  -o fifo_1to1_benchmark
** time ./fifo_1to1_benchmark [1 [10000000]]
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

#define BUFFER_SIZE 1022

static int is_test_lock_type = 0;
static int max_test_num = 10000000;

/*
static int *data[1] = {NULL,};
static int *get_data[1];
*/

/*
static int produce_num = 0;
static int consume_num = 0;
*/

void* produce_task(void *arg)
{
	fifo_1to1 *fifo = (fifo_1to1 *)arg;
	int produce_num = 0;
	int *data[1];

	int *p = (int *)malloc(sizeof(int));
	if (p == NULL)
	{
		printf("malloc data fail in produce task:%u, reason:%s\n", errno, strerror(errno));
		return ((void*)0);
	}
	(*p) = 42;

	data[0] = p;
	for(;;)
	{
		int put_num;

		if (is_test_lock_type)
		{
			put_num = fifo_1to1_put(fifo, (TYPE *)data, 1);
		}
		else
		{
			put_num = __fifo_1to1_put(fifo, (TYPE *)data, 1);
		}

		if (put_num <= 0)
		{
			continue;
		}
		//printf("produce0:%d %d\n", produce_num, put_num);

		produce_num ++;
		if (produce_num >= max_test_num)
			return ((void*)0);
	}
	return((void *)0);
}

void* consume_task(void *arg)
{
	fifo_1to1 *fifo = (fifo_1to1 *)arg;
	int consume_num = 0;
	int *get_data[1];

	for(;;)
	{
		int get_num;

		if (is_test_lock_type)
		{
			get_num = fifo_1to1_get(fifo, (TYPE *)get_data, 1);
		}
		else
		{
			get_num = __fifo_1to1_get(fifo, (TYPE *)get_data, 1);
		}
		if (get_num <= 0)
		{
			continue;
		}
		//printf("consume:%d %d %d\n", consume_num, get_num, *(get_data[0]));

		consume_num ++;
		if (consume_num >= max_test_num)
			return ((void*)0);
	}
	return((void *)0);
}

int main(int argc, char** argv)
{
	int err;
	fifo_1to1* fifo;
	pthread_t p_tid, c_tid;

	if (argc > 1)
	{
		is_test_lock_type = atoi(argv[1]);
	}

	if (argc > 2)
	{
		max_test_num = atoi(argv[2]);
	}

	if (is_test_lock_type)
	{
		printf("======TEST LOCK VERSION START======\n");
	}
	else
	{
		printf("======TEST LOCK FREE VERSION START======\n");
	}

	fifo = fifo_1to1_alloc(BUFFER_SIZE);
	if (fifo == NULL)
	{
		fprintf(stderr,"fail init fifo:%u, reason:%s\n", errno, strerror(errno));
		exit(1);
	}

	printf("start create produce thread.\n");
	err = pthread_create(&p_tid, NULL, produce_task, (void *)fifo);
	if (err != 0)
	{
		printf("can't create thread: %s\n", strerror(err));
		exit(1);
	}
	printf("end create produce thread.\n");

	printf("start create consume thread.\n");
	err = pthread_create(&c_tid, NULL, consume_task, (void *)fifo);
	if (err != 0)
	{
		printf("can't create thread: %s\n", strerror(err));
		exit(1);
	}
	printf("end create consume thread.\n");

	printf("produce thread join.\n");
	err = pthread_join(p_tid, NULL);
	if (err != 0)
	{
		printf("can't join thread: %s\n", strerror(err));
		exit(1);
	}
	printf("produce thread end.\n");

	printf("consume thread join.\n");
	err = pthread_join(c_tid, NULL);
	if (err != 0)
	{
		printf("can't join thread: %s\n", strerror(err));
		exit(1);
	}
	printf("consume thread end.\n");

	fifo_1to1_free(fifo);	

	if (is_test_lock_type)
	{
		printf("======TEST LOCK VERSION END======\n");
	}
	else
	{
		printf("======TEST LOCK FREE VERSION END======\n");
	}
	return 0;
}
