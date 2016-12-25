/*
** 2016-12-15 19:59:20 zhougang
** 测试fifo_1to1.c fifo_1to1.h 加锁版本与非加锁版本性能评测
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

#ifdef LOCK_VERSION_TEST

# define PUT_DATA(fifo, element, elem_num) fifo_1to1_put(fifo, element, elem_num)
# define GET_DATA(fifo, element, elem_num) fifo_1to1_get(fifo, element, elem_num)

#else

# define PUT_DATA(fifo, element, elem_num) __fifo_1to1_put(fifo, element, elem_num)
# define GET_DATA(fifo, element, elem_num) __fifo_1to1_get(fifo, element, elem_num)

#endif

void* produce_task(void *arg)
{
	fifo_1to1 *fifo = (fifo_1to1 *)arg;
	for(;;)
	{
	}
}
