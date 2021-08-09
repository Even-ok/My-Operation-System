/* --------------------------------
    B Y : S T O N
    HELO OS �����ļ�
        ver. 1.0
         DATE : 2019-1-19  
----------------------------------- */
/* copyright(C) 2019 PZK . */


#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED      0xed
#define START_MEMORY	0x00600000
#define END_MEMORY		0xbfffffff
#define CONSOLE_W       287
#define CONSOLE_H       562
#define CONSOLE_TEXT_W  280
#define CONSOLE_TEXT_H  556

struct S mutex;
struct S wrt;
int readcount;
int share_bupt;

void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);
void close_console(struct SHEET *sht);
void close_constask(struct TASK *task);
void task_b_main(struct SHEET *sht_back);

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    struct SHTCTL *shtctl;
    char s[40];
    struct FIFO32 fifo, keycmd;
    int fifobuf[128], keycmd_buf[32];
    struct TIMER *timer, *timer2, *timer3 , *timer4,*second;  //定时器
    int mx, my, i, new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0,cursor_x, cursor_c, task_b_esp;
    int color=0,z=4;
	int maxmx=0,maxmy=0,minmx=binfo->scrnx ,minmy= binfo->scrny; //800 600
	int a,b,c,d;
	int linex1,linex2,liney1,liney2;
	int flagshu=0,flagmin=0,flagline=0,flagj=0,flagclear=0,flagsan=0;
	int wrong=0,flagwrong=0;
	int wx,wy;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    unsigned char *buf_back, buf_mouse[256],*buf_qidong,*buf_boot,*buf_dong,*buf_paint,*buf_zihao,*buf_color;
    struct SHEET *sht_back, *sht_mouse,*sht_qidong,*sht_boot,*sht_dong,*sht_paint,*sht_zihao,*sht_color;
    struct TASK *task_a, *task_b[3],*task;
    static char keytable0[0x80] = {
        0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08, 0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a, 0, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
    };
    static char keytable1[0x80] = {
        0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x08, 0,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0x0a, 0, 'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
        '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
    };
    static char keytable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
	};
    int kdx1[60] = { 400, 410, 421, 431, 441, 450, 459, 467, 474, 481, 487, 491, 495, 498, 499,
					500, 499, 498, 495, 491, 487, 481, 474, 467, 459, 450, 441, 431, 421, 410,
					400, 390, 379, 369, 359, 350, 341, 333, 326, 319, 313, 309, 305, 302, 301,
					300, 301, 302, 305, 309, 313, 319, 326, 333, 341, 350, 359, 369, 379, 390};

	int kdy1[60] = { 200, 201, 202, 205, 209, 213, 219, 226, 233, 241, 250, 259, 269, 279, 290,
					300, 310, 321, 331, 341, 350, 359, 367, 374, 381, 387, 391, 395, 398, 399,
					400, 399, 398, 395, 391, 387, 381, 374, 367, 359, 350, 341, 331, 321, 310,
					300, 290, 279, 269, 259, 250, 241, 233, 226, 219, 213, 209, 205, 202, 201};

    int kx1[60]  = { 400, 408, 416, 423, 431, 438, 444, 450, 456, 461, 465, 469, 471, 473, 474,
	                475, 474, 473, 471, 469, 465, 461, 456, 450, 444, 438, 431, 423, 416, 408,
					400, 392, 384, 377, 369, 362, 356, 350, 344, 339, 335, 331, 329, 327, 326,
					325, 326, 327, 329, 331, 335, 339, 344, 350, 356, 362, 369, 377, 384, 392};

    int ky1[60]  = { 225, 226, 227, 229, 231, 235, 239, 244, 250, 256, 262, 269, 277, 284, 292,
	                300, 308, 316, 323, 331, 338, 344, 350, 356, 361, 365, 369, 371, 373, 374,
					375, 374, 373, 371, 369, 365, 361, 356, 350, 344, 338, 331, 323, 316, 308,
					300, 292, 284, 277, 269, 262, 256, 250, 244, 239, 235, 231, 229, 227, 226};
    int key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;
    int j, x, y, mmx = -1, mmy = -1, mmx2 = 0;
    struct SHEET *sht = 0, *key_win, *sht2;
    int *fat;
    unsigned char *nihongo;
    struct FILEINFO *finfo;
    extern char hankaku[4096];

    init_gdtidt();
    init_pic();
    io_sti(); /* IDT/PIC�̏��������I������̂�CPU�̊��荞�݋֎~������ */
    fifo32_init(&fifo, 128, fifobuf, 0);
    *((int *) 0x0fec) = (int) &fifo;
    init_pit();
    init_keyboard(&fifo, 256);
    enable_mouse(&fifo, 512, &mdec);
    io_out8(PIC0_IMR, 0xf8); /* PIT��PIC1�ƃL�[�{�[�h������(11111000) */
    io_out8(PIC1_IMR, 0xef); /* �}�E�X������(11101111) */
    fifo32_init(&keycmd, 32, keycmd_buf, 0);

    timer = timer_alloc();
	timer_init(timer, &fifo, 10);
	timer_settime(timer, 10);
	timer2 = timer_alloc();
	timer_init(timer2, &fifo, 15);
	timer_settime(timer2, 5);
	timer3 = timer_alloc();
	//timer_init(timer3, &fifo, 1);
	//timer_settime(timer3, 50);
	second = timer_alloc();

    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_palette();
    shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
    task_a = task_init(memman);
    fifo.task = task_a;
    task_run(task_a, 1, 2);
    *((int *) 0x0fe4) = (int) shtctl;
    task_a->langmode = 0;

    /* sht_back */
    sht_back  = sheet_alloc(shtctl);
    buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
    sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* �����F�Ȃ� */
    init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/* log window */
	key_win = open_console(shtctl, memtotal);

    /* sht_mouse */
    sht_mouse = sheet_alloc(shtctl);
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    init_mouse_cursor8(buf_mouse, 99);
    mx = (binfo->scrnx - 16) / 2; /* ��ʒ����ɂȂ�悤�ɍ��W�v�Z */
    my = (binfo->scrny - 28 - 16) / 2;

    //其他页面

    sht_qidong = sheet_alloc(shtctl);
	sht_boot = sheet_alloc(shtctl);
	sht_paint = sheet_alloc(shtctl);
	sht_zihao = sheet_alloc(shtctl);
	sht_color = sheet_alloc(shtctl);

    buf_qidong = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_boot = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_paint = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	buf_zihao = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	buf_color = (unsigned char *) memman_alloc_4k(memman, 160 * 52);

    sheet_setbuf(sht_qidong, buf_qidong, binfo->scrnx,binfo->scrny, -1);
	sheet_setbuf(sht_boot, buf_boot, binfo->scrnx,binfo->scrny, -1);
	sheet_setbuf(sht_paint, buf_paint, binfo->scrnx,binfo->scrny, -1);
	sheet_setbuf(sht_zihao, buf_zihao, 160,52, -1);
	sheet_setbuf(sht_color, buf_color, 160,52, -1);
    
    //make_window8(buf_paint, 800, 600, "paint"，0);
	make_window7(buf_zihao,160,52,"zihao");
	make_window6(buf_color,160,52,"color");
    make_window8(buf_paint, binfo->scrnx, binfo->scrny,  "paint", 0);
    bootpage(buf_boot, binfo->scrnx, binfo->scrny);//密码窗
    //qidong
	boxfill8(buf_qidong,binfo->scrnx,7,0,0,binfo->scrnx,binfo->scrny);
	sheet_refresh(sht_qidong,0,0,binfo->scrnx,binfo->scrny);

    sheet_slide(sht_back,  0,  0);
    mx = (binfo->scrnx - 16) / 2; /* ��ʒ����ɂȂ�悤�ɍ��W�v�Z */
	my = (binfo->scrny - 28 - 16) / 2;
    sheet_slide(key_win,   32, 4);
    sheet_slide(sht_mouse, mx, my);
    sheet_slide(sht_qidong, 0, 0);
	sheet_slide(sht_boot, 0, 0);
	sheet_slide(sht_paint, 0, 0);
	sheet_slide(sht_zihao, 10, 50);
	sheet_slide(sht_color, 10, 50);

    // sheet_updown(sht_back,  0);
    // sheet_updown(key_win,   1);
    // sheet_updown(sht_mouse, 2);
    sheet_updown(sht_qidong, 4);///////
	sheet_updown(sht_back,  0);
	sheet_updown(key_win,   -1);
	sheet_updown(sht_mouse, 1);//1
	sheet_updown(sht_boot, 2);//2
	sheet_updown(sht_paint, -1);
	sheet_updown(sht_zihao, -1);
	sheet_updown(sht_color, -1);
    keywin_on(key_win);


    make_textbox8(sht_zihao, 8, 28, 144, 16, COL8_FFFFFF);//描绘键盘输入背景（这个的放置位置要改一下）
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;

    /* �ŏ��ɃL�[�{�[�h��ԂƂ̐H���Ⴂ���Ȃ��悤�ɁA�ݒ肵�Ă������Ƃɂ��� */
    fifo32_put(&keycmd, KEYCMD_LED);
    fifo32_put(&keycmd, key_leds);

    /* nihongo.fnt�̓ǂݍ��� */
    //nihongo = (unsigned char *) memman_alloc_4k(memman, 16 * 256 + 32 * 94 * 47);
    nihongo = (unsigned char *) memman_alloc_4k(memman, 0x5d5d * 32);
    fat = (int *) memman_alloc_4k(memman, 4 * 2880);
    file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
    finfo = file_search("HZK16.fnt", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    if (finfo != 0) {
        file_loadfile(finfo->clustno, finfo->size, nihongo, fat, (char *) (ADR_DISKIMG + 0x003e00));
    } else {
        for (i = 0; i < 16 * 256; i++) {
            nihongo[i] = hankaku[i]; /* �t�H���g���Ȃ������̂Ŕ��p�������R�s�[ */
    	}
        for (i = 16 * 256; i < 16 * 256 + 32 * 94 * 47; i++) {
            nihongo[i] = 0xff; /* �t�H���g���Ȃ������̂őS�p������0xff�Ŗ��ߐs���� */
    	}
    }

    *((int *) 0x0fe8) = (int) nihongo;
    memman_free_4k(memman, (int) fat, 4 * 2880);

//int b=2;
	int qidong=0;//,flagq=0;
	while(qidong==0){  //开机动画
	//flagq=1;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
		if(i==15){
			timer_init(timer2, &fifo, 16);
			boxfill8(buf_qidong,binfo->scrnx,8,0,0,binfo->scrnx,binfo->scrny);
			line(buf_qidong,binfo->scrnx,7,470,300,500,300,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
		}
		if(i==16){
			timer_init(timer2, &fifo, 17);
			line(buf_qidong,binfo->scrnx,7,461,265,487,250,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
		}
		if(i==17){
			timer_init(timer2, &fifo, 18);
			line(buf_qidong,binfo->scrnx,7,435,239,450,213,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
		}
		if(i==18){
			timer_init(timer2, &fifo, 19);
			line(buf_qidong,binfo->scrnx,7,400,200,400,230,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
		}
		if(i==19){
			timer_init(timer2, &fifo, 20);
			line(buf_qidong,binfo->scrnx,7,350,213,365,239,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
		}
		if(i==20){
			timer_init(timer2, &fifo, 21);
			line(buf_qidong,binfo->scrnx,7,313,250,339,265,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
		}
		if(i==21){
			timer_init(timer2, &fifo, 22);
			line(buf_qidong,binfo->scrnx,7,300,300,330,300,3);
			line(buf_qidong,binfo->scrnx,8,470,300,500,300,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 5);
			//qidong=1;
			//i=0;
		}
		if(i==22){
			timer_init(timer2, &fifo, 23);
			line(buf_qidong,binfo->scrnx,7,313,350,339,335,3);
			line(buf_qidong,binfo->scrnx,8,461,265,487,250,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 5);
			//qidong=1;
		}
		if(i==23){
			timer_init(timer2, &fifo, 24);
			line(buf_qidong,binfo->scrnx,7,350,387,365,361,3);
			line(buf_qidong,binfo->scrnx,8,435,239,450,213,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 5);
			//qidong=1;
		}
		if(i==24)
		{
			timer_init(timer2, &fifo, 25);
			line(buf_qidong,binfo->scrnx,7,400,400,400,370,3);
			line(buf_qidong,binfo->scrnx,8,400,200,400,230,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 5);
		}
		if(i==25)
		{
			timer_init(timer2, &fifo, 26);
			line(buf_qidong,binfo->scrnx,7,435,361,450,387,3);
			line(buf_qidong,binfo->scrnx,8,350,213,365,239,3);
			/*
			int i;
			for(i=1;i<=60;i=i+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kdx1[i]-4, kdy1[i]-4, kdx1[i]+4, kdy1[i]+4);
			}
            int j;
			for(j=0;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 1, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}*/
			//boxfill8(buf_qidong,binfo->scrnx,7,0,0,280,600);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 5);
		}
		if(i==26)
		{
			timer_init(timer2, &fifo, 27);
			line(buf_qidong,binfo->scrnx,7,461,335,487,350,3);
			line(buf_qidong,binfo->scrnx,8,313,250,339,265,3);
			/*
			int j;
			for(j=0;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 1, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}
			//boxfill8(buf_qidong,binfo->scrnx,7,0,0,280,600);

			boxfilly(buf_qidong, binfo->scrnx, 3, 325, 225, 475, 375);
			boxfilly(buf_qidong, binfo->scrnx, 1, 348, 248, 452, 352);
			boxfilly(buf_qidong, binfo->scrnx, 3, 385, 285, 415, 315);
*/
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 5);
		}
		if(i==27)
		{
			timer_init(timer2, &fifo, 28);
			line(buf_qidong,binfo->scrnx,7,470,300,500,300,3);
			line(buf_qidong,binfo->scrnx,8,300,300,330,300,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 2);
			//qidong=1;
		}
		if(i==28)
		{
			timer_init(timer2, &fifo, 29);
			line(buf_qidong,binfo->scrnx,7,461,265,487,250,3);
			line(buf_qidong,binfo->scrnx,8,313,350,339,335,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 2);
			//qidong=1;
		}
		if(i==29)
		{
			timer_init(timer2, &fifo, 30);
			line(buf_qidong,binfo->scrnx,7,435,239,450,213,3);
			line(buf_qidong,binfo->scrnx,8,350,387,365,361,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
			//qidong=1;
		}
		if(i==30)
		{
			timer_init(timer2, &fifo, 31);
			line(buf_qidong,binfo->scrnx,7,400,200,400,230,3);
			line(buf_qidong,binfo->scrnx,8,400,400,400,370,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 2);
			//qidong=1;
		}
		if(i==31)
		{
			timer_init(timer2, &fifo, 32);
			line(buf_qidong,binfo->scrnx,7,350,213,365,239,3);
			line(buf_qidong,binfo->scrnx,8,435,361,450,387,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
		}
		if(i==32)
		{
			timer_init(timer2, &fifo, 33);
			line(buf_qidong,binfo->scrnx,7,313,250,339,265,3);
			line(buf_qidong,binfo->scrnx,8,461,335,487,350,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 2);
			//qidong=1;
		}
		if(i==33)
		{
			timer_init(timer2, &fifo, 34);
			line(buf_qidong,binfo->scrnx,7,300,300,330,300,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 2);
			//qidong=1;
		}
		if(i==34)
		{
			timer_init(timer2, &fifo, 35);
			line(buf_qidong,binfo->scrnx,7,313,350,339,335,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 2);
			//qidong=1;
		}
		if(i==35)
		{
			timer_init(timer2, &fifo, 36);
			line(buf_qidong,binfo->scrnx,7,350,387,365,361,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 2);
			//qidong=1;
		}
		if(i==36)
		{
            timer_init(timer2, &fifo, 37);
			line(buf_qidong,binfo->scrnx,7,400,400,400,370,3);

			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 3);
			//qidong=1;
		}
		if(i==37)
		{
            timer_init(timer2, &fifo, 38);
			line(buf_qidong,binfo->scrnx,7,435,361,450,387,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 20);
			//qidong=1;
		}
		if(i==38)
		{
            timer_init(timer2, &fifo, 39);
			line(buf_qidong,binfo->scrnx,7,461,335,487,350,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 10);
			//qidong=1;
		}
		if(i==39)
		{
            timer_init(timer2, &fifo, 40);
			line(buf_qidong,binfo->scrnx,8,400,200,400,230,3);
			line(buf_qidong,binfo->scrnx,8,400,400,400,370,3);
			line(buf_qidong,binfo->scrnx,8,300,300,330,300,3);
			line(buf_qidong,binfo->scrnx,8,470,300,500,300,3);
			line(buf_qidong,binfo->scrnx,8,435,239,450,213,3);
			line(buf_qidong,binfo->scrnx,8,461,265,487,250,3);
			line(buf_qidong,binfo->scrnx,8,461,335,487,350,3);
			line(buf_qidong,binfo->scrnx,8,435,361,450,387,3);
			line(buf_qidong,binfo->scrnx,8,313,350,339,335,3);
			line(buf_qidong,binfo->scrnx,8,350,387,365,361,3);
			line(buf_qidong,binfo->scrnx,8,313,250,339,265,3);
			line(buf_qidong,binfo->scrnx,8,350,213,365,239,3);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 20);
			//qidong=1;
		}
		if(i==40)
		{
            timer_init(timer2, &fifo, 41);
			boxfill8(buf_qidong,binfo->scrnx,15,0,0,binfo->scrnx,binfo->scrny);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 100);
			//qidong=1;
		}
		if(i==41)
		{
            timer_init(timer2, &fifo, 42);
			
			boxfilly(buf_qidong, binfo->scrnx, 16, 350, 250, 450, 350);
			boxfilly(buf_qidong, binfo->scrnx, 7, 360, 260, 440, 340);
			boxfilly(buf_qidong, binfo->scrnx, 16, 385, 285, 415, 315);
			
			int i;
			for(i=3;i<=60;i=i+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kx1[i]-2, ky1[i]-2, kx1[i]+2, ky1[i]+2);
			}
			line(buf_qidong,binfo->scrnx,7,0,420,50,420,2);
			line(buf_qidong,binfo->scrnx,7,750,420,800,420,2);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 100);
			//qidong=1;
		}
		if(i==42)
		{
            timer_init(timer2, &fifo, 43);


			boxfilly(buf_qidong, binfo->scrnx, 16, 340, 240, 460, 360);
			boxfilly(buf_qidong, binfo->scrnx, 7, 350, 250, 450, 350);
			boxfilly(buf_qidong, binfo->scrnx, 16, 375, 275, 425, 325);
			int j;
			for(j=4;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}

			line(buf_qidong,binfo->scrnx,7,50,420,150,420,2);
			line(buf_qidong,binfo->scrnx,7,650,420,750,420,2);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 100);
			//qidong=1;
		}
		if(i==43)
		{
            timer_init(timer2, &fifo, 44);
			line(buf_qidong,binfo->scrnx,7,50,420,150,420,2);
			line(buf_qidong,binfo->scrnx,7,650,420,750,420,2);

			int j;
			for(j=4;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 15, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}
			int i;
			for(i=3;i<=60;i=i+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kdx1[i]-4, kdy1[i]-4, kdx1[i]+4, kdy1[i]+4);
			}

			//boxfill8(buf_qidong,binfo->scrnx,0,0,0,800,600);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 100);
			//qidong=1;
		}
		if(i==44)
		{
            timer_init(timer2, &fifo, 45);
			line(buf_qidong,binfo->scrnx,7,150,420,350,420,2);
			line(buf_qidong,binfo->scrnx,7,450,420,650,420,2);
			
			int j;
			for(j=3;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 15, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}
			int i;
			for(i=2;i<=60;i=i+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kdx1[i]-4, kdy1[i]-4, kdx1[i]+4, kdy1[i]+4);
			}
			
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 100);
			//qidong=1;
		}
		if(i==45)
		{
            timer_init(timer2, &fifo, 46);
			int j;
			for(j=2;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 15, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}
			int i;
			for(i=1;i<=60;i=i+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kdx1[i]-4, kdy1[i]-4, kdx1[i]+4, kdy1[i]+4);
			}

			line(buf_qidong,binfo->scrnx,7,350,420,400,420,2);
			line(buf_qidong,binfo->scrnx,7,400,420,450,420,2);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 100);
			//qidong=1;
		}
		if(i==46)
		{
            timer_init(timer2, &fifo, 47);
			
			int j;
			for(j=1;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 15, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}
			int i;
			for(i=0;i<=60;i=i+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kdx1[i]-4, kdy1[i]-4, kdx1[i]+4, kdy1[i]+4);
			}

			line(buf_qidong,binfo->scrnx,7,350,420,400,420,2);
			line(buf_qidong,binfo->scrnx,7,400,420,450,420,2);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 40);
			//qidong=1;
		}
		if(i==47)
		{
            timer_init(timer2, &fifo, 48);
			int j;
			for(j=0;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 15, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}
			int i;
			for(i=4;i<=60;i=i+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kdx1[i]-4, kdy1[i]-4, kdx1[i]+4, kdy1[i]+4);
			}

			line(buf_qidong,binfo->scrnx,7,350,420,400,420,2);
			line(buf_qidong,binfo->scrnx,7,400,420,450,420,2);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 40);
			//qidong=1;
		}
		if(i==48)
		{
            timer_init(timer2, &fifo, 49);
			int j;
			for(j=4;j<=60;j=j+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 15, kdx1[j]-4, kdy1[j]-4, kdx1[j]+4, kdy1[j]+4);
			}
			int i;
			for(i=3;i<=60;i=i+5)
			{
				boxfilly(buf_qidong, binfo->scrnx, 7, kdx1[i]-4, kdy1[i]-4, kdx1[i]+4, kdy1[i]+4);
			}
			line(buf_qidong,binfo->scrnx,7,350,420,400,420,2);
			line(buf_qidong,binfo->scrnx,7,400,420,450,420,2);
			boxfill8(buf_qidong,binfo->scrnx, 0, 0, 0,binfo->scrnx,binfo->scrny);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 40);
			//qidong=1;
		}
		if(i==49)
		{
            timer_init(timer2, &fifo, 50);
			line(buf_qidong,binfo->scrnx,7,350,420,400,420,2);
			line(buf_qidong,binfo->scrnx,7,400,420,450,420,2);
			sheet_refresh(sht_qidong, 0, 0, binfo->scrnx, binfo->scrny);
			timer_settime(timer2, 40);
			qidong=1;
		}
	}
}

	sheet_updown(sht_qidong,-1);
	//sheet_updown(sht_boot,2);

    // sheet_updown(sht_back,  0);
    // sheet_updown(key_win,   1); 
    // sheet_updown(sht_mouse, 2);

