/* mouse.c, 鼠标管理程序接口 */

#include "bootpack.h"

/* 管理鼠标缓冲队列的全局指针;
 * 标识鼠标缓冲队列中数据的全局变量。*/
struct FIFO32 *mousefifo;
int mousedata0;

/* inthandler2c,
 * 鼠标中断C处理函数,读取鼠标输入到鼠标缓冲队列中。
 * 
 * 当有鼠标输入而向PIC输出中断时,
 * CPU处理PIC申请的鼠标中断时会执行IDT[0x2c]中的处
 * 理函数_asm_inthandler2c(见int.c和dsctbl.c),该处
 * 理函数会调用此处的C处理函数inthandler2c。*/
void inthandler2c(int *esp)

{
	int data;
	io_out8(PIC1_OCW2, 0x64);	
	io_out8(PIC0_OCW2, 0x62);	
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);
	return;
}

/* KEYCMD_SENDTO_MOUSE,
 * 写鼠标命令,具体的鼠标命令随后由60h端口下发;
 *
 * MOUSECMD_ENABLE,开启鼠标发送数据功能的命令。*/
#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

/* enable_mouse,
 * 初始化键盘控制器使能鼠标并设置鼠标缓冲队列和鼠标数据标识。*/
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec)
{
	mousefifo = fifo;
	mousedata0 = data0;
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0; 
	return;
}

/* mouse_decode,
 * 解析鼠标数据dat,将解析结果存于mdec指向的结构体中。
 *
 * 鼠标被操作时会向PIC输出中断信号, 中断C处理函数inthandler2c
 * 在每次鼠标中断发生时就从8042输出寄存器中读取一字节鼠标数据,
 * 鼠标以3字节为一组。
 * 
 * 当每读满3字节并完成解析时mouse_decode返回1;若鼠标数据解析失
 * 败则返回-1,若还未解析满3字节则返回0。*/
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat)
{
	if (mdec->phase == 0) {
		if (dat == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	if (mdec->phase == 1) {
		if ((dat & 0xc8) == 0x08) {
			mdec->buf[0] = dat;
			mdec->phase = 2;
		}
		return 0;
	}
	if (mdec->phase == 2) {
		mdec->buf[1] = dat;
		mdec->phase = 3;
		return 0;
	}
	if (mdec->phase == 3) {
		mdec->buf[2] = dat;
		mdec->phase = 1;
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];
		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = - mdec->y;
		return 1;
	}
	return -1; 
}
