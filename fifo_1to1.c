/*
** 2016-12-15 19:59:20 zhougang
** 一个lock-free的循环队列，可用于单消费者-单生产者模式
** 实现方式类似liunux内核的KFIFO数据结构
*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "fifo_1to1.h"

#define BUG_ON(cond) assert(!(cond))

#if defined(__GNUC__) || defined(__x86_64__)
#define mb() \
	__asm__ __volatile__("mfence": : :"memory")

#define rmb() \
	__asm__ __volatile__("": : :"memory")

#define wmb() \
	__asm__ __volatile__("": : :"memory")

#define smp_mb() mb()
#define smp_rmb() rmb()
#define smp_wmb() wmb()

#define min(a,b) ((a) > (b) ? (b): (a))

#else
#error "smp_mb has not been implemented for this architecture."
#endif


/**                                                                                
* __fls - find last (most-significant) set bit in a unsinged int
* Undefined if no set bit exists, so code should check against 0 first.           
* expample: value is 0000 1000 0010 0000 0000 0000 0010 0000 return 27
* from: include/asm-generic/bitops/__fls.h
*/     
static inline unsigned int __fls(unsigned int val)
{
	unsigned int bits_per_int = 32;
	unsigned int num = bits_per_int;

	//if (!(val & (~0ul << (bits_per_int-16)))) {
	if (!(val & 0xffff0000u)) {
		num -= 16; 
		val <<= 16; 
	}   
	//if (!(val & (~0ul << (bits_per_int-8)))) {
	if (!(val & 0xff000000u)) {
		num -= 8;
		val <<= 8;
	}   
	//if (!(val & (~0ul << (bits_per_int-4)))) {
	if (!(val & 0xf0000000u)) {
		num -= 4;
		val <<= 4;
	}   
	//if (!(val & (~0ul << (bits_per_int-2)))) {
	if (!(val & 0xc0000000u)) {
		num -= 2;
		val <<= 2;
	}
	//if (!(val & (~0ul << (bits_per_int-1))))
	if (!(val & 0x80000000u))
		num -= 1;

	return num;
}

static inline unsigned int roundup_pow_of_two(unsigned int x)
{
	return (1U << __fls(x - 1));
}

fifo_1to1* fifo_1to1_init(TYPE *buffer, unsigned int size, lock_t *lock)
{
	fifo_1to1 *fifo;

	/* size must be a power of 2 */
	//printf("size %d, %d %d \n", size, size-1, (size & size-1));
	BUG_ON(size & (size - 1));

	fifo = malloc(sizeof(fifo_1to1));
	if (fifo == NULL)
		return NULL;

	fifo->buffer = buffer;
	fifo->size = size;
	fifo->in = fifo->out = 0;
	fifo->lock = lock;

	return fifo;
}

fifo_1to1* fifo_1to1_alloc(unsigned int size)
{
	TYPE *buffer;
	fifo_1to1 *ret;
	lock_t *lock;

	if (size & (size - 1))
	{
		BUG_ON(size > 0x80000000);
		size = roundup_pow_of_two(size);
	}

	lock = (lock_t*)malloc(sizeof(lock_t));
	if (lock == NULL)
		return NULL;

	buffer = malloc(size * sizeof(TYPE));
	if (buffer == NULL)
	{
		free(lock);
		return NULL;
	}

	ret = fifo_1to1_init(buffer, size, lock);

	if (ret == NULL)
	{
		free(lock);
		free(buffer);
	}

	return ret;
}

void fifo_1to1_free(fifo_1to1 *fifo)
{
	free(fifo->lock);
	free(fifo->buffer);
	free(fifo);
}

unsigned int __fifo_1to1_put(fifo_1to1 *fifo, const TYPE * const element, unsigned int elem_num)
{
	unsigned int num;
	unsigned int type_len = sizeof(TYPE);

	elem_num = min(elem_num, fifo->size - fifo->in + fifo->out);

	/*  
	 * Ensure that we sample the fifo->out index -before- we  
	 * start putting bytes into the kfifo.  
	*/
	smp_mb();  
	

	/* first put the data starting from fifo->in to buffer end */
	num = min(elem_num, fifo->size - (fifo->in & (fifo->size - 1)));
	memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), element, num * type_len);

	/* then put the rest (if any) at the beginning of the buffer */
	memcpy(fifo->buffer, element + num, (elem_num - num) * type_len);

	/*
	 * Ensure that we add the bytes to the kfifo -before-
	 *  we update the fifo->in index.
	*/
	smp_wmb(); 

	fifo->in += elem_num;

	return elem_num;
}

unsigned int __fifo_1to1_get(fifo_1to1 *fifo, TYPE *element, unsigned int elem_num)
{
	unsigned int num;
	unsigned int type_len = sizeof(TYPE);

	elem_num = min(elem_num, fifo->in - fifo->out);

    /*
	 * Ensure that we sample the fifo->in index -before- we 
	 * start removing bytes from the kfifo.
	*/
	smp_rmb();

	/* first get the data from fifo->out until the end of the buffer */
	num = min(elem_num, fifo->size - (fifo->out & (fifo->size - 1)));
	memcpy(element, fifo->buffer + (fifo->out & (fifo->size - 1)), num * type_len);

	/* then get the rest (if any) from the beginning of the buffer */
	memcpy(element + num, fifo->buffer, (elem_num - num) * type_len);

	/*
	 * Ensure that we remove the bytes from the kfifo -before-
	 * we update the fifo->out index.
	*/
	smp_mb();

	fifo->out += elem_num;


	return elem_num;
}