static char mima[6] = {'1','2','3','4','5','6'};//初始密码
	int cursor_pin = 327; //密码输入处的光标
	int flag_mima[10] = {0};
	int cursor_cpin = 0;
	int mima_count = 0;
	flag_mima[6] = 1; //如果密码按下的超过了6位，这一位就会变为0.

	int p;

	timer_init(timer3, &fifo, 1);
	timer_settime(timer3, 50);

	/* 判断是否双击的变量 */
	int songkai = 0, flagL = 0, time_count=0;

	while(sht_boot->height >= 0) {
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_stihlt();
		} else {
			i = fifo32_get(&fifo);
			io_sti();	
			cursor_pin = cin_pin(sht_boot, i, keytable, cursor_pin, cursor_cpin, timer3,&fifo,mima, &mima_count, flag_mima);				
				if (i <= 1) 
    			{ /* 光标颜色 */
					if (i != 0) {
						timer_init(timer3, &fifo, 0); 
						cursor_cpin = 0;
					} else {
						timer_init(timer3, &fifo, 1); 
						cursor_cpin = 7;
					}
        		timer_settime(timer3, 50);
				boxfill8(sht_boot->buf, sht_boot->bxsize, cursor_cpin, cursor_pin-10, 403, cursor_pin -10 + 1, 419);
				sheet_refresh(sht_boot, cursor_pin-10, 403, cursor_pin-10 + 1, 419);
				}
				
				if (if_right(flag_mima)) //密码正确就进入桌面
				{
					sheet_updown(sht_boot,  -1);
					//sheet_updown(sht_dong,2);
				} 
				else if (flag_mima[7] == 1) //密码错误但是已经按下了enter键
				{
					wrong++;
					if(wrong==3)
					{
						sheet_updown(key_win,3);
						//timer_init(timer4, &fifo, 50);
						putfonts8_asc_sht(key_win, 250, 300, COL8_000000, 7,  "After 3 seconds can you try again!", 34);
						//timer_settime(timer4, 300);
						//sheet_updown(key_win,-1);
						int i=0;
						while(i<=500000000)
						{
                            i++;
						}
						if(i>=500000000)
						{
							//sheet_updown(key_win,-1);
							//wrong=0;
							putfonts8_asc_sht(key_win, 250, 300, COL8_000000, 7,  "After 2 seconds can you try again!", 34);
						}
						int i1=0;
						while(i1<=500000000)
						{
							i1++;
						}
						if(i1>=500000000)
						{
							putfonts8_asc_sht(key_win, 250, 300, COL8_000000, 7,  "After 1 seconds can you try again!", 34);
						}
						int i2=0;
						while(i2<=500000000)
						{
							i2++;
						}
						if(i2>=500000000)
						{
							sheet_updown(key_win,-1);
							wrong=0;
						}
							
					}
						
					timer_init(timer4, &fifo, 2);
					timer_settime(timer4, 60);
					putfonts8_asc_sht(sht_boot, 350, 360, COL8_000000, 7,  "Wrong Number!", 13);
					putfonts8_asc_sht(sht_boot, 327-10, 403, 7, 7,  " ", 16);
					//boxfill8(buf_boot, sht_boot->bxsize, 21,sht_boot->vx0 + 400-71, sht_boot->vy0 + 404,sht_boot->vx0 + 400+65, sht_boot->vy0 + 418);
					cursor_pin = 327;//光标恢复原位
					for (p=0; p<8; p++) flag_mima[p] = 0;
					cursor_cpin = 0;
					mima_count = 0;
					flag_mima[6] = 1; 
				}

				if (i == 2) //密码错误
				{
					putfonts8_asc_sht(sht_boot, 350, 360, COL8_000000, 28,  "                                     ", 33);
				}
            
		}
		//sheet_updown(sht_boot,-1);
	}
    sheet_updown(sht_boot,-1);
    sheet_updown(sht_back,  0);
    sheet_updown(key_win,   1); 
    sheet_updown(sht_mouse, 2);

    for (;;) {

        int count_pro=0;
        int i;
        for(i=0;i<10;i++)
        count_pro+=taskctl->level[i].running;    //把所有在运行的进程统计出来
        // task = key_win->task;
        // cons_putstr0(task->cons, "\nBreak(key) :\n");
        if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
            /* �L�[�{�[�h�R���g���[���ɑ���f�[�^������΁A���� */
            keycmd_wait = fifo32_get(&keycmd);
            wait_KBC_sendready();
            io_out8(PORT_KEYDAT, keycmd_wait);
    	}
        io_cli();
        if (fifo32_status(&fifo) == 0) {
            /* FIFO��������ۂɂȂ����̂ŁA�ۗ����Ă���`�悪����Ύ��s���� */
            if (new_mx >= 0) {
                io_sti();
                sheet_slide(sht_mouse, new_mx, new_my);
                new_mx = -1;
            } else if (new_wx != 0x7fffffff) {
                io_sti();
                sheet_slide(sht, new_wx, new_wy);
                new_wx = 0x7fffffff;
            } else {
                task_sleep(task_a);
                io_sti();
    		}
        } else {
            i = fifo32_get(&fifo);
            io_sti();
            if (key_win != 0 && key_win->flags == 0) {  /* �E�B���h�E������ꂽ */
                if (shtctl->top == 1) { /* �����}�E�X�Ɣw�i�����Ȃ� */
                    key_win = 0;
            	} else {
                    key_win = shtctl->sheets[shtctl->top - 1];
                    keywin_on(key_win);
        		}
    		}
            if (256 <= i && i <= 511) { /* �L�[�{�[�h�f�[�^ */
                if (i < 0x80 + 256) { /* �L�[�R�[�h�𕶎��R�[�h�ɕϊ� */
                    if (key_shift == 0) {
                        s[0] = keytable0[i - 256];
                	} else {
                        s[0] = keytable1[i - 256];
        			}
            	} else {
                	s[0] = 0;
        		}
                if ('A' <= s[0] && s[0] <= 'Z') {   /* ���͕������A���t�@�x�b�g */
                    if (((key_leds & 4) == 0 && key_shift == 0) ||
                            ((key_leds & 4) != 0 && key_shift != 0)) {
                        s[0] += 0x20;   /* �啶�����������ɕϊ� */
        			}
        		}
                if (s[0] != 0 && key_win != 0) { /* �ʏ핶���A�o�b�N�X�y�[�X�AEnter */
                    fifo32_put(&key_win->task->fifo, s[0] + 256);
        		}
                if (i == 256 + 0x0f && key_win != 0) {	/* Tab */
                    keywin_off(key_win);
                    j = key_win->height - 1;
                    if (j == 0) {
                        j = shtctl->top - 1;
        			}
                    key_win = shtctl->sheets[j];
                    keywin_on(key_win);
        		}
                if (i == 256 + 0x2a) {  /* ���V�t�g ON */
                    key_shift |= 1;
        		}
                if (i == 256 + 0x36) {  /* �E�V�t�g ON */
                    key_shift |= 2;
        		}
                if (i == 256 + 0xaa) {  /* ���V�t�g OFF */
                    key_shift &= ~1;
        		}
                if (i == 256 + 0xb6) {  /* �E�V�t�g OFF */
                    key_shift &= ~2;
        		}
                if (i == 256 + 0x3a) {  /* CapsLock */
                    key_leds ^= 4;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
        		}
                if (i == 256 + 0x45) {  /* NumLock */
                    key_leds ^= 2;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
        		}
                if (i == 256 + 0x46) {  /* ScrollLock */
                    key_leds ^= 1;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
        		}
                if (i == 256 + 0x3b && key_shift != 0 && key_win != 0) {    /* Shift+F1 */ //强制结束
                    task = key_win->task;
                    if (task != 0 && task->tss.ss0 != 0) {
                        cons_putstr0(task->cons, "\nBreak(key) :\n");
                        io_cli();   /* �����I���������Ƀ^�X�N���ς��ƍ��邩�� */
                        task->tss.eax = (int) &(task->tss.esp0);
                        task->tss.eip = (int) asm_end_app;
                		io_sti();
                        task_run(task, -1, 0);  /* �I���������m���ɂ�点�邽�߂ɁA�Q�Ă�����N���� */
                        count_pro-=1;
        			}
        		}
                if (i == 256 + 0x3c && key_shift != 0) {    /* Shift+F2 */
                    /* �V����������R���\�[������͑I����Ԃɂ���i���̂ق����e�؂���ˁH�j */
                    if (key_win != 0) {
                        keywin_off(key_win); //关闭命令行光标
        			}
                    key_win = open_console(shtctl, memtotal);
                    sheet_slide(key_win, 32, 4);
                    sheet_updown(key_win, shtctl->top);
                    keywin_on(key_win);
                     count_pro+=1;
        		}
                if(i == 256 + 0x3d && key_shift != 0){    /* Shift+F3 */  // 进程切换，进程处理
                        char strii[21]={0};
                        task = key_win->task;
                        sprintf(strii,"%d",count_pro);
                        cons_putstr0(task->cons," process number:\n");
                        cons_putstr0(task->cons,strii);
                        }
                if (i == 256 + 0x57) {	/* F11 */
                    sheet_updown(shtctl->sheets[1], shtctl->top - 1);
        		}
                if (i == 256 + 0xfa) {  /* �L�[�{�[�h���f�[�^�𖳎��Ɏ󂯎���� */
                    keycmd_wait = -1;
        		}
                if (i == 256 + 0xfe) {  /* �L�[�{�[�h���f�[�^�𖳎��Ɏ󂯎��Ȃ����� */
                    wait_KBC_sendready();
                    io_out8(PORT_KEYDAT, keycmd_wait);
        		}
            } else if (512 <= i && i <= 767) { /* �}�E�X�f�[�^ */
                if (mouse_decode(&mdec, i - 512) != 0) {
                    /* �}�E�X�J�[�\���̈ړ� */
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) {
                		mx = 0;
        			}
                    if (my < 0) {
                		my = 0;
        			}
                    if (mx > binfo->scrnx - 1) {
                        mx = binfo->scrnx - 1;
        			}
                    if (my > binfo->scrny - 1) {
                        my = binfo->scrny - 1;
        			}
                    new_mx = mx;
                    new_my = my;
                    if ((mdec.btn & 0x01) != 0) {
                        /* ���{�^���������Ă��� */
                    	if (mmx < 0) {
                            /* �ʏ탂�[�h�̏ꍇ */
                            /* ��̉��������珇�ԂɃ}�E�X���w���Ă��鉺������T�� */
                            for (j = shtctl->top - 1; j > 0; j--) {
                                sht = shtctl->sheets[j];
                            	x = mx - sht->vx0;
                            	y = my - sht->vy0;
                                if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize) {
                                    if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
                                        sheet_updown(sht, shtctl->top - 1);
                                		if (sht != key_win) {
                                			keywin_off(key_win);
                            				key_win = sht;
                                			keywin_on(key_win);
            							}
                                        if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21) {
                                            mmx = mx;	/* �E�B���h�E�ړ����[�h�� */
                    						mmy = my;
                            				mmx2 = sht->vx0;
                                			new_wy = sht->vy0;
            							}
                                        if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19) {
                                        	/* �u�~�v�{�^���N���b�N */
                                            if ((sht->flags & 0x10) != 0) {     /* �A�v����������E�B���h�E���H */
                                				task = sht->task;
                                                cons_putstr0(task->cons, "\nBreak(mouse) :\n");
                                                io_cli();   /* �����I���������Ƀ^�X�N���ς��ƍ��邩�� */
                                                task->tss.eax = (int) &(task->tss.esp0);
                                                task->tss.eip = (int) asm_end_app;
                        						io_sti();
                                    			task_run(task, -1, 0);
                                    		} else {	/* �R���\�[�� */
                                				task = sht->task;
                                                sheet_updown(sht, -1); /* �Ƃ肠������\���ɂ��Ă��� */
                                				keywin_off(key_win);
                                                key_win = shtctl->sheets[shtctl->top - 1];
                                				keywin_on(key_win);
                        						io_cli();
                                        		fifo32_put(&task->fifo, 4);
                        						io_sti();
            								}
            							}
                						break;
            						}
            					}
        					}
                		} else {
                            /* �E�B���h�E�ړ����[�h�̏ꍇ */
                            x = mx - mmx;   /* �}�E�X�̈ړ��ʂ��v�Z */
                    		y = my - mmy;
                            new_wx = (mmx2 + x + 2) & ~3;
                            new_wy = new_wy + y;
                            mmy = my;   /* �ړ���̍��W�ɍX�V */
        				}
                	} else {
                        /* ���{�^���������Ă��Ȃ� */
                        mmx = -1;   /* �ʏ탂�[�h�� */
                        if (new_wx != 0x7fffffff) {
                            sheet_slide(sht, new_wx, new_wy);	/* ��x�m�肳���� */
                            new_wx = 0x7fffffff;
        				}
        			}
        		}
            } else if (768 <= i && i <= 1023) { /* �R���\�[���I������ */
                close_console(shtctl->sheets0 + (i - 768));
            } else if (1024 <= i && i <= 2023) {
                close_constask(taskctl->tasks0 + (i - 1024));
            } else if (2024 <= i && i <= 2279) {    /* �R���\�[����������� */
                sht2 = shtctl->sheets0 + (i - 2024);
                memman_free_4k(memman, (int) sht2->buf, CONSOLE_W * CONSOLE_H);
                sheet_free(sht2);
    		}
            //��ʾ����
            sprintf(s, "DATE: %d-%d-%d", get_year(), get_mon_hex(), get_day_of_month());
            putfonts8_asc_sht(sht_back, binfo->scrnx - 180, binfo->scrny -20, COL8_000000, COL8_C6C6C6, s, 15);
            //��ʾʱ��
            sprintf(s, "%d:%d", get_hour_hex(), get_min_hex());
            putfonts8_asc_sht(sht_back, binfo->scrnx - 45, binfo->scrny -20, COL8_000000, COL8_C6C6C6, s, 5);
            sheet_refresh(sht_back, binfo->scrnx - 130, binfo->scrny -20,binfo->scrnx - 45 + 5*8, binfo->scrny -50+16);
    	}
    }
}

