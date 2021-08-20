/* timer.c, 定时器程序接口 */

#include "bootpack.h"

/* 定时器控制寄存器8254控制器端口地址和通道0计数器端口地址 */
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

/* 管理定时器的全局变量 */
struct TIMERCTL timerctl;

/* 标识管理定时器结构体已分配状态和正在使用状态 */
#define TIMER_FLAGS_ALLOC		1
#define TIMER_FLAGS_USING		2

/* init_pit,
 * 初始化定时器控制器8254,配置其发起中断请求的超时时间;
 * 并启动一个"永不超时的定时器"作为哨兵定时器。
 *
 * 永不超时定时器共包含0xffffffff个计数,haribote定时器
 * 处理函数每约10ms发生一次计数,则0xffffffff约为497年。*/
void init_pit(void)
{
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = 0;
	}
	t = timer_alloc();
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next = 0;
	timerctl.t0 = t;
	timerctl.next = 0xffffffff;
	return;
}

/* timer_alloc,
 * 分配一个空闲的定时器管理结构体用作定时器管理。
 * 成功则返回该空闲元素地址; 失败返回0. */
struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timers0[i].flags == 0) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			timerctl.timers0[i].flags2 = 0;
			return &timerctl.timers0[i];
		}
	}
	return 0;
}

/* timer_free,
 * 释放timer指向的定时器结构体,将其状态设置为未使用状态。*/
void timer_free(struct TIMER *timer)
{
	timer->flags = 0;
	return;
}

/* timer_init,
 * 设置timer所指定时器的循环队列和超时数据。
 * 
 * 定时器超时时,将数据data发往fifo所指缓冲队列中,其他程序(任务)就可从该
 * 队列里读到data数据,从而可根据该data做出后续动作。*/
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
	return;
}

/* timer_settime,
 * 设置timer所指定时器的超时值为timeout(单位10ms),
 * 并将timer插入到以超时值升序排列的定时器链表中。*/
void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t, *s;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	e = io_load_eflags();
	io_cli();
	t = timerctl.t0;
	if (timer->timeout <= t->timeout) {
		timerctl.t0 = timer;
		timer->next = t;
		timerctl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
			s->next = timer; 
			timer->next = t;
			io_store_eflags(e);
			return;
		}
	}
}

/* inthandler20,
 * 定时器中断C处理函数。
 * 定时器中断约每10ms会向8259A IRQ0发起一次中断,见 init_pit()。
 * 定时器中断发生时会调用注册在IDT[20h]中的 _asm_inthandler20,
 * 该程序会调用此处的定时器中断C处理函数 inthandler20 以完成中
 * 断处理,即该函数约每10ms该函数就会被调用一次。*/
void inthandler20(int *esp)
{
	struct TIMER *timer;
	char ts = 0;
	io_out8(PIC0_OCW2, 0x60);
	timerctl.count++;
	if (timerctl.next > timerctl.count) {
		return;
	}
	timer = timerctl.t0;
	for (;;) {
		if (timer->timeout > timerctl.count) {
			break;
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		if (timer != task_timer) {
			fifo32_put(timer->fifo, timer->data);
		} else {
			ts = 1;
		}
		timer = timer->next; 
	}
	timerctl.t0 = timer;
	timerctl.next = timer->timeout;
	if (ts != 0) {
		task_switch();
	}
	return;
}

/* timer_cancel,
 * 取消timer所指定时器,成功返回1,失败返回0。*/
int timer_cancel(struct TIMER *timer)
{
	int e;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();
	if (timer->flags == TIMER_FLAGS_USING) {
		if (timer == timerctl.t0) {
			t = timer->next;
			timerctl.t0 = t;
			timerctl.next = t->timeout;
		} else {
			t = timerctl.t0;
			for (;;) {
				if (t->next == timer) {
					break;
				}
				t = t->next;
			}
			t->next = timer->next;
		}
		timer->flags = TIMER_FLAGS_ALLOC;
		io_store_eflags(e);
		return 1;
	}
	io_store_eflags(e);
	return 0; 
}

/* timer_cancelall,
 * 取消所关联队列首地址为fifo的所有定时器。
 * 经该函数取消的定时器结构体的状态恢复未使用状态。*/
void timer_cancelall(struct FIFO32 *fifo)
{
	int e, i;
	struct TIMER *t;
	e = io_load_eflags();
	io_cli();
	for (i = 0; i < MAX_TIMER; i++) {
		t = &timerctl.timers0[i];
		if (t->flags != 0 && t->flags2 != 0 && t->fifo == fifo) {
			timer_cancel(t);
			timer_free(t);
		}
	}
	io_store_eflags(e);
	return;
}
