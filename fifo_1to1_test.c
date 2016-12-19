/*
** 2016-12-15 19:59:20 zhougang
** 测试fifo_1to1实现
*/

#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <assert.h>
#include <pthread.h>

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

int main()
{
	fifo_1to1* fifo;
	pthread_t produce_pid;
	pthread_t consume_pid;
	
	fifo = fifo_1to1_alloc(BUFFER_SIZE);
	if (buff == NULL)
	{
		fprintf(stderr,"fail init fifo:%u, reason:%s\n", errno, strerror(errno));
	}


	pthread_join(produce_pid, NULL);
	pthread_join(consume_pid, NULL);

	fifo_1to1_free(fifo);	

	return 0;
}