void keywin_off(struct SHEET *key_win)
{
    change_wtitle8(key_win, 0);
    if ((key_win->flags & 0x20) != 0) {
        fifo32_put(&key_win->task->fifo, 3); /* �R���\�[���̃J�[�\��OFF */
    }
    return;
}

void keywin_on(struct SHEET *key_win)
{
    change_wtitle8(key_win, 1);
    if ((key_win->flags & 0x20) != 0) {
        fifo32_put(&key_win->task->fifo, 2); /* �R���\�[���̃J�[�\��ON */
    }
    return;
}

struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct TASK *task = task_alloc();
    int *cons_fifo = (int *) memman_alloc_4k(memman, 128 * 4);
    task->cons_stack = memman_alloc_4k(memman, 64 * 1024);
    task->tss.esp = task->cons_stack + 64 * 1024 - 12;
    task->tss.eip = (int) &console_task;
    task->tss.es = 1 * 8;
    task->tss.cs = 2 * 8;
    task->tss.ss = 1 * 8;
    task->tss.ds = 1 * 8;
    task->tss.fs = 1 * 8;
    task->tss.gs = 1 * 8;
    *((int *) (task->tss.esp + 4)) = (int) sht;
    *((int *) (task->tss.esp + 8)) = memtotal;
    task_run(task, 2, 2); /* level=2, priority=2 */
    fifo32_init(&task->fifo, 128, cons_fifo, task);
    return task;
}

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct SHEET *sht = sheet_alloc(shtctl);
    // unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 300 * 200);
    // sheet_setbuf(sht, buf, 300, 200, -1); /* �����F�Ȃ� */
    // make_window8(buf, 300, 200, "console", 0);
    // make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
    unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 4*CONSOLE_W * CONSOLE_H);
    sheet_setbuf(sht, buf, CONSOLE_W, CONSOLE_H, -1); /* �����F�Ȃ� */
    make_window8(buf, CONSOLE_W, CONSOLE_H,  "console", 0);
    make_textbox8(sht, 8, 28,  CONSOLE_W-16 , CONSOLE_H-37, COL8_000000);
    sht->task = open_constask(sht, memtotal);
    sht->flags |= 0x20; /* �J�[�\������ */
    return sht;
}

