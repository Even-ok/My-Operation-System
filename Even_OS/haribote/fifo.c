/* fifo.c, 循环队列(fifo)程序接口,用于进程(任务)通信 */

#include "bootpack.h"

/* 队列满标志 */
#define FLAGS_OVERRUN		0x0001

/* fifo32_init,
 * 初始化一个用于当前任务和task所指任务通信的队列,由fifo指向。*/
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; 
	fifo->flags = 0;
	fifo->p = 0;
	fifo->q = 0;
	fifo->task = task; 
	return;
}

/* fifo32_put,
 * 往fifo所指队列队尾端加入数据data。*/
int fifo32_put(struct FIFO32 *fifo, int data)
{
	if (fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	if (fifo->task != 0) {
		if (fifo->task->flags != 2) { 
			task_run(fifo->task, -1, 0); 
		}
	}
	return 0;
}

/* fifo32_get,
 * 从fifo所指队列队头读取数据,队列中无数据返回-1。*/
int fifo32_get(struct FIFO32 *fifo)
{
	int data;
	if (fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

/* fifo32_status,
 * 获取fifo所指队列是否完全空闲。
 * 完全空闲-返回0,否则返回队列数据个数。*/
int fifo32_status(struct FIFO32 *fifo)
{
	return fifo->size - fifo->free;
}
