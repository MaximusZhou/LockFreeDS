/*
** 2016-12-15 19:59:20 zhougang
** 一个lock-free的循环队列，可用于单消费者-单生产者模式
** 实现方式类似liunux内核的KFIFO数据结构
*/

#include <stdlib.h>
#include "fifo_1to1.h"

