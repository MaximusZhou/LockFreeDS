/*
** 2016-12-15 19:59:20 zhougang
** 一个lock-free的循环队列，可用于单消费者-单生产者模式
** 实现方式类似liunux内核的KFIFO数据结构
*/

#include <stdlib.h>
#include <assert.h>

#include "fifo_1to1.h"

#define BUG_ON(cond) assert(!cond)

#if defined(__GNUC__) || defined(__x86_64__)
#define TPOOL_COMPILER_BARRIER() __asm__ __volatile("" : : : "memory")

static inline void FullMemoryBarrier()
{
	    __asm__ __volatile__("mfence": : : "memory");
}

#define smp_mb() FullMemoryBarrier()
#define smp_rmb() TPOOL_COMPILER_BARRIER()
#define smp_wmb() TPOOL_COMPILER_BARRIER()

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
	unsigned int num = bits_per_int - 1;

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
	return (1U << fls(x - 1));
}

fifo_1to1* fifo_1to1_init(TYPE *buffer, unsigned int size, lock_t *lock)
{
	fifo_1to1 *fifo;

	/* size must be a power of 2 */
	BUG_ON(size & (size - 1));

	fifo = malloc(sizeof(struct kfifo));
	if (!fifo)
		return NULL;

	fifo->buffer = buffer;
	fifo->size = size;
	fifo->in = fifo->out = 0;
	fifo->lock = lock;

	return fifo;
}

fifo_1to1* fifo_1to1_alloc(unsigned int size, lock_t *lock)
{
	TYPE *buffer;
	fifo_1to1 *ret;

	if (size & (size - 1)) {
		BUG_ON(size > 0x80000000);
		size = roundup_pow_of_two(size);
	}

	buffer = malloc(size * sizeof(TYPE));
	if (!buffer)
		return NULL;

	ret = fifo_1to1_init(buffer, size, lock);

	if (ret == NULL)
		free(buffer);

	return ret;
}

void fifo_1to1_free(fifo_1to1 *fifo);
{
	free(fifo->buffer);
	free(fifo);
}
