/* mtask.c, 多任务管理程序接口 */

#include "bootpack.h"

/* 系统任务管理结构体全局指针变量;
 * 任务定时器全局指针变量。*/
struct TASKCTL *taskctl;
struct TIMER *task_timer;

/* task_now,
 * 获取系统任务中管理当前正运行任务的结构体基址。*/
struct TASK *task_now(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	return tl->tasks[tl->now];
}

/* task_add,
 * 在系统任务管理结构体中加入task。*/
void task_add(struct TASK *task)
{
	struct TASKLEVEL *tl = &taskctl->level[task->level];
	tl->tasks[tl->running] = task;
	tl->running++;
	task->flags = 2; 
	taskctl->runningNum++; 
	taskctl->runningTasks[taskctl->runningNum] = task; 
	return;
}

/* task_remove,
 * 将task所管理任务从其任务层中移除。*/
void task_remove(struct TASK *task)
{
	int i;
	struct TASKLEVEL *tl = &taskctl->level[task->level];

	for (i = 0; i < tl->running; i++) {
		if (tl->tasks[i] == task) {
			break;
		}
	}

	tl->running--;
	taskctl->runningNum--;
	if (i < tl->now) {
		tl->now--;
	}
	if (tl->now >= tl->running) {
		tl->now = 0;
	}
	task->flags = 1;

	for (; i < tl->running; i++) {
		tl->tasks[i] = tl->tasks[i + 1];
	}

	return;
}

/* task_switchsub,
 * 遍历并记录当前应被调度运行任务的任务层。
 * 
 * 任务层[0,MAX_TASKLEVELS)的调度优先级依次降低。
 * 若高优先级任务层中含处于可运行状态的任务时则
 * 优先调度该任务层中的任务运行。*/
void task_switchsub(void)
{
	int i;
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running > 0) {
			break; 
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}

/* task_idle,
 * 空闲任务程序代码。
 * 该程序任务代码位于任务管理的最低层,当系统
 * 中没有其他任务运行时, 该任务会被调度运行。
 *
 * task_idle 让CPU进入休眠; 当复位或中断到来
 * 时CPU才会被唤醒而继续执行下一条指令。*/
void task_idle(void)
{
	for (;;) {
		io_hlt();
	}
}

/* task_init,
 * 系统任务管理初始化,包括
 * 初始化GDT和IDT;
 * 初始化 管理系统所有任务运行的 结构体;同时为当
 * 前程序分配任务管理结构体;设置闲置任务到优先级
 * 最低任务层中,以在系统无其他任务运行时被调度运行。*/
struct TASK *task_init(struct MEMMAN *memman)
{
	int i;
	struct TASK *task, *idle;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl->runningNum = 1; 
	taskctl->sleepTasksNum = 0;
	taskctl->stoppedTasksNum = 0;

	taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof (struct TASKCTL));
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = 0;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		taskctl->tasks0[i].tss.ldtr = (TASK_GDT0 + MAX_TASKS + i) * 8;
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
		set_segmdesc(gdt + TASK_GDT0 + MAX_TASKS + i, 15, (int) taskctl->tasks0[i].ldt, AR_LDT);
	}
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running = 0;
		taskctl->level[i].now = 0;
	}

	task = task_alloc();
	task->flags = 2;	
	task->priority = 2; 
	task->level = 0;
	task_add(task);
	task_switchsub();
	load_tr(task->sel);
	task_timer = timer_alloc();
	timer_settime(task_timer, task->priority);

	idle = task_alloc();
	idle->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024;
	idle->tss.eip = (int) &task_idle;
	idle->tss.es = 1 * 8;
	idle->tss.cs = 2 * 8;
	idle->tss.ss = 1 * 8;
	idle->tss.ds = 1 * 8;
	idle->tss.fs = 1 * 8;
	idle->tss.gs = 1 * 8;
	task_run(idle, MAX_TASKLEVELS - 1, 1);
	idle->TaskName = "idle";

	return task;
}

/* task_alloc,
 * 在系统任务管理结构体中遍历一个任务管理结构体并初始化,
 * 若成功返回该任务管理结构体首地址, 失败则返回0。*/
struct TASK *task_alloc(void)
{
	int i;
	struct TASK *task;
	for (i = 0; i < MAX_TASKS; i++) {
		if (taskctl->tasks0[i].flags == 0) {
			task = &taskctl->tasks0[i];
			task->flags = 1; 
			task->tss.eflags = 0x00000202; /* IF = 1; */
			task->tss.eax = 0; 
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.iomap = 0x40000000;
			task->tss.ss0 = 0;
			return task;
		}
	}
	return 0; 
}

/* task_run,
 * 将task所管理程序任务的运行时间片设置为priority,然后将该任
 * 务置在任务层level中以待被调度运行。task_run会置任务层调度
 * 标志,这会让任务调度函数task_switch调用task_switchsub调度
 * 含可运行任务的优先级最高的任务层中的任务运行。*/
void task_run(struct TASK *task, int level, int priority)
{
	taskctl->runningNum++; 
	taskctl->runningTasks[taskctl->runningNum] = task;
	if (level < 0) {
		level = task->level; 
	}
	if (priority > 0) {
		task->priority = priority;
	}

	if (task->flags == 2 && task->level != level) {
		task_remove(task); 
	}
	if (task->flags != 2) {
		task->level = level;
		task_add(task);
		taskctl->runningNum--;//新增
	}

	taskctl->lv_change = 1;
	return;
}

/* task_sleep,
 * 将task所指任务休眠即将其从其所在任务层中移除, 若task所指
 * 任务当前正在运行则在本任务休眠后调度优先级最高的任务运行。*/
void task_sleep(struct TASK *task)
{
	struct TASK *now_task;
	if (task->flags == 2) {
		now_task = task_now();
		task_remove(task); 
		if (task == now_task) {
			task_switchsub();
			now_task = task_now(); 
			farjmp(0, now_task->sel);
		}
	}
	return;
}

/* task_switch,任务切换。*/
void task_switch(void)
{
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = tl->tasks[tl->now];
	tl->now++;
	if (tl->now == tl->running) {
		tl->now = 0;
	}
	if (taskctl->lv_change != 0) {
		task_switchsub();
		tl = &taskctl->level[taskctl->now_lv];
	}
	new_task = tl->tasks[tl->now];
	timer_settime(task_timer, new_task->priority);
	if (new_task != now_task) {
		farjmp(0, new_task->sel);
	}
	return;
}