struct SHEET *open_log(struct SHTCTL *shtctl, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	unsigned char *buf = (unsigned char *) memman_alloc_4k(memman, 4*CONSOLE_W * CONSOLE_H);
	sheet_setbuf(sht, buf, CONSOLE_W, CONSOLE_H, -1);
	make_window8(buf, CONSOLE_W, CONSOLE_H, "log", 0);
	make_textbox8(sht, 8, 28, CONSOLE_W-16 , CONSOLE_H-37, COL8_000000);
	sht->task = open_constask(sht, memtotal);
	sht->flags |= 0x20;
	return sht;
}

void close_constask(struct TASK *task)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    task_sleep(task);
    memman_free_4k(memman, task->cons_stack, 64 * 1024);
    memman_free_4k(memman, (int) task->fifo.buf, 128 * 4);
    task->flags = 0; /* task_free(task); �̑��� */
    return;
}

void close_console(struct SHEET *sht)
{
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct TASK *task = sht->task;
    memman_free_4k(memman, (int) sht->buf, CONSOLE_W * CONSOLE_H);
    sheet_free(sht);
    close_constask(task);
    return;
}

void task_b_main(struct SHEET *key_win_b)
{
	struct FIFO32 fifo;
	struct TIMER *timer_1s;
	int i, fifobuf[128], count = 0, count0 = 0;
	char s[12];

	fifo32_init(&fifo, 128, fifobuf, 0);
	timer_1s = timer_alloc();
	timer_init(timer_1s, &fifo, 100);
	timer_settime(timer_1s, 100);

	for (;;) {
		count++;
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i == 100) {
				sprintf(s, "%11d", count - count0);
				putfonts8_asc_sht(key_win_b, 24, 28, COL8_000000, COL8_C6C6C6, s, 11);
				count0 = count;
				timer_settime(timer_1s, 100);
			}
		}
	}
}

