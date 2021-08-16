//内存管理，采用固定分区分配

#include "bootpack.h"
#include <stdio.h>
#define EFLAGS_AC_BIT		0x00040000
#define CR0_CACHE_DISABLE	0x60000000

/*
	返回当前剩余空间大小
*/
unsigned int memtest(unsigned int start, unsigned int end)//检查内存
{
	//先检查CPU是不是在486以上，如果是，就将缓存设为OFF。
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	//确认CPU是386或486，取出eflags寄存器中的值将AC位置1
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; // AC-bit = 1
	io_store_eflags(eflg);
	eflg = io_load_eflags();	//置1后存入寄存器中,再取出
	if ((eflg & EFLAGS_AC_BIT) != 0) { //当是386时，即使设定AC=1，AC的值还会自动回到0
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT; // AC-bit = 0
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; //禁止486以上的CPU使用缓存
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);//内存检查处理

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; //检查结束后允许486缓存	//~取反，~AC_bit=0Xfffbffff
		store_cr0(cr0);
	}

	return i;
}

void memman_init(struct MEMMAN *man)//初始化，设定为空
{
	man->frees = 0;			//可用信息数目
	man->maxfrees = 0;		//用于观察可用情况：frees的最大值
	man->lostsize = 0;		//释放失败的内存的大小总和
	man->losts = 0;			//释放失败次数
	return;
}

/*	得到空余内存大小的合计	*/
unsigned int memman_total(struct MEMMAN *man)
{
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;//累计
	}
	return t;
}

//分配（产生很多碎片）
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
{
	unsigned int i, a;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			//找到可以满足分配的的内存
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			//free[i]为0，即此块没有剩余空闲空间
			if (man->free[i].size == 0) {
				//减掉一条可用信息
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; //代入结构体
				}
			}
			return a;
		}
	}
	return 0; //没有可用空间
}

//释放内存
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i, j;
	//为便于归纳内存，将free[]按照addr的顺序排列
	//决定放置位置
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	//根据释放内存前后的内存的可用情况，判断该块内存是否需要与其他合并
	//free[i-1].addr < addr < free[i].addr
	if (i > 0) {
		//当前面有可用内存时
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			//可以与前面的可用内存归在一起，合成一个
			man->free[i - 1].size += size;
			if (i < man->frees) {
				//后面的也是可用内存
				if (addr + size == man->free[i].addr) {
					//把他也与后面的可用内存合并在一起
					man->free[i - 1].size += man->free[i].size;
					// man->free[i]删除
					// free[i]变成0后归纳到前面去
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1];  //结构体赋值
					}
				}
			}
			return 0; //释放内存完成
		}
	}
	//不能与前面的可用空间归纳到一起
	if (i < man->frees) {
		//后面的是可用空间
		if (addr + size == man->free[i].addr) {
			//与后面的可用空间归纳到一起
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; //释放内存完成
		}
	}
	//前后都没用可用内存
	if (man->frees < MEMMAN_FREES) {
		// free[i]之后的，向后移动，腾出空间
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; //更新空闲内存最大值
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; //释放内存完成
	}
	//不能往后移动
	man->losts++;
	man->lostsize += size;	//累计为释放失败的内存的大小
	return -1; //释放内存失败
}

//以1字节为单位进行内存管理
//分配
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size)//0X1000字节的大小是4KB
{//向上舍入
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;	
	a = memman_alloc(man, size);
	return a;
}

//释放
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}

//显示内存
unsigned int memman_show(struct CONSOLE *cons, struct MEMMAN *man){
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}


int memman_free_fdata(struct MEMMAN *memman, unsigned int fdata_addr){
	struct MYFILEDATA *temp_fdata, *prev_temp_fdata;
	int i;
	char s[50];
	unsigned int fdata_size;

	fdata_size = sizeof(struct MYFILEDATA);
	temp_fdata = (struct MYFILEDATA *)fdata_addr;

	do{ 
		prev_temp_fdata = temp_fdata;
		i = memman_free(memman, (unsigned int)temp_fdata, fdata_size);
		if(i == -1){
			sprintf(s, "return memman_free() was failed.\n");
			debug_print(s);
			return 0;
		}else{
			sprintf(s, "%d length was released from the addr 0x%08x\n", fdata_size, temp_fdata);
			debug_print(s);
		}
		temp_fdata = temp_fdata->head.next_fdata;
	}while(prev_temp_fdata->head.next_fdata != 0);

	return 1;
}
