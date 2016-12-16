/*
** 2016-12-15 19:59:20 zhougang
** һ��lock-free��ѭ�����У������ڵ�������-��������ģʽ
** ʵ�ַ�ʽ����liunux�ں˵�KFIFO���ݽṹ
*/

#ifndef _FIFO_1TO1_H
#define _FIFO_1TO1_H

#include <pthread.h>

#define lock_op(lock_ptr) pthread_mutex_lock(lock_ptr)
#define unlock_op(lock_ptr) pthread_mutex_unlock(lock_ptr)

typedef pthread_mutex_t lock_t;
typedef void* TYPE;

typedef struct fifo_1to1 {
	TYPE *buffer;               //�������ݵ�buff
	unsigned int size;          //�����buff��С
	unsigned int in;            //��һ�����ݱ����λ��(in % size)
	unsigned int out;           //��һ��ȡ�������ݱ����λ��(out % size)
	lock_t *lock;           //��Զ�������-��������ģʽ�ӵ���
} fifo_1to1;


extern fifo_1to1* fifo_1to1_init(TYPE *buffer, unsigned int size, lock_t *lock);
extern fifo_1to1* fifo_1to1_alloc(unsigned int size, lock_t *lock);
extern void fifo_1to1_free(fifo_1to1 *fifo);
extern unsigned int __fifo_1to1_put(fifo_1to1 *fifo, const TYPE * const element);
extern unsigned int __fifo_1to1_get(fifo_1to1 *fifo, TYPE *element);


//�����汾���FIFO�����е�����
static inline void __fifo_1to1_reset(fifo_1to1 *fifo)
{
	fifo->in = fifo->out = 0;
}

//�����汾���FIFO�����е�����
static inline void fifo_1to1_reset(fifo_1to1 *fifo)
{
	lock_op(fifo->lock);

	__fifo_1to1_reset(fifo);

	unlock_op(fifo->lock);
}

//�������ȡFIFO��Ԫ�ظ���
static inline unsigned int __fifo_1to1_len(fifo_1to1 *fifo)
{
	return fifo->in - fifo->out;
}

//�������ȡFIFO��Ԫ�ظ���
static inline unsigned int fifo_1to1_len(fifo_1to1 *fifo)
{
	unsigned int ret;

	lock_op(fifo->lock);

	ret = __fifo_1to1_len(fifo);

	unlock_op(fifo->lock);

	return ret;
}

//��������FIFO�м���һ��Ԫ��
static inline unsigned int fifo_1to1_put(fifo_1to1 *fifo, const TYPE * const element)
{
	unsigned int ret;

	lock_op(fifo->lock);

	ret = __fifo_1to1_put(fifo, element);

	unlock_op(fifo->lock);

	return ret;
}

//�������FIFO����ȡһ��Ԫ��
static unsigned int __fifo_1to1_get(fifo_1to1 *fifo, TYPE *element)
{
	unsigned int ret;

	lock_op(fifo->lock);

	ret = __fifo_1to1_get(fifo, element);

	//���Ϊ�գ�������in out����ʼλ��
	if (fifo->in == fifo->out)
		fifo->in = fifo->out = 0;

	unlock_op(fifo->lock);

	return ret;
}


#endif