// void create_process(struct MEMMAN *memman,struct SHEET *shtctl)
// {
//                     int cursor_x, cursor_c;
//                 	unsigned char *buf_win, *buf_win_b;
//                     struct SHEET *key_win, *key_win_b[3];
//                     struct TASK *task_a, *task_b[3];
//                     struct TIMER *timer;
//                     char s[128]
//                     task_a = task_init(memman);
// 	                    fifo.task = task_a;
// 	                    task_run(task_a, 1, 2);
                        
//                         struct SHEET *key_win_b[3];
//                         unsigned char *buf_win_b;

//                         /* key_win */
//                         key_win   = sheet_alloc(shtctl);
//                         buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
//                         sheet_setbuf(key_win, buf_win, 144, 52, -1);
//                         make_window8(buf_win, 144, 52, "task_a", 1);
//                         make_textbox8(key_win, 8, 28, 128, 16, COL8_FFFFFF);
//                         cursor_x = 8;
//                         cursor_c = COL8_FFFFFF;
//                         timer = timer_alloc();
//                         timer_init(timer, &fifo, 1);
//                         timer_settime(timer, 50);

//                         /* key_win_b */
//                     for (int i = 0; i < 3; i++) {

//                         key_win_b[i] = sheet_alloc(shtctl);
//                         buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
//                         sheet_setbuf(key_win_b[i], buf_win_b, 144, 52, -1); /* 摟柧怓側偟 */
//                         sprintf(s, "task_b%d", i);
//                         make_window8(buf_win_b, 144, 52, s, 0);
//                         task_b[i] = task_alloc();
//                         task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
//                         task_b[i]->tss.eip = (int) &task_b_main;
//                         task_b[i]->tss.es = 1 * 8;
//                         task_b[i]->tss.cs = 2 * 8;
//                         task_b[i]->tss.ss = 1 * 8;
//                         task_b[i]->tss.ds = 1 * 8;
//                         task_b[i]->tss.fs = 1 * 8;
//                         task_b[i]->tss.gs = 1 * 8;
//                         *((int *) (task_b[i]->tss.esp + 4)) = (int) key_win_b[i];
//                         task_run(task_b[i], 2, i + 1);
// 	                    }
//                         sheet_slide(key_win_b[0], 168,  56);
//                         sheet_slide(key_win_b[1],   8, 116);
//                         sheet_slide(key_win_b[2], 168, 116);
//                         sheet_slide(key_win,        8,  56);
//                         //sheet_updown(sht_back,     0);
//                         sheet_updown(key_win_b[0], 1);
//                         sheet_updown(key_win_b[1], 2);
//                         sheet_updown(key_win_b[2], 3);
//                         sheet_updown(key_win,      4);
//                         //sheet_updown(sht_mouse,    5);
//                         }
//                         close_console(key_win);
                        

