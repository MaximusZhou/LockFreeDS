/*
** 2016-12-15 19:59:20 zhougang
** 一个lock-free的循环队列，可用于单消费者-单生产者模式
** 实现方式类似liunux内核的KFIFO数据结构
*/

#ifndef _FIFO_1TO1_H
#define _FIFO_1TO1_H

#include <pthread.h>

#define lock_op(lock_ptr) pthread_mutex_lock(lock_ptr)
#define unlock_op(lock_ptr) pthread_mutex_unlock(lock_ptr)

typedef pthread_mutex_t lock_t;
typedef void* TYPE;

typedef struct fifo_1to1 {
	TYPE *buffer;               //保存数据的buff
	unsigned int size;          //分配的buff大小
	unsigned int in;            //下一个数据保存的位置(in % size)
	unsigned int out;           //下一个取出的数据保存的位置(out % size)
	lock_t *lock;           //针对多消费者-多生产者模式加的锁
} fifo_1to1;


extern fifo_1to1* fifo_1to1_init(TYPE *buffer, unsigned int size, lock_t *lock);
extern fifo_1to1* fifo_1to1_alloc(unsigned int size, lock_t *lock);
extern void fifo_1to1_free(fifo_1to1 *fifo);
extern unsigned int __fifo_1to1_put(fifo_1to1 *fifo, const TYPE * const element);
extern unsigned int __fifo_1to1_get(fifo_1to1 *fifo, TYPE *element);


//无锁版本清除FIFO中所有的数据
static inline void __fifo_1to1_reset(fifo_1to1 *fifo)
{
	fifo->in = fifo->out = 0;
}

//加锁版本清除FIFO中所有的数据
static inline void fifo_1to1_reset(fifo_1to1 *fifo)
{
	lock_op(fifo->lock);

	__fifo_1to1_reset(fifo);

	unlock_op(fifo->lock);
}

//无锁版获取FIFO中元素个数
static inline unsigned int __fifo_1to1_len(fifo_1to1 *fifo)
{
	return fifo->in - fifo->out;
}

//加锁版获取FIFO中元素个数
static inline unsigned int fifo_1to1_len(fifo_1to1 *fifo)
{
	unsigned int ret;

	lock_op(fifo->lock);

	ret = __fifo_1to1_len(fifo);

	unlock_op(fifo->lock);

	return ret;
}

//加锁版往FIFO中加入一个元素
static inline unsigned int fifo_1to1_put(fifo_1to1 *fifo, const TYPE * const element)
{
	unsigned int ret;

	lock_op(fifo->lock);

	ret = __fifo_1to1_put(fifo, element);

	unlock_op(fifo->lock);

	return ret;
}

//加锁版从FIFO中提取一个元素
static unsigned int __fifo_1to1_get(fifo_1to1 *fifo, TYPE *element)
{
	unsigned int ret;

	lock_op(fifo->lock);

	ret = __fifo_1to1_get(fifo, element);

	//如果为空，则重置in out到初始位置
	if (fifo->in == fifo->out)
		fifo->in = fifo->out = 0;

	unlock_op(fifo->lock);

	return ret;
}


#endif

