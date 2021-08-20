/* int.c, 可编程中断控制器(PIC)8259A初始化程序接口 */

#include "bootpack.h"
#include <stdio.h>

/* init_pic,
 * 初始化配置可编程中断控制器(PIC),
 * 为PIC分配中断向量[0x20, 0x2f],暂屏蔽所有中断。*/
void init_pic(void)
{
	io_out8(PIC0_IMR,  0xff  );
	io_out8(PIC1_IMR,  0xff  );

	io_out8(PIC0_ICW1, 0x11  ); 
	io_out8(PIC0_ICW2, 0x20  ); 
	io_out8(PIC0_ICW3, 1 << 2); 
	io_out8(PIC0_ICW4, 0x01  ); 

	io_out8(PIC1_ICW1, 0x11  ); 
	io_out8(PIC1_ICW2, 0x28  );
	io_out8(PIC1_ICW3, 2     ); 
	io_out8(PIC1_ICW4, 0x01  ); 

	io_out8(PIC0_IMR,  0xfb  ); 
	io_out8(PIC1_IMR,  0xff  );

	return;
}