//                     	task_a = task_init(memman);
// 	                    fifo.task = task_a;
// 	                    task_run(task_a, 1, 2);
//                         // struct SHEET *key_win_b[3];
//                         // unsigned char *buf_win_b;

//                         /* key_win */
//                         key_win   = sheet_alloc(shtctl);
//                         buf_win   = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
//                         sheet_setbuf(key_win, buf_win, 144, 52, -1); /* 摟柧怓側偟 */
//                         make_window8(buf_win, 144, 52, "task_a", 1);
//                         make_textbox8(key_win, 8, 28, 128, 16, COL8_FFFFFF);
//                         cursor_x = 8;
//                         cursor_c = COL8_FFFFFF;
//                         timer = timer_alloc();
//                         timer_init(timer, &fifo, 1);
//                         timer_settime(timer, 50);

//                         /* key_win_b */
//                     for (i = 0; i < 3; i++) {

//                         key_win_b[i] = sheet_alloc(shtctl);
//                         buf_win_b = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
//                         sheet_setbuf(key_win_b[i], buf_win_b, 144, 52, -1); /* 摟柧怓側偟 */
//                         sprintf(s, "task_b%d", i);
//                         make_window8(buf_win_b, 144, 52, s, 0);
//                         task_b[i] = task_alloc();
//                         task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
//                         task_b[i]->tss.eip = (int) &task_b_main;
//                         task_b[i]->tss.es = 1 * 8;
//                         task_b[i]->tss.cs = 2 * 8;
//                         task_b[i]->tss.ss = 1 * 8;
//                         task_b[i]->tss.ds = 1 * 8;
//                         task_b[i]->tss.fs = 1 * 8;
//                         task_b[i]->tss.gs = 1 * 8;
//                         *((int *) (task_b[i]->tss.esp + 4)) = (int) key_win_b[i];
//                         task_run(task_b[i], 2, i + 1);
// 	                    }
//                         sheet_slide(key_win_b[0], 168,  56);
//                         sheet_slide(key_win_b[1],   8, 116);
//                         sheet_slide(key_win_b[2], 168, 116);
//                         sheet_slide(key_win,        8,  56);
//                         sheet_updown(sht_back,     0);
//                         sheet_updown(key_win_b[0], 1);
//                         sheet_updown(key_win_b[1], 2);
//                         sheet_updown(key_win_b[2], 3);
//                         sheet_updown(key_win,      4);
//                         sheet_updown(sht_mouse,    5);
// }

