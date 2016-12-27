# LockFreeDS
实现常用的lock-free数据结构，下面对各个源文件做一个简单的说明.

队列实现、测试和性能测试相关文件如下：

    fifo_1to1.c fifo_1to1.h  //类似于Linux内核KFIFO的Lock-Free实现，用于单生产者和单消费者时非常高效
    fifo_1to1_test.c         //对fifo_1to1实现正确性测试
    fifo_1to1_benchmark.c    //对使用fifo_1to1与通常使用mutex情况进行性能对比