void bootpage(unsigned char *buf, int xsize, int ysize){
	boxfill8(buf, xsize, 28, 0, 0, xsize, ysize);
	boxfilly(buf, xsize, 7, 300, 100, 500, 300);
	boxfilly(buf, xsize, 8, 375, 150, 425, 200);
	boxfilly(buf, xsize, 7, 382, 157, 418, 193);
	int i,j,x,y;
	int x0 = xsize / 2;
	for(i=350;i<=450;i++){
		for(j=200;j<=250;j++){
			if((i-400)*(i-400)+(j-250)*(j-250)>=40*40&&(i-400)*(i-400)+(j-250)*(j-250)<=50*50){
				buf[j * xsize + i] = COL8_C6C6C6;
			}
		}
	}
	static char cursor[24][70]=
	{
      "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OO***OOOOOO***OOOO************OOOOOOO************OOOO*********OOOOOOOO",
	  "OO***OOOOOO***OOO***OOOOOOOOO***OOOOO***OOOOOOOOOOOOO***OOOOOO***OOOOO",
	  "OO***OOOOOO***OOOOO***OOOOOOOOO**OOOO***OOOOOOOOOOOOO***OOOOOO***OOOOO",
	  "OO***OOOOOO***OOOOOOO***OOOOOOOOOOOOO***OOOOOOOOOOOOO***OOOOO***OOOOOO",
	  "OO***OOOOOO***OOOOOOOOOO***OOOOOOOOOO************OOOO********OOOOOOOOO",
	  "OO***OOOOOO***OOOOOOOOOOOOO***OOOOOOO***OOOOOOOOOOOOO***OOO***OOOOOOOO",
	  "OO***OOOOOO***OOOOOOOOOOOOOOO***OOOOO***OOOOOOOOOOOOO***OOOOO***OOOOOO",
	  "OO***OOOOOO***OOOOOOOOOOOOOOOOO***OOO***OOOOOOOOOOOOO***OOOOOOO***OOOO",
	  "OOO*********OOOOOOO*************OOOOO************OOOO***OOOOOOOOO***OO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",
	  "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO",

	}; 
	for (y = 0; y < 24; y++) {
		for (x = 0; x < 70; x++) {
			if (cursor[y][x] == '*') {
				buf[(y+310) * xsize + x+ x0-35] = 7;
			}
			/*if (cursor[y][x] == 'O') {
				buf[(y+320) * xsize + x+x0-20] = 16;
			}*/
		}
	}
	//密码输入框
    boxfill8(buf, xsize, 8, x0-85, 400,x0+69+30-10 , 422);
    boxfill8(buf, xsize, 7, x0-83, 402,x0+67-10 , 420);
    //boxfill8(buf, xsize, 7, x0-71, 404,x0+65, 418);
    //boxfill8(buf, xsize, 8, x0+70, 400,x0+72 , 422);

    static char cursor2[15][16] =
    {
        "oooooooo*ooooooo",
        "oooooooo**oooooo",
        "oooooooo***ooooo",
        "oooooooo****oooo",
        "oooooooo*****ooo",
        "oooooooo******oo",
        "***************o",
        "****************",
        "***************o",
        "oooooooo******oo",
        "oooooooo*****ooo",
        "oooooooo****oooo",
        "oooooooo***ooooo",
        "oooooooo**oooooo",
        "oooooooo*ooooooo"
    };

    for (y = 0; y < 15; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor2[y][x] == '*') {
				buf[(y+404) * xsize + x+ x0+75-10] = 7;
			}
		}
	}

}


//输入密码，每次返回光标的坐标
int cin_pin(struct SHEET *sht, int i, char keytable[],int cursor_pin, int cursor_cpin, 
    struct TIMER *timer3, struct FIFO32 *fifo, char mima[], int *mima_count, int *flag_mima) //i为中断发出的数据
{
    char s[2];
    if (i >= 256 && i < 0x54 + 256) 
    {
		if (keytable[i - 256] != 0 && cursor_pin < 455) 
        { /*键盘输入 */
				//s[0] = keytable[i - 256];
                s[0] = keytable[55];
				s[1] = 0;
				putfonts8_asc_sht(sht, cursor_pin-10, 403,COL8_000000, 7,  s, 1);
				cursor_pin += 8;
                //判断密码是否正确
                if (*mima_count >= 6) 
                        flag_mima[6] = 0;
                else if (keytable[i - 256] == mima[*mima_count])
                {
                    flag_mima[*mima_count] = 1;
                }
                else flag_mima[*mima_count] = 0;

                (*mima_count)++;
		}
	}
	if (i == 256 + 0x0e && cursor_pin > 327) 
    { /* 删除键 */
		putfonts8_asc_sht(sht, cursor_pin-10, 403, COL8_000000, 7,  " ", 1);
		cursor_pin -= 8;

        if (*mima_count < 6)
            flag_mima[*mima_count] = 0;
        else if (*mima_count == 6)
            flag_mima[6] = 1;
        
        (*mima_count)--;
	}
	boxfill8(sht->buf, sht->bxsize, 7, cursor_pin-10, 403, cursor_pin-10 + 1, 419);
    boxfill8(sht->buf, sht->bxsize, 7, cursor_pin+1-10, 403, cursor_pin + 7-10, 419);
	sheet_refresh(sht, cursor_pin-10, 403, cursor_pin-10 + 7, 419);

    if (i == 256 + 0x1c)//enter键
    {
        flag_mima[7] = 1;
    }
    return cursor_pin;
}
int if_right(int *flag_mima) //判断密码是否正确
{
    int i;
    for (i=0; i<8; ++i)
    {
        if (flag_mima[i] == 0)
            return 0;
    }
    return 1;
}