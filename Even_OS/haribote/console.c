/* ï¿?Rï¿½ï¿½ï¿?\ï¿?[ï¿½ï¿½ï¿½Ö?W */
#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define CMDHIS_NR 32
#define CMDBUF 1024


struct CMDHISTORY {
int first, last, head, tail, now;
char ** argv;
char *buf;
};

struct CONSOLE *log;
//unsigned int *g_current_dir;
// int g_pathname_length = 0;
int console_id=0;
struct MYFILEDATA *setfdata = 0;


struct CMDHISTORY cmdhis;
char cmd_buf[CMDBUF];

cmdhisbuf = cmd_buf;

char s[100];
int gdt_addr;
int ldt_addr_offset;
struct SEGMENT_DESCRIPTOR temp;
int ldt_addr;
int phy_addr;

int gdt;
int ldt_index;
int ldt_des;
int ds_index;
int ds_des;
int ds_addr;
unsigned int addrlist[100];
unsigned int sizelist[100];
int num = 0;

struct S mutex;
struct S wrt;
int readcount;
int share_bupt;


/* debugæ¨¡å?æ????¥æ??ä»¶ç³»ç»????é¢? */
void debug_print(char *str){
	// char s[50];
	// sprintf(s, "[debug] ");
	// cons_putstr(log, s);
	// int i;
	// for(i=0; str[i]!='0' && str[i]!='\0'; i++){
	// 	if(i == 150){
	// 		str[i] = '0';
	// 		break;
	// 	}
	// }

	// if(i<150){
	// 	cons_putstr(log, str);
	// }else{
	// 	sprintf(s, "[CAUTION:(str.length>150)]");
	// 	cons_putstr(log, s);
	// 	cons_putstr(log, str);
	// }
	
	return;
}

/**
 * manage console tasks using memory domain
 */
void console_task(struct SHEET *sheet, int memtotal)
{
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i, *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	struct CONSOLE cons;
	struct FILEHANDLE fhandle[8];
	char cmdline[100];
	// char s[100]; // for debug
	int path_length = 0; // for calculating cmdline
	unsigned char *nihongo = (char *) *((int *) 0x0fe8);

	cons.sht = sheet;
	cons.cur_x =  8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	cons.current_dir = (struct MYDIRINFO *) ROOT_DIR_ADDR;
	cons.id = console_id;
	console_id++;
	task->cons = &cons;
	task->cmdline = cmdline;

	if (cons.sht != 0) {
		cons.timer = timer_alloc();
		timer_init(cons.timer, &task->fifo, 1);
		timer_settime(cons.timer, 50);
	}
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	for (i = 0; i < 8; i++) {
		fhandle[i].buf = 0;	
	}
	task->fhandle = fhandle;
	task->fat = fat;
	if (nihongo[4096] != 0xff) {	/* ï¿½ï¿½ï¿?{ï¿½ï¿½tï¿?Hï¿½ï¿½ï¿?gï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½????ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½H */
		task->langmode = 1;
	} else {
		task->langmode = 0;
	}
	task->langbyte1 = 0;
	task->langmode = 3;

	/* ï¿?vï¿½ï¿½ï¿½ï¿½ï¿?vï¿?gï¿?\ï¿½ï¿½ */
	if(cons.id == 1){
		cmd_mkfs(&cons);	/* ï¿½ï¿½ï¿½ï¿½ï¿?Rï¿½ï¿½ï¿?\ï¿?[ï¿½ï¿½ï¿½É???ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½Iï¿½ï¿½mkfsï¿½ï¿½ï¿?gï¿½ï¿½ */
	}else if(cons.id == 0){
		cmd_setlog(&cons);	/* ï¿½ï¿½ï¿?Oï¿?pï¿?Rï¿½ï¿½ï¿?\ï¿?[ï¿½ï¿½ï¿½É???ï¿½ï¿½??ï¿½ï¿½Oï¿?oï¿½Í?pï¿½Ì???ï¿½ï¿½ï¿?{ï¿½ï¿½ */
	}
	path_length = cons_putdir(&cons);
	
	cons_putchar(&cons, '>', 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons.sht != 0) { /* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿?pï¿?^ï¿?Cï¿?} */
				if (i != 0) {
					timer_init(cons.timer, &task->fifo, 0); /* ï¿½ï¿½ï¿½ï¿½0ï¿½ï¿½ */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(cons.timer, &task->fifo, 1); /* ï¿½ï¿½ï¿½ï¿½1ï¿½ï¿½ */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
			}
			if (i == 2) {	/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ON */
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½OFF */
				if (cons.sht != 0) {
					boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000,
							cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				cons.cur_c = -1;
			}
			if (i == 4) {	/* ï¿?Rï¿½ï¿½ï¿?\ï¿?[ï¿½ï¿½ï¿½Ì?uï¿?~ï¿?vï¿?{ï¿?^ï¿½ï¿½ï¿?Nï¿½ï¿½ï¿?bï¿?N */
				cmd_exit(&cons, fat);
			}
			if (i == 5) {	//È¡ï¿½ï¿½ï¿½ï¿½5  
				//cmd_app(&cons, fat, "lines");
			}
			if (i == 6) {	//È¡ï¿½ï¿½ï¿½ï¿½6  
				cmd_app(&cons, fat, "noodle");
			}
			if (i == 7) {	//È¡ï¿½ï¿½ï¿½ï¿½7  
		    	//cmd_app(&cons, fat, "star1"); 
			}
			if (i == 8) {	//È¡ï¿½ï¿½ï¿½ï¿½8  
				//cmd_app(&cons, fat, "color2"); 
			}
			if (i == 9) {	//È¡ï¿½ï¿½ï¿½ï¿½9  
				//cmd_app(&cons, fat, "walk");
			}
			if (i == 10) {	//È¡ï¿½ï¿½ï¿½ï¿½10    ï¿½ï¿½ï¿½ï¿½ 
				
			}
			if (i == 11) {	//È¡ï¿½ï¿½ï¿½ï¿½11 
			   // cmd_app(&cons, fat, "bball");
			}
			if (i == 12) {	//Í¼Æ¬ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½  task->cmdline
				cmdline[0]='G'; cmdline[1]='V'; cmdline[2]='I'; cmdline[3]='E'; cmdline[4]='W';  cmdline[5]=' '; 
				cmdline[6]='5'; cmdline[7]='.'; cmdline[8]='J'; cmdline[9]='P'; cmdline[10]='G'; cmdline[11]=0; 
				cons_runcmd(cmdline, &cons, fat, memtotal);
			}
			if (i == 13) {	 //Í¼Æ¬ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½  task->cmdline
				cmd_app(&cons, fat, "invader");
			}
			if (i == 18) {	
			    cmd_app(&cons, fat, "user");
			}
			if (i == 21) {	
			    newBackGround("newbg 2.jpg", fat, &cons);
			}
			if (i == 22) {	
			    newBackGround("newbg 3.jpg", fat, &cons);
			}
			if (i == 23) {	
			    cmd_app(&cons, fat, "time");
			}
			if (256 <= i && i <= 511) { /* ï¿?Lï¿?[ï¿?{ï¿?[ï¿?hï¿?fï¿?[ï¿?^ï¿?iï¿?^ï¿?Xï¿?NAï¿?oï¿?Rï¿?j */
				if (i == 8 + 256) { /* ï¿?oï¿?bï¿?Nï¿?Xï¿?yï¿?[ï¿?X */
					if (cons.cur_x > 16 + path_length * 8) {
						/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½ï¿?Xï¿?yï¿?[ï¿?Xï¿½Å?ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿?Aï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½1ï¿½Â???ï¿? */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) { /* if press Enter */
					/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½ï¿?Xï¿?yï¿?[ï¿?Xï¿½Å?ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½ï¿?sï¿½ï¿½ï¿½ï¿½ */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - (path_length) - 2] = 0;
					cons_newline(&cons);

					cmdhis.argv = (char **)cmd_buf;
					*cmdhis.argv = cmdhis.buf;
					cmdhis.first = 0;
					cmdhis.last = 0;
					cmdhis.head = CMDHIS_NR * 4;
					cmdhis.tail = 0; /* not use but we reserving it */
					cmdhis.now = 0;

					// *****ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½ï¿½ï¿?Cï¿½ï¿½ï¿½Ì?fï¿?oï¿?bï¿?Oï¿?Rï¿?[ï¿?h*****
					// sprintf(s, "original cmdline = %s[EOF]\n", cmdline);
					// cons_putstr(&cons, s);

											/* ???ä»¥ä??æ·»å??å¦?ä¸?ä»£ç?? */
						if (cmdline[0] != 0) {
						if (cmdhis.head+strlen(cmdline)+1 >= CMDBUF) {
						cmdhis.head = CMDHIS_NR << 2;
						}
						if ((unsigned int)(cmdhis.buf + cmdhis.head) <=
						(unsigned int)*(cmdhis.argv + cmdhis.last) &&
						cmdhis.first != cmdhis.last) {
						while ((unsigned int)(cmdhis.buf + cmdhis.head) +
						strlen(cmdline) + 1 >
						(unsigned int)*(cmdhis.argv + cmdhis.last)) {
						if (++cmdhis.last == CMDHIS_NR) cmdhis.last = 0;
						if (cmdhis.last == cmdhis.first) break;

						}
						}
						strcpy(cmdhis.buf + cmdhis.head, cmdline);
						*(cmdhis.argv + cmdhis.first) = cmdhis.buf + cmdhis.head;

						cmdhis.head = cmdhis.head + strlen(cmdline) + 1;
						cmdhis.now = cmdhis.first;
						if(++cmdhis.first == CMDHIS_NR) cmdhis.first = 0;
						if(cmdhis.last == cmdhis.first) {
						if(++cmdhis.last == CMDHIS_NR) cmdhis.last = 0;
						}
						*(cmdhis.argv + cmdhis.first) = 0;
						}

					cons_runcmd(cmdline, &cons, fat, memtotal);	/* ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½ï¿½ï¿?s */
					if (cons.sht == 0) {
						cmd_exit(&cons, fat);
					}
					/* ï¿?vï¿½ï¿½ï¿½ï¿½ï¿?vï¿?gï¿?\ï¿½ï¿½ */
					path_length = cons_putdir(&cons);
					cons_putchar(&cons, '>', 1);
				} else if (i == 18 + 256 && (*(cmdhis.argv+cmdhis.now) != 0)) {
					while(cons.cur_x > 24) {
					cons_putchar(&cons, ' ', 0);
					cons.cur_x -= 8;
					}
					cons_putstr0(&cons, *(cmdhis.argv + cmdhis.now));
					strcpy(cmdline, *(cmdhis.argv+cmdhis.now));

					if(cmdhis.now == cmdhis.last) cmdhis.now++;
					cmdhis.now--;
					if(cmdhis.first < cmdhis.last && cmdhis.now < 0)
					cmdhis.now = CMDHIS_NR-1;
					} else if (i == 20 + 256 && (*(cmdhis.argv+cmdhis.now) != 0)) {
						while(cons.cur_x > 24) {
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
						}
						cons_putstr0(&cons, *(cmdhis.argv + cmdhis.now)); 
						strcpy(cmdline, *(cmdhis.argv+cmdhis.now));
						cmdhis.now++;

						if(cmdhis.now == cmdhis.first) cmdhis.now--;
						if(cmdhis.now == CMDHIS_NR){
						if(cmdhis.first == 0) {
						cmdhis.now--;
						}else {
						cmdhis.now = 0;
						}
						}
						}			
				
				else {
					/* ï¿½ï¿½??ï¿½ï¿½ï¿? */
					if (cons.cur_x < 240) {
						/* ï¿½ê?¶ï¿½ï¿½ï¿½\ï¿½ï¿½ï¿½ï¿½ï¿½Ä?ï¿½ï¿½ï¿?Aï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½1ï¿½Â?iï¿½ß?ï¿? */
						/* ï¿½ï¿½ï¿½ï¿½,
						 * cons.cur_x / 8 = ï¿?wï¿½ï¿½ï¿½ï¿½ï¿½Ú???ï¿½ï¿½ï¿?
						 * -2 = 0ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½('>')ï¿½Í???????ï¿?
						 */
						cmdline[cons.cur_x / 8 - (path_length) - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½Ä?\ï¿½ï¿½ */
			if (cons.sht != 0) {
				if (cons.cur_c >= 0) {
					boxfill8(cons.sht->buf, cons.sht->bxsize, cons.cur_c,
							cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
			}
		}
	}
}

/**
 * put character in specific console.
 */
void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {	/* ï¿?^ï¿?u */
		for (;;) {
			if (cons->sht != 0) {
				putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			}
			cons->cur_x += 8;
			//if (cons->cur_x == 8 + 240) {
			if (cons->cur_x == 8 + cons->sht->bxsize-16){
				cons_newline(cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;	/* 32ï¿½Å?ï¿½ï¿½ï¿½Ø???½ï¿½ï¿?break */
			}
		}
	} else if (s[0] == 0x0a) {	/* ï¿½ï¿½ï¿?s */
		cons_newline(cons);
	} else if (s[0] == 0x0d) {	/* ï¿½ï¿½ï¿?A */
		/* ï¿½Æ????ï¿½ï¿½ï¿½ï¿½ï¿½È???ï¿½ï¿½ï¿½ï¿½??ï¿? */
	} else {	/* ï¿½ï¿½ï¿½Ê???ï¿½ï¿½ï¿? */
		if (cons->sht != 0) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		}
		if (move != 0) {
			/* moveï¿½ï¿½0ï¿½Ì???ï¿½ï¿½??Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½iï¿½ß???ï¿? */
			cons->cur_x += 8;
			if (cons->cur_x == 8 + cons->sht->bxsize-16) {
				cons_newline(cons);
			}
		}
	}
	return;
}

/**
 * make new line in specified console.
 */
void cons_newline(struct CONSOLE *cons)
{
	int x, y, xmax, ymax;
	xmax = cons->sht->bxsize - 16;
	ymax = cons->sht->bysize - 37;
	struct SHEET *sheet = cons->sht;
	struct TASK *task = task_now();
	//if (cons->cur_y < 28 + 112) {
	if (cons->cur_y < 28 + ymax - 16){
		cons->cur_y += 16; /* ï¿½ï¿½ï¿½Ì?sï¿½ï¿½ */
	} else {
		/* ï¿?Xï¿?Nï¿½ï¿½ï¿?[ï¿½ï¿½ */
		if (sheet != 0) {
			/* VRAMï¿½ï¿½ï¿½Ì?e1ï¿?sï¿½ï¿½ï¿?Aï¿½ï¿½??ï¿½Ì????ï¿½É?Rï¿?sï¿?[ï¿½ï¿½ï¿½ï¿½ */
			for (y = 28; y < 28 + ymax - 16; y++) {
				for (x = 8; x < 8 + xmax; x++) {
					sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
				}
			}

			/* ï¿½Å?ï¿½Ì?sï¿½ï¿½ï¿½ï¿½ï¿½Å?hï¿½ï¿½????ï¿? */
			for (y = 28 + ymax - 16; y < 28 + ymax; y++) {
				for (x = 8; x < 8 + xmax; x++) {
					sheet->buf[x + y * sheet->bxsize] = COL8_000000;
				}
			}
			/* ï¿?Vï¿?[ï¿?gï¿½ï¿½ï¿½ï¿½8<x<248, 28<y<156ï¿½Ì?????ï¿½ï¿½??`ï¿½æ?·ï¿½ï¿? */
			sheet_refresh(sheet, 8, 28, 8 + xmax, 28 + ymax);
		}
	}
	cons->cur_x = 8;
	if (task->langmode == 1 && task->langbyte1 != 0) {
		cons->cur_x = 16;
	}
	return;
}

/**
 * consoleï¿½É?ï¿½ï¿½ï¿½ï¿½ï¿?sï¿½ï¿½ï¿?oï¿½Í?ï¿½ï¿½ï¿?
 */
void cons_putstr(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}


/**
 * consoleï¿½É?ï¿½ï¿½ï¿½ï¿½ï¿?sï¿½ï¿½ï¿?oï¿½Í?ï¿½ï¿½ï¿?
 */
void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

/**
 * consoleï¿½É?ï¿½ï¿½ï¿½ï¿½ï¿?sï¿½ï¿½ï¿?Aï¿½ï¿½ï¿½ï¿½lenï¿½Ü???oï¿½Í?ï¿½ï¿½ï¿?
 */
void cons_putstr1(struct CONSOLE *cons, char *s, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return;
}

/* ï¿½ï¿½ï¿½İ???fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½ï¿½Ì?ï¿½ï¿½Pathï¿½ï¿½\ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½.
 * @return pathnameï¿½Ì?ï¿½ï¿½ï¿½ï¿½ï¿½Ì?ï¿½ï¿½ï¿½ï¿½ï¿½Ô?ï¿?(ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½ï¿½ï¿?Cï¿½ï¿½ï¿½Ì?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½vï¿?Zï¿½ï¿½ï¿½é?½ï¿½ï¿?)
 */
int cons_putdir(struct CONSOLE *cons){
	struct MYDIRINFO *dinfo = cons->current_dir;
	char pathname[MAX_CMDLINE];
	int i;
	int pathname_length = 0;

	get_pathname(pathname, dinfo);	// ï¿?pï¿?Xï¿½ï¿½ï¿½ï¿½Tï¿½ï¿½ï¿½ï¿½ï¿?Apathnameï¿½É?iï¿?[ï¿½ï¿½ï¿½ï¿½
	for(i=0; pathname[i]!='\0';i++) pathname_length++;

	/* pathï¿½ï¿½ï¿?Rï¿½ï¿½ï¿?\ï¿?[ï¿½ï¿½ï¿½É?\ï¿½ï¿½ (ï¿½ï¿½ï¿½Ó?Fï¿½ï¿½ï¿½Ì???ï¿½ï¿½??ï¿½ï¿½sï¿½ï¿½ï¿½ï¿½ï¿½È?ï¿?)*/
	cons_putstr(cons, pathname);
	return pathname_length;
}

/**
 * get current directory path.
 * @param pathname: set path name into pathname
 * @param dinfo: directory information
 */
void get_pathname(char *pathname, struct MYDIRINFO *dinfo){
	char s[100];
	char tempname[MAX_CMDLINE];
	char dirname[MAX_NAME_LENGTH];

	// initialize
	sprintf(pathname, "");
	while(dinfo->parent_dir != 0){
		sprintf(dirname, dinfo->name);
		sprintf(tempname, "%s/%s", dirname, pathname);
		dinfo = (struct MYDIRINFO *)dinfo->parent_dir;

		// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
		sprintf(pathname, "%s", tempname);
		sprintf(dirname, "");
	}

	// pathnameï¿½ï¿½"/"(ROOT)ï¿½ï¿½tï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½B
	sprintf(s, "/%s", pathname);
	sprintf(pathname, "%s", s);

	return;
}


/**
 * read command line and execute called function.
 */
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, int memtotal)
{
	// debug code
	//char s[30];
	//sprintf(s, "cmdline = %s[EOF]\n", cmdline);
	//cons_putstr(cons, s);

	if (strncmp(cmdline, "cat ", 4) == 0 && cons->sht != 0) {
		cmd_cat(cons, fat, cmdline);
	} else if (strncmp(cmdline, "cd ", 3) == 0){
		cmd_cd(cons, cmdline);
	} else if (strcmp(cmdline, "cls") == 0 && cons->sht != 0) {
		cmd_cls(cons);
	} else if (strcmp(cmdline, "dir") == 0 && cons->sht != 0) {
		cmd_dir(cons);
	} else if (strcmp(cmdline, "exit") == 0) {
		cmd_exit(cons, fat);
	} else if (strncmp(cmdline, "edit ", 5) == 0 && cons->sht != 0){
		cmd_edit(cons, cmdline);
	} else if (strcmp(cmdline, "fddir") == 0 && cons->sht != 0) {
		cmd_fddir(cons);
	} else if (strncmp(cmdline, "fview ", 6) == 0 && cons->sht != 0){
		cmd_fview(cons, cmdline);
	} else if (strncmp(cmdline, "langmode ", 9) == 0) {
		cmd_langmode(cons, cmdline);
	} /*else if (strcmp(cmdline, "log") == 0 && cons->sht != 0){
		cmd_log(cons);
	} */else if (strcmp(cmdline, "logcls") == 0 && cons->sht != 0){
		cmd_logcls(cons);
	} else if (strcmp(cmdline, "mem") == 0 && cons->sht != 0) {
		cmd_mem(cons, memtotal);
	} else if (strcmp(cmdline, "mkfs") == 0 && cons->sht != 0){
		cmd_mkfs(cons);
	}else if (strncmp(cmdline, "mkdir ", 6) == 0){
		cmd_mkdir(cons, cmdline);
	} else if (strncmp(cmdline, "mkfile ", 7) == 0){
		cmd_mkfile(cons, cmdline);
	} else if (strcmp(cmdline, "memmap") == 0 && cons->sht != 0) {
		cmd_memmap(cons, memtotal);
	} else if (strncmp(cmdline, "start ", 6) == 0) {
		cmd_start(cons, cmdline, memtotal);
	} else if (strcmp(cmdline, "setlog") == 0 && cons->sht != 0){
		cmd_setlog(cons);
	} else if (strncmp(cmdline, "show ", 5) == 0 && cons->sht != 0){
		cmd_show(cons, cmdline);
	} else if (strcmp(cmdline, "top") == 0 && cons->sht != 0) {
		cmd_top(cons);
	}
	else if(strcmp(cmdline, "stamp") == 0&& cons->sht != 0)
	{
		cmd_stamp(cons, cmdline);
	}else if (strcmp(cmdline, "reader")== 0) {
		cmd_reader();
	} else if (strcmp(cmdline, "writer")== 0) {
		cmd_writer();
	}else if (strncmp(cmdline, "alloc", 5)== 0) {
		cmd_mymem(cmdline);
	}
	else if(strcmp(cmdline,"free")==0){
		cmd_free();
	}
	else if (strcmp(cmdline, "1") == 0 && cons->sht != 0) {
		produce(cons);
	}else if (strcmp(cmdline, "2") == 0 && cons->sht != 0) {
		consume(cons);
	}else if (strcmp(cmdline, "add") == 0 && cons->sht != 0) {
		shareadd(cons);
	}else if (strcmp(cmdline, "rps") == 0&& cons->sht != 0)  //Rock-Scissors-Paper game
	{
		cmd_rps(cons, cmdline);
	}
	else if (strcmp(cmdline, "guess") == 0&& cons->sht != 0)  //Guess game
	{
		cmd_guess(cons, cmdline);
	}
	
	else if (cmdline[0] != 0) {
		if (cmd_app(cons, fat, cmdline) == 0) {
			/* ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½Å?????ï¿½ï¿½Aï¿?Aï¿?vï¿½ï¿½ï¿½Å?ï¿½ï¿½??ï¿½ï¿½Aï¿½ï¿½ï¿½ï¿½??ï¿?sï¿½Å?ï¿½ï¿½??ï¿? */
			cons_putstr0(cons, "Bad command.\n\n");
		}
	}
	return;
}

/**
 * command: change directory
 * cmdline = cd .. -> change parent directory
 * cmdline = cd [dir name] -> change [dir name] directory if it exists.
 */
void cmd_cd(struct CONSOLE *cons, char *cmdline){
	struct MYDIRINFO *dest_dinfo;
	char *filename = cmdline + 3;
	char *cdline = filename;

	dest_dinfo = parse_cdline(cons, cdline);

	if(dest_dinfo != 0){
		/* ï¿?\ï¿½ï¿½ï¿½ï¿½????ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½ï¿?Aï¿½Ú?Iï¿?nï¿½É???ï¿½ï¿½ï¿½ï¿½ï¿? */
		cons->current_dir = dest_dinfo;
	}
	return;
}

/**
 * cdï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½É?^ï¿½ï¿½ï¿½ï¿½??½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Í?ï¿½ï¿½ï¿?(ï¿½ï¿½ï¿½İ?Jï¿½ï¿½ï¿½ï¿½)
 * @param cons: console addr
 * @param cdline: characters given to cd command
 */
#define isFILENAME() (('A'<=cdline[cp] && cdline[cp]<='Z') || ('a'<=cdline[cp] && cdline[cp]<='z') || ('0'<=cdline[cp] && cdline[cp]<='9'))
#define isDOUBLEPOINT() ((cdline[cp] == '.') && (cdline[cp+1] == '.'))
int cp;

void get_dirname(char *dirname, char *cdline){
	int p;

	p=0;
	while(isFILENAME()){
		dirname[p] = cdline[cp];
		p++; cp++;
	}
	dirname[p] = '\0';
	return;
}

/* cdï¿½Ì?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Í?ï¿½ï¿½ï¿½Ö?ï¿?*/
struct MYDIRINFO *parse_cdline(struct CONSOLE *cons, char *cdline){
	struct MYDIRINFO *dinfo;
	char s[100];
	char prev_pname[100];	//debug
	char pname[100];		//debug
	char dirname[MAX_NAME_LENGTH];
	int loop_count = 0;	//debug
	struct MYFILEINFO *finfo;

	debug_print("*****IN FUNC: parse_cdline()*****\n");

	dinfo = 0;
	cp = 0;
	sprintf(prev_pname, "N/A");

	while(cdline[cp] != '\0'){
		if(cp != 0){
			get_pathname(prev_pname, dinfo);
			loop_count++;
		}

		if(isFILENAME()){
			if(cp == 0){
				debug_print("filename has found.\n");
				/*ï¿½ï¿½ï¿½Î?pï¿?Xï¿½Æ?ï¿½ï¿½??ï¿½ï¿½ï¿?*/
				sprintf(s, "change directory using relative path\n");
				debug_print(s);
				dinfo = cons->current_dir;
				get_pathname(prev_pname, dinfo); //debug
			}
		}else if(cdline[cp] == '/'){
			debug_print("'/' has found.\n");
			cp++;
			if(cp == 1){
				/*ï¿½ï¿½??pï¿?Xï¿½Æ?ï¿½ï¿½??ï¿½ï¿½ï¿?*/
				sprintf(s, "change directory using absolute path\n");
				debug_print(s);
				dinfo = (struct MYDIRINFO *)ROOT_DIR_ADDR;
			}
		}else if(isDOUBLEPOINT()){
			debug_print("\"..\" has found.\n");
			cp += 2;
			if(cp == 2){
				/*ï¿½ï¿½ï¿½Î?pï¿?Xï¿½Æ?ï¿½ï¿½??ï¿½ï¿½ï¿?*/
				sprintf(s, "change directory using relative path\n");
				debug_print(s);
				dinfo = cons->current_dir;
				get_pathname(prev_pname, dinfo);
			}

			if(dinfo->parent_dir == 0){
				cd_error(cons, "Can't move because here is ROOT directory.\n");
				debug_print("*********************************\n");
				return 0; // parseï¿½ï¿½ï¿?s
			}
			dinfo = dinfo->parent_dir;
			goto PARSE_NEXT;
		}else{
			/*ï¿?Gï¿½ï¿½ï¿?[ï¿½ï¿½ï¿½ï¿½*/
			cd_error(cons, "Incorrect initial character.\n");
			debug_print("*********************************\n");
			return 0;	// parseï¿½ï¿½ï¿?s
		}

		if(isDOUBLEPOINT() || cdline[cp] == '\0'){
			// ".."ï¿½Ü?ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½kï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½È???½ï¿½ï¿½ï¿½ï¿½ï¿½??ï¿?
		}else{
			/* ï¿?wï¿½è?³ï¿½??½ï¿½fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½ï¿½ï¿½Tï¿½ï¿½ */
			get_dirname(dirname, cdline);
			finfo = myfinfo_search(dirname, dinfo, MAX_FINFO_NUM);
			if(finfo == 0){
				/* ï¿?Yï¿½ï¿½ï¿½ï¿½ï¿½ï¿½fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Â?ï¿½ï¿½ï¿½È?ï¿½ï¿½ï¿½ï¿½ï¿? */
				cd_error(cons, "Can't find this directory.\n");
				debug_print("*********************************\n");
				return 0; // parseï¿½ï¿½ï¿?s
			}else{
				/* ï¿?Yï¿½ï¿½ï¿½ï¿½ï¿½ï¿½fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Â?ï¿½ï¿½ï¿½ï¿½ï¿? */
				dinfo = finfo->child_dir;
			}
		}

		PARSE_NEXT:
		get_pathname(pname, dinfo);
		sprintf(s, "[%d]change directory: %s -> %s\n", loop_count, prev_pname, pname);
		debug_print(s);
	}

	get_pathname(prev_pname, cons->current_dir);
	get_pathname(pname, dinfo);
	sprintf(s, "[RESULT]destination: %s -> %s\n", prev_pname, pname);
	debug_print(s);

	/*ï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½ï¿½Ì???ï¿?*/
	//cons->current_dir = dinfo;
	debug_print("*********************************\n");
	cons_newline(cons);
	return dinfo;
}

/* ï¿?Gï¿½ï¿½ï¿?[ï¿½Ì?oï¿½ï¿½ */
void cd_error(struct CONSOLE *cons, char *message){
	char s[50];
	int i, j, k;

	get_pathname(s, cons->current_dir);
	for(i=0; s[i]!='\0'; i++)s[i] = ' ';	// ï¿?pï¿?Xï¿½ï¿½ï¿½ï¿½ï¿½Ì?ï¿?
	for(j=0; j<3; j++) s[i+j] = ' ';		// "cd "ï¿½ï¿½ï¿½Ì?ï¿?
	for(k=0; k<MAX_CMDLINE; k++){
		if(k<cp){
			s[i+j+k] = ' ';
		}else{
			s[i+j+k] = '^';
			s[i+j+k+1] = '\n';
			s[i+j+k+2] = '\0';
			break;
		}
	}
	cons_putstr(cons, s);
	cons_putstr(cons, message);
	cons_newline(cons);
	return;
}


/* ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½ï¿½ï¿?Cï¿½ï¿½ï¿½ï¿½????Pï¿½È?tï¿?@ï¿?Cï¿½ï¿½ï¿½Ò?Wï¿½ï¿½ï¿?sï¿½ï¿½ï¿½ï¿½??ï¿?
 * (myfopen/myfread/myfwrite/myfcloseï¿½Ö?ï¿½ï¿½??eï¿?Xï¿?gï¿?p) */
/* ï¿½Ò?Wï¿½ï¿½ï¿?[ï¿?h */
#define MODE_DEF	0x00
#define MODE_CLS	0x01
#define MODE_INS	0x02
#define MODE_ADD	0x04
#define MODE_OPEN	0x08
#define MODE_ALL	0xFF
void cmd_edit(struct CONSOLE *cons, char *cmdline){
	struct MYDIRINFO *dinfo = cons->current_dir;	// open»ñÈ¡µ±Ç°Â·¾¶
	struct MYFILEDATA *fdata;	
	int i, p;
	char s[BODY_SIZE + BODY_SIZE_OFFSET];
	char option[15];
	char editline[50];
	int length_editline;
	unsigned int file_size;
	unsigned char mode = MODE_DEF;
	int temp_p;
	char temp_body[500];

	//sprintf(s, "cmdline = %s\n", cmdline);
	//debug_print(s);

	/* initialize */
	sprintf(option, "");
	sprintf(editline, "");

	/* command line parser */
	p = 5;
	while(cmdline[p] == ' ') p++; // »ñÈ¡ºóÃæµÄ¿ÚÁî

	if(cmdline[p] == '-'){
		/* option*/
		temp_p=0;
		p++; 
		while(cmdline[p] != ' ' && cmdline[p] != 0){
			if(temp_p >= 10){
				/* option too long */
				sprintf(s, "option is too long.\n");
				cons_putstr(cons, s);
				cons_newline(cons);
				return;
			}
			option[temp_p] = cmdline[p];
			temp_p++;
			p++;
		}

		option[temp_p] = '\0';	

		//sprintf(s, "option=%s[EOF]\n", option);
		//debug_print(s);

		/* OPEN MODE */
		if(strcmp(option, "open") == 0){
			/*´ò¿ªÎÄ¼ş */
			debug_print("EDIT:open mode\n");
			mode = MODE_OPEN;
		}

		if(setfdata == 0 && mode != MODE_OPEN){
			/* µ±Ç°»¹Ã»ÓĞÎÄ¼ş±»´ò¿ª */
			sprintf(s, "can't edit: There is no file being opened.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}

		if(strcmp(option, "cls") == 0){
			/* Çå³ıÄ£Ê½ */
			debug_print("EDIT:clear mode\n");
			mode = MODE_CLS;
			myfwrite(setfdata, "");	// ¸³Öµ¿ÕµÄ×Ö·û´®
		}else if(strcmp(option, "ins") == 0){
			/* ´ÓÍ·²åÈë×Ö·û´®*/
			debug_print("EDIT:insert mode\n");
			mode = MODE_INS;
		}else if(strcmp(option, "add") == 0){
			/* ×·¼Ó×Ö·û´®*/
			debug_print("EDIT:add mode\n");
			mode = MODE_ADD;
		}else if(strcmp(option, "show") == 0){
			debug_print("EDIT:show mode\n");
			myfread(temp_body, setfdata);
			sprintf(s, "setfdata->head.stat=0x%02x\n", setfdata->head.stat);
			debug_print(s);
			sprintf(s, "%s[EOF]\n", temp_body);
			cons_putstr(cons, s);
			sprintf(s, "size: %d[byte]\n", get_size_myfdata(setfdata));
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}else if(strcmp(option, "close") == 0){
			debug_print("EDIT:close mode\n");
			myfclose(setfdata);
			setfdata = 0;
			sprintf(s, "opened file was closed.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}else if(strcmp(option, "save") == 0){
			//cons_putstr(cons, "enter myfsave!");
			if(myfsave(setfdata) == -1){
				sprintf(s, "Can't save because of error in myfinfo_search() in myfsave()\n");
				cons_putstr(cons, s);
				cons_newline(cons);
			}else{
				sprintf(s, "finished saving opened file.\n");
				cons_putstr(cons, s);
				cons_newline(cons);
				return;
			}
		}else if(strcmp(option, "open") == 0){
			
		}else{
			/*ÃüÁîÊäÈë´íÎó*/
			sprintf(s, "invalid option.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}
		if(cmdline[p] == ' ') p++; 

	}else if(setfdata == 0 && mode != MODE_OPEN){

		sprintf(s, "Can't edit: There is no file being opened.\n");
		cons_putstr(cons, s);
		cons_newline(cons);
		return;
	}

	/* edit line*/
	temp_p = 0;
	while(cmdline[p] != 0 && cmdline[p] != '\0'){
		if(temp_p >=48){
			/*editºóÃæÊäÈëµÄ×Ö·û´®Ì«³¤*/
			sprintf(s, "edit line is too long.");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}
		editline[temp_p] = cmdline[p];
		temp_p++;
		p++;
	}
	length_editline = temp_p;
	editline[temp_p] = '\0';	
	/**** end of parser ****/

	char temp[1024];	
	if(mode == MODE_DEF || mode == MODE_ADD){
		/* Èô²»ÊäÈë-ins»ò-add£¬Ä¬ÈÏÎª×·¼ÓÄ£Ê½ */
		myfread(s, setfdata);
		strcat(s, editline);
		debug_print("char *s out of myfwrite() is shown below.\n");
		debug_print(s);
		debug_print("\n");
		myfwrite(setfdata, s);

	}else if(mode == MODE_CLS){
		/* CLEAR SCREEN MODE*/
		myfread(temp, setfdata);
		sprintf(temp, "");
		strcat(temp, editline);
		myfwrite(setfdata, temp);	

	}else if(mode == MODE_INS){
		int nullFlag = 0;
		/* myfwrite*/
		myfread(temp, setfdata);
		for(i=0; i<length_editline; i++){
			if(editline[i] != ' '){
				
				temp[i] = editline[i];
			}else if(temp[i] == '\0'){
				temp[i] = ' ';
				nullFlag = 1;
			}
		}
		if(nullFlag == 1)temp[i] = '\0';	
		sprintf(s, "INS MODE RESULT: %s\n", temp);
		debug_print(s);
		myfwrite(setfdata, temp);

	}else if(mode == MODE_OPEN){
		/*´ò¿ªÎÄ¼ş*/
		if(setfdata != 0){
			sprintf(s, "can't open: There is a file being opened.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}
		fdata = myfopen(editline, dinfo);
		if(fdata == 0){
			sprintf(s, "can't open \"%s\".\n", editline);
			cons_putstr(cons, s);
			cons_newline(cons);

		}else{
			sprintf(s, "opened \"%s\" file.\n", editline);
			cons_putstr(cons, s);
			cons_newline(cons);
			setfdata = fdata;
		}
		return;
	}else{
		debug_print("unexpected error at edit mode.\n");
	}

	myfread(temp_body, setfdata);
	sprintf(s, "edit result:\n%s[EOF]\n", temp_body);
	cons_putstr(cons, s);

	/* »ñÈ¡ÎÄ¼ş´óĞ¡*/
	file_size = get_size_myfdata(setfdata);
	sprintf(s, "size: %d[byte]\n", file_size);
	cons_putstr(cons, s);
	cons_newline(cons);

	return;
}

/**
 * make directory in my filesystem
 */
void cmd_mkdir(struct CONSOLE *cons, char *cmdline){
	int i, j;
	char s[50];
	char *dir_name;
	struct MYDIRINFO *dinfo = cons->current_dir;
	struct MYFILEINFO *finfo; // for debug

	dir_name = cmdline + 6;
	for(i=0; dir_name[i] != 0; i++);
	if(i > 8){
		sprintf(s, "directory name shoule be within 8 letters.\n");
		cons_putstr(cons, s);
		cons_newline(cons);
		return;
	}else if((finfo = myfinfo_search(dir_name, dinfo, MAX_FINFO_NUM)) != 0){
		sprintf(s, "this directory name is already used, please use other name.\n");
		cons_putstr(cons, s);

		sprintf(s, "\tname = %s\n\text = %s\n\ttype = 0x%02x\n", finfo->name, finfo->ext, finfo->type);
		debug_print(s);
		cons_newline(cons);
		return;
	}

	/* ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½Ì???ï¿½ï¿½ï¿½Ü?ï¿?iï¿½ï¿½iï¿½ß?ï¿? */
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		/* ï¿½ï¿½ï¿½ï¿½Fileï¿½ï¿½ï¿½ï¿½ï¿½ï¿½????¶ï¿½??ï¿½ï¿½??ï¿½ï¿½???, breakï¿½ï¿½ï¿½ï¿½ */
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}
	}

	/* ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Í???¶ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿? */
	for(j=0; dir_name[j] != 0; j++) {
		if ('a' <= dir_name[j] && dir_name[j] <= 'z') {
			dir_name[j] -= 'a'-'A';
		}
		// dir_nameï¿½Ì?ï¿½ï¿½ï¿½ï¿½????ï¿½ï¿½??¶ï¿½ï¿½ï¿½ï¿½ï¿½??iï¿?[ï¿½ï¿½ï¿½ï¿½
		dinfo->finfo[i].name[j] = dir_name[j];
	}

	// ï¿?cï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ó?????ï¿½ï¿½??ï¿?
	for(; j<8 ;j++) dinfo->finfo[i].name[j] = ' ';

	// dir_nameï¿½Ì?ï¿½ï¿½ï¿½ï¿½????ï¿½ï¿½??¶ï¿½ï¿½ï¿½ï¿½ï¿½??iï¿?[ï¿½ï¿½ï¿½ï¿½
	//for(j=0; dir_name[j] != 0; j++) dinfo->finfo[i].name[j] = dir_name[j];

	/* ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½Ì?Vï¿?Kï¿½ì?? */
	dinfo->finfo[i].child_dir = get_newdinfo();	// ï¿½ï¿½ï¿½ï¿½mydirï¿½Ì????ï¿½ï¿½ï¿?iï¿?[
	dinfo->finfo[i].clustno = 0;
	dinfo->finfo[i].date = 0;
	dinfo->finfo[i].type = 0x10;
	dinfo->finfo[i].size = sizeof(dinfo->finfo[i]);
	dinfo->finfo[i].fdata = 0;	// ï¿?Oï¿?Fï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?fï¿?[ï¿?^ï¿½ï¿½ï¿½ï¿½ï¿½İ?ï¿½ï¿½??ï¿?

	/* ï¿½ì??ï¿½ï¿½ï¿½ï¿½ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½Ì?oï¿½ï¿½ */
	sprintf(s, "created directory: name = %s\n", dinfo->finfo[i].name);
	cons_putstr(cons, s);
	sprintf(s, "\tchild dir address = 0x%08x\n", dinfo->finfo[i].child_dir);
	debug_print(s);
	sprintf(s, "\ttype=0x%02x\n", dinfo->finfo[i].type);
	debug_print(s);

	/* ï¿?qï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½ï¿½ï¿½MYDIRINFOï¿½Å?}ï¿?Eï¿½ï¿½ï¿?gï¿½ï¿½ï¿½ï¿½B(dinfoï¿½ï¿½child_dinfoï¿½Ì?eï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½) */
	struct MYDIRINFO *child_dinfo = (struct MYDIRINFO *)dinfo->finfo[i].child_dir;
	child_dinfo->this_dir = dinfo->finfo[i].child_dir; // ï¿?qï¿½ï¿½addr ï¿½ï¿½ ï¿?eï¿½ï¿½dir addr
	sprintf(child_dinfo->name, dir_name); // ï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?iï¿?[
	child_dinfo->parent_dir = dinfo->this_dir; // ï¿?qï¿½Ì?eaddr ï¿½ï¿½ ï¿?eaddr

	/* ï¿?qï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½ï¿½ï¿½ï¿½Ì?oï¿½ï¿½(ï¿?fï¿?oï¿?bï¿?Oï¿?p) */
	sprintf(s, "\tchild dinfo addr = 0x%08x\n", child_dinfo->this_dir);
	debug_print(s);
	sprintf(s, "\tchild dinfo name = %s\n", child_dinfo->name);
	debug_print(s);
	sprintf(s, "\tchild dinfo parent addr = 0x%08x\n", child_dinfo->parent_dir);
	debug_print(s);

	cons_newline(cons);	// ï¿½ï¿½ï¿?s
	return;
}

/**
 * make file in my filesystem
 */
void cmd_mkfile(struct CONSOLE *cons, char *cmdline){
	/* (0x0010 + 0x0026)ÊÇÎÄ¼şÄ¿Â¼ÆğÊ¼Î»ÖÃ*/
	struct MYDIRINFO *dinfo = cons->current_dir;
	int i, j;
	char s[50];
	char *name = cmdline + 7;
	char filename[12];
	struct MYFILEINFO *finfo;


	for (j = 0; j < 11; j++) {
		filename[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) {
			sprintf(s, "file name should be within 8 letters,\n");
			cons_putstr(cons, s);
			sprintf(s, "and extension should be within 3 letters.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			filename[j] = name[i];
			if ('a' <= filename[j] && filename[j] <= 'z') {
				/*×ª»»³É´óĞ´ */
				filename[j] -= 'a'-'A';
			}
			j++;
		}
	}

	if((finfo = myfinfo_search(filename, dinfo, MAX_FINFO_NUM)) != 0){
		sprintf(s, "this file name is already used, please use other name.\n");
		cons_putstr(cons, s);
		///* debug code: Viewing already used file name
		sprintf(s, "\tname = %s\n\text = %s\n\ttype = 0x%02x\n", finfo->name, finfo->ext, finfo->type);
		cons_putstr(cons, s);
		cons_newline(cons);
		//*/
		cons_newline(cons);
		return;
	}

	/***** ³õÊ¼»¯ *****/
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}
	}

	for(j=0; j<8; j++) dinfo->finfo[i].name[j] = filename[j];
	for(j=0; j<3; j++) dinfo->finfo[i].ext[j] = filename[8+j];
	dinfo->finfo[i].clustno = 0;
	dinfo->finfo[i].date = 0;
	dinfo->finfo[i].type = 0x20;	/*ÎÄ¼şÀàĞÍÊÇ0x20ËµÃ÷ÎªÕı³£ÎÄ¼ş*/
	dinfo->finfo[i].size = 0;

	struct MYFILEDATA test;	//ĞÂ½¨Ò»¸öÎÄ¼ştest
	test.head.stat = STAT_ALL;	
	test.head.this_dir = dinfo->this_dir;//»ñÈ¡µ±Ç°Â·¾¶
	struct MYFILEDATA *fdata = get_newfdata(&test);
	strcpy(fdata->head.name, name);	

	/***** debug *****/
	sprintf(s, "fdata->head.name =\t\t%s[EOF]\n", fdata->head.name);
	debug_print(s);
	sprintf(s, "dinfo->finfo[i].name =\t%s[EOF]\n", dinfo->finfo[i].name);
	debug_print(s);
	struct MYFILEINFO *debug = myfinfo_search(fdata->head.name, dinfo, MAX_FINFO_NUM);
	sprintf(s, "test(should not be 0) = %d\n", debug);
	debug_print(s);
	/*****************/
	fdata->head.stat = 0x01;	// valid bitï¿½ğ?????ï¿?
	fdata->head.this_fdata = fdata; // ï¿?fï¿?[ï¿?^ï¿½Ì?ï¿½Ì?ï¿½Ô?????mï¿½Û?ï¿½ï¿½ï¿?
	fdata->head.this_dir = dinfo->this_dir;
	dinfo->finfo[i].fdata = fdata;	// ï¿?fï¿?tï¿?Hï¿½ï¿½ï¿?gï¿½Í?O(ï¿?vï¿½ï¿½ï¿½ï¿½ï¿?I)

	// debug code: Viewing status of created file.
	sprintf(s, "created file name = %s[EOF]\n", filename);
	cons_putstr(cons, s);

	sprintf(s, "file name = %s\n", dinfo->finfo[i].name);
	cons_putstr(cons, s);
	sprintf(s, "file type=0x%02x\n", dinfo->finfo[i].type);
	cons_putstr(cons, s);
	cons_newline(cons);
	return;
}

/**
 * given console becomes console for log
 */
void cmd_setlog(struct CONSOLE *cons){
	char s[100];
	log = cons;
	sprintf(s, "    log cons\nmem:%d %d\nsht:%d %d\n", &log, &cons, log->sht, cons->sht);
	cons_putstr0(log, s);
	cons_newline(log);
	return;
}

/**
 * show total memory
 */
void cmd_mem(struct CONSOLE *cons, int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int i;
	char s[60];
	sprintf(s, "frees  %d\n", memman->frees);
	cons_putstr0(cons, s);
	for(i=0; i< memman->frees; i++){
		sprintf(s, "(%d) size=%dKB addr=0x%x\n", i, memman->free[i].size/1024, memman->free[i].addr);
		cons_putstr(cons, s);
	}
	sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	return;
}

void cmd_memmap(struct CONSOLE *cons, int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[60];
	sprintf(s, "total   %dMB\nfree %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	cons_putstr0(cons, s);
	return;
}

/**
 * show something in log console
 */
void cmd_log(struct CONSOLE *cons){
	char s[100];
	sprintf(s, "log test");
	cons_putstr0(log, s);
	cons_newline(log);
	return;
}

/**
 * ???
 */
void cmd_cat(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct MYFILEINFO *finfo = myfinfo_search(cmdline + 4, cons->current_dir, MAX_FINFO_NUM);
	char *p;
	char s[30];
	sprintf(s, "filename=%s\n", finfo->name);
	cons_putstr(cons, s);

	if (finfo != 0) {
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, finfo->size);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		cons_putstr0(cons, "File not found.\n");
	}
	cons_newline(cons);
	return;
}

/**
 * clear screen in console
 */
void cmd_cls(struct CONSOLE *cons)
{
	int x, y, xmax, ymax;
	struct SHEET *sheet = cons->sht;
	xmax = sheet->bxsize-16;
	ymax = sheet->bysize-37;
	for (y = 28; y < 28 + ymax; y++) {
		for (x = 8; x < 8 + xmax; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + xmax, 28 + ymax);
	cons->cur_y = 28;
	return;
}

/**
 * making large console for log
 */
void cmd_logcls(struct CONSOLE *cons){
	int x, y, xmax, ymax;
	struct SHEET *sheet = log->sht;
	xmax = sheet->bxsize-16;
	ymax = sheet->bysize-37;
	for (y = 28; y < 28 + ymax; y++) {
		for (x = 8; x < 8 + xmax; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + xmax, 28 + ymax);
	log->cur_y = 28;
	return;
}

/**
 * test function for command.
 */
void cmd_test(struct CONSOLE *cons){
	char s[100];

	sprintf(s, "BLOCK_SIZE = %d\n", BLOCK_SIZE);
	debug_print(s);
	sprintf(s, "sizeof(struct HEAD) = %d\n", sizeof(struct HEAD));
	debug_print(s);
	sprintf(s, "BODY_SIZE = BLOCK_SIZE - sizeof(struct HEAD) = %d\n", BODY_SIZE);
	debug_print(s);

	/*
	char alpha[2];
	alpha[0] = 'A';
	alpha[1] = '\0';
	while('A' <= alpha[0] && alpha[0] <= 'z'){
		sprintf(s, "alpha = %s(%d)\n", alpha, alpha[0]);
		debug_print(s);
		alpha[0]++;
	}
	 */

	cons_newline(cons);
	return;
}

/**
 * ?¼¨»ØÄêÊ¸·ïÅª¾õ?.
 * "show [file name]"
 */
void cmd_show(struct CONSOLE *cons, char *cmdline){
	char s[150];
	struct MYDIRINFO *dinfo = cons->current_dir;
	char *filename = cmdline + 5;
	struct MYFILEINFO *finfo = myfinfo_search(filename, dinfo, MAX_FINFO_NUM);

	///* for debug
	sprintf(s, "\tfinfo addr = 0x%08x\n", finfo);
	cons_putstr(cons, s);
	//*/

	if(finfo == 0){
		sprintf(s, "\t%s was not found.\n", filename);
		cons_putstr(cons, s);
		sprintf(s, "\tfinfo number was %d.\n", finfo);
		cons_putstr(cons, s);
		cons_newline(cons);
		return;

		/* if finfo was found */
	}else{
		sprintf(s, "\tname=%s[EOF]\n\text=%s[EOF]\n\tclustno=%d\n\tdate=%d\n\tsize=%d[byte]\n\ttime=%d\n\ttype=0x%02x\n",
				finfo->name,
				finfo->ext,
				finfo->clustno,
				finfo->date,
				finfo->size,
				finfo->time,
				finfo->type);
		cons_putstr(cons, s);
	}
	/* show the strings in char s[100] */
	// show detail file type info(ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½sï¿½Ì?oï¿½Í?ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿?iï¿?mï¿?Fï¿½Ï???j)
	unsigned char filetype = finfo->type;
	sprintf(s, "\tdetail file type: ");
	if((filetype & FTYPE_DIR) != 0x00)	sprintf(s, "%s directory/readonly ", s);
	if((filetype & FTYPE_FILE) != 0x00)	sprintf(s, "%s file ", s);
	if((filetype & FTYPE_SYS) != 0x00) sprintf(s, "%s system ", s);
	if((filetype & FTYPE_OTHER) != 0x00)	sprintf(s, "%s other ", s);
	sprintf(s, "%s\n", s);
	cons_putstr0(cons, s);
	cons_newline(cons);
	return;
}

/**
 * view file data including header.
 * @param cmdline is "fview [file name]"
 */
void cmd_fview(struct CONSOLE *cons, char *cmdline){
	char s[130];
	struct MYDIRINFO *dinfo = cons->current_dir;
	char *filename = cmdline + 6;
	struct MYFILEINFO *finfo = myfinfo_search(filename, dinfo, MAX_FINFO_NUM);

	///* for debug */
	sprintf(s, "finfo addr = 0x%08x\n", finfo);
	debug_print(s);
	//*/

	if(finfo == 0){
		sprintf(s, "\t%s was not found.\n", filename);
		cons_putstr(cons, s);
		sprintf(s, "\tfinfo number was %d.\n", finfo);
		cons_putstr(cons, s);
		cons_newline(cons);
		return;

		/* if finfo was found */
	}else{
		if(finfo->fdata != 0){
			/* ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?fï¿?[ï¿?^ï¿½ï¿½ï¿½ï¿½ï¿½İ?ï¿½ï¿½ï¿½ï¿½??? */
			sprintf(s, "fdata addr = 0x%08x\n", finfo->fdata);
			debug_print(s);
			sprintf(s, "head.data_addr=0x%08x\n", finfo->fdata->head.this_fdata);
			debug_print(s);
			sprintf(s, "head.dir_addr=0x%08x\n",	finfo->fdata->head.this_dir);
			debug_print(s);
			sprintf(s, "head.stat=0x%02x\n", finfo->fdata->head.stat);
			debug_print(s);

			myfread(s, finfo->fdata);
			cons_putstr(cons, s);
			cons_putstr(cons, "[EOF]\n");	// bodyï¿½ï¿½ï¿½Ì???ï¿½É???ï¿½ï¿½sï¿½ï¿½ï¿½È?ï¿½ï¿½????}ï¿½ï¿½
			sprintf(s, "size: %d\n", get_size_myfdata(finfo->fdata));
			cons_putstr(cons, s);
			cons_newline(cons);
		}else{
			/* ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?fï¿?[ï¿?^ï¿½ï¿½ï¿½È?ï¿½ï¿½???(Ex. ï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½) */
			sprintf(s, "\tthis is not file.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
		}
	}

	/* show the strings in char s[100] */
	return;
}


/* FDï¿½É?iï¿?[ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿?Sï¿½Ä???tï¿?@ï¿?Cï¿½ï¿½ï¿½ğ???ï¿?, ï¿?\ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??ï¿? */
void cmd_mkfs(struct CONSOLE * cons){
	struct MYDIRINFO dinfo;
	/* (0x0010 + 0x0026)Ê¸·ïÌÜ?µ¯»Ï°ÌÃÖ. */
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];

	sprintf(dinfo.name, "ROOT    ");
	dinfo.parent_dir = 0x00; // 0x00: root directory
	dinfo.this_dir = (struct MYDIRINFO *)ROOT_DIR_ADDR;
	cons->current_dir = (struct MYDIRINFO *)dinfo.this_dir;

	/* ï¿½Æ????ï¿½ï¿½ï¿½ï¿½10ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿?Rï¿?sï¿?[ï¿½ï¿½ï¿½Ä???ï¿? */
	for (i = 0; i < 10; i++) {
		/* ï¿½ï¿½ï¿½ï¿½Fileï¿½ï¿½ï¿½ï¿½ï¿½ï¿½????¶ï¿½??ï¿½ï¿½??ï¿½ï¿½???, breakï¿½ï¿½ï¿½ï¿½ */
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		/* 0xe5:ï¿½ï¿½ï¿½É?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??½ï¿½tï¿?@ï¿?Cï¿½ï¿½ */
		if (finfo[i].name[0] != 0xe5) {
			/* 0x18 = 0x10 + 0x18
			 * 0x10:ï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½
			 * 0x08:ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½Å?????ï¿½ï¿½ï¿½ï¿½(ï¿?fï¿?Bï¿?Xï¿?Nï¿½Ì?ï¿½ï¿½Oï¿½Æ?ï¿?)
			 * ï¿½ï¿½ï¿½ï¿½??AFile Typeï¿½ï¿½"ï¿?tï¿?@ï¿?Cï¿½ï¿½"ï¿½Å?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??? */
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j]; /* "filename"ï¿½ï¿½ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?lï¿?[ï¿½ï¿½ï¿½Å????ï¿½ï¿½ */
				}
				s[ 9] = finfo[i].ext[0];	/* "e"ï¿½ï¿½ï¿½ã??ï¿½ï¿½ */
				s[10] = finfo[i].ext[1];	/* "x"ï¿½ï¿½ï¿½ã??ï¿½ï¿½ */
				s[11] = finfo[i].ext[2];	/* "t"ï¿½ï¿½ï¿½ã??ï¿½ï¿½ */

				dinfo.finfo[i].clustno = finfo[i].clustno;
				dinfo.finfo[i].date = finfo[i].date;
				for(j=0;j<3 ;j++) dinfo.finfo[i].ext[j] = finfo[i].ext[j];
				for(j=0;j<8 ;j++) dinfo.finfo[i].name[j] = finfo[i].name[j];
				for(j=0;j<10;j++) dinfo.finfo[i].reserve[j] = finfo[i].reserve[j];
				dinfo.finfo[i].size = finfo[i].size;
				dinfo.finfo[i].time = finfo[i].time;
				dinfo.finfo[i].type = finfo[i].type;

				cons_putstr0(cons, s);
			}
		}
	}
	// memcpy(dinfo.finfo, finfo, sizeof(finfo));
	memcpy((unsigned int *)ROOT_DIR_ADDR, &dinfo, sizeof(dinfo));
	sprintf(s, "make filesystem!\n");
	cons_putstr0(cons, s);
	cons_newline(cons);
	return;
}

/**
 * show files information in new filesystem
 */
void cmd_dir(struct CONSOLE *cons){
	/* (0x0010 + 0x0026)ï¿½Ì????ï¿½É?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½, FILEINFOï¿½Ì?fï¿?[ï¿?^ï¿?\ï¿½ï¿½ï¿½Æ?ï¿½ï¿½ï¿?, ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??Rï¿?sï¿?[ï¿½ï¿½ï¿½ï¿½. */
	struct MYDIRINFO *dinfo = cons->current_dir;
	int i, j;
	char s[50];

	/* show related directory */
	sprintf(s, "directory name\t%s\n", dinfo->name);
	cons_putstr(cons, s);
	sprintf(s, "current dir addr\t0x%08x\n", dinfo->this_dir);
	debug_print(s);
	sprintf(s, "parent dir addr\t0x%08x\n", dinfo->parent_dir);
	debug_print(s);

	// search present my directory
	// int dir_num;
	// dir_num = get_newdinfo(cons);	// ï¿?fï¿?oï¿?bï¿?Oï¿½Ì?ï¿½ï¿½??ï¿?&consï¿½ï¿½ï¿½ï¿½ï¿½Ä?ï¿½ï¿½ï¿?B

	// debug code: ï¿½ï¿½ï¿½É?}ï¿?Eï¿½ï¿½ï¿?gï¿½ï¿½ï¿½ï¿½MYDIRINFOï¿½Ì???ï¿½ï¿½ï¿?\ï¿½ï¿½
	//sprintf(s, "dinfo number = %d\n", dir_num);
	//cons_putstr(cons, s);

	//display files in current dir
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		/* ï¿½ï¿½ï¿½ï¿½Fileï¿½ï¿½ï¿½ï¿½ï¿½ï¿½????¶ï¿½??ï¿½ï¿½??ï¿½ï¿½???, breakï¿½ï¿½ï¿½ï¿½ */
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}

		/* 0xe5:ï¿½ï¿½ï¿½É?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??½ï¿½tï¿?@ï¿?Cï¿½ï¿½ */
		if (dinfo->finfo[i].name[0] != 0xe5) {
			/* 0x18 = 0x10 + 0x18
			 * 0x10:ï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½
			 * 0x08:ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½Å?????ï¿½ï¿½ï¿½ï¿½(ï¿?fï¿?Bï¿?Xï¿?Nï¿½Ì?ï¿½ï¿½Oï¿½Æ?ï¿?)
			 */
			/* File Typeï¿½ï¿½"ï¿?tï¿?@ï¿?Cï¿½ï¿½"ï¿½Ì???? */
			if ((dinfo->finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext\t%7d [FILE]\n", dinfo->finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = dinfo->finfo[i].name[j]; /* "filename"ï¿½ï¿½ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?lï¿?[ï¿½ï¿½ï¿½Å????ï¿½ï¿½ */
				}
				s[ 9] = dinfo->finfo[i].ext[0];	/* "e"ï¿½ï¿½ï¿½ã??ï¿½ï¿½ */
				s[10] = dinfo->finfo[i].ext[1];	/* "x"ï¿½ï¿½ï¿½ã??ï¿½ï¿½ */
				s[11] = dinfo->finfo[i].ext[2];	/* "t"ï¿½ï¿½ï¿½ã??ï¿½ï¿½ */
				cons_putstr(cons, s);

				/* File Typeï¿½ï¿½"ï¿?fï¿?Bï¿½ï¿½ï¿?Nï¿?gï¿½ï¿½"ï¿½Ì???? */
			}else if((dinfo->finfo[i].type & 0x10) == 0x10){
				sprintf(s, "filename    \t%7d [DIR]\n", dinfo->finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = dinfo->finfo[i].name[j]; /* "filename"ï¿½ï¿½ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?lï¿?[ï¿½ï¿½ï¿½Å????ï¿½ï¿½ */
				}
				cons_putstr(cons, s);
				//sprintf(s, "test %s\t%d\t[DIR]",dinfo->finfo[i].name, dinfo->finfo[i].size);
				//cons_putstr(cons, s);
			}
		}
	}

	/* ï¿½Ğ?????ï¿½ï¿½tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Â?ï¿½ï¿½ï¿½È?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿? */
	if(i == 0){
		sprintf(s, "this directory has no file...\n");
		cons_putstr(cons, s);
	}
	cons_newline(cons);
	return;
}

/**
 * ??§æ??ä»¶ç³»ç»????å±?ç¤ºå½¢å¼?
 */
void cmd_fddir(struct CONSOLE *cons)
{

	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}

		/* 0xe5:Ç¡²ÌÊ¸·ïÌ¾ÅªÂè°ìĞ¤»ú??0xe5¡¤ÂåÉ½?Ğ¤Ê¸·ïÖá?Èï?½üÎ» */
		if (finfo[i].name[0] != 0xe5) {
			/* 0x18 = 0x10 + 0x18
			 * 0x10:ÌÜ?
			 * 0x08:¡ÄÈóÊ¸·ï¿®Â©¡ÊÈæÇ¡¼§?Ì¾¾ÎÅù¡Ë
			 * */
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j]; 
				}
				s[ 9] = finfo[i].ext[0];	//?½¼¹¡?
				s[10] = finfo[i].ext[1];	
				s[11] = finfo[i].ext[2];	
				cons_putstr0(cons, s);
				sprintf(s, "filetype=0x%02x\n", finfo[i].type);
				cons_putstr0(cons, s);
			}
		}
	}
	cons_newline(cons);
	return;
}

/**
 * ?½ü¹µÀ©Âæ
 */
void cmd_exit(struct CONSOLE *cons, int *fat)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = task_now();
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct FIFO32 *fifo = (struct FIFO32 *) *((int *) 0x0fec);
	if (cons->sht != 0) {
		timer_cancel(cons->timer);
	}
	//¾­?·ú?ãÙ¸ı?½êÀêÍÑ ÅªÆâÂ¸¶õ?Á´Éô?Êü½ĞÍè¡¤Á³¹¡?¼ûÍ×?ÊüãÙ¸ıÅª??ÏÂÇ¤??Ã?
	memman_free_4k(memman, (int) fat, 4 * 2880);
	io_cli();
	if (cons->sht != 0) {
		fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);	
	} else {
		fifo32_put(fifo, task - taskctl->tasks0 + 1024);	
	}
	io_sti();
	for (;;) {
		//Ç¤?µÙÌ²
		task_sleep(task);
	}
}

/**
 * make new thread to start new application
 */
void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal)
{
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht = open_console(shtctl, memtotal);
	struct FIFO32 *fifo = &sht->task->fifo;
	int i;
	sheet_slide(sht, 32, 4);
	sheet_updown(sht, shtctl->top);
	/*¾­Ì¿Îá¹Ô?ÆşÅª»úÉä¶úÃà»ú?À©Åş¿·ÅªÌ¿Îá¹ÔãÙ¸ıÃæ*/
	for (i = 6; cmdline[i] != 0; i++) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);	/* Enter */
	cons_newline(cons);
	return;
}


/*
show running task
*/
void cmd_top(struct CONSOLE *cons)
{
	int i, j, nameLen;
	//cons_putstr0(cons, "Task Name  ID  priority  state\n");
	struct TASKLEVEL *tl = &taskctl->level[taskctl->now_lv];
	for(i = 1; i < taskctl->runningNum; i++) {
		//char prio = taskctl->runningTasks[i]->priority + '0';
		char s[60];
		sprintf(s, "Task Name:%s\nID:%d\nPriority:%d\nState: Running\n",taskctl->runningTasks[i]->TaskName , taskctl->runningTasks[i]->sel,taskctl->runningTasks[i]->priority);
		cons_putstr0(cons, s);
		cons_putstr0(cons, "\n");
	}
	char ss[60];
	sprintf(ss,"Now, %d\tprocesses are running",i-1);
	cons_putstr0(cons, ss);
	cons_newline(cons);
	return;
}

/**
 * change language mode
 */
void cmd_langmode(struct CONSOLE *cons, char *cmdline)
{
	struct TASK *task = task_now();
	unsigned char mode = cmdline[9] - '0';
	if (mode <= 2) {
		task->langmode = mode;
	} else {
		cons_putstr0(cons, "mode number error.\n");
	}
	cons_newline(cons);
	return;
}

/**
 * ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½ï¿½ï¿?Cï¿½ï¿½ï¿½Ì?ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½ï¿?
 * APIï¿½É?ï¿½ï¿½Rï¿?[ï¿½ï¿½ï¿½Å?ï¿½ï¿½???ï¿½Ç?ï¿½ï¿½ï¿½ï¿½??????ï¿½Ö?ï¿?
 */
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	char name[18], *p, *q;
	struct TASK *task = task_now();
	int i, segsiz, datsiz, esp, dathrb, appsiz;
	struct SHTCTL *shtctl;
	struct SHEET *sht;

	/* ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½ï¿½ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½ï¿½tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½ğ???ï¿? */
	for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0; /* ï¿½Æ????ï¿½ï¿½ï¿½ï¿½ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½Ì?ï¿½ï¿½ï¿?0ï¿½É?ï¿½ï¿½ï¿? */

	/* ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½Tï¿½ï¿½ */
	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		/* ï¿½ï¿½ï¿½Â?ï¿½ï¿½ï¿½È?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½????ï¿½ï¿½ï¿?".HRB"ï¿½ï¿½ï¿½Â?ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½ï¿?xï¿?Tï¿½ï¿½ï¿½Ä???ï¿? */
		name[i    ] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	if (finfo != 0) {
		/* ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Â?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??? */
		appsiz = finfo->size;	// ï¿?Aï¿?vï¿½ï¿½ï¿?Pï¿?[ï¿?Vï¿½ï¿½ï¿½ï¿½ï¿½Ì?Tï¿?Cï¿?Yï¿½ï¿½ï¿½æ??
		p = file_loadfile2(finfo->clustno, &appsiz, fat);
		if (appsiz >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
			/* ï¿?Tï¿?Cï¿?Yï¿½ï¿½36byteï¿½È?ï¿? && Header == "Hari" && ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½Ì?ï¿½ï¿½[ï¿?hï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ê?? */
			segsiz = *((int *) (p + 0x0000));	// ï¿?Zï¿?Oï¿½ï¿½ï¿½ï¿½ï¿?gï¿½Ì?Tï¿?Cï¿?Yï¿½ï¿½ï¿½æ??
			esp    = *((int *) (p + 0x000c));	// ï¿½ï¿½ï¿?Wï¿?Xï¿?^ESPï¿½ï¿½ï¿½æ??
			datsiz = *((int *) (p + 0x0010));	// ï¿?fï¿?[ï¿?^ï¿?Tï¿?Cï¿?Yï¿½ï¿½ï¿½æ??
			dathrb = *((int *) (p + 0x0014));	// ï¿?fï¿?[ï¿?^ï¿½Ì?ï¿½ï¿½eï¿½ï¿½ï¿½æ??
			q = (char *) memman_alloc_4k(memman, segsiz);	// ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ì?ï¿½ï¿½ï¿?mï¿½ï¿½
			task->ds_base = (int) q;	// task
			set_segmdesc(task->ldt + 0, appsiz - 1, (int) p, AR_CODE32_ER + 0x60);
			set_segmdesc(task->ldt + 1, segsiz - 1, (int) q, AR_DATA32_RW + 0x60);
			for (i = 0; i < datsiz; i++) {
				q[esp + i] = p[dathrb + i];
			}
			start_app(0x1b, 0 * 8 + 4, esp, 1 * 8 + 4, &(task->tss.esp0));
			shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
			for (i = 0; i < MAX_SHEETS; i++) {
				sht = &(shtctl->sheets0[i]);
				if ((sht->flags & 0x11) == 0x11 && sht->task == task) {
					/* ï¿?Aï¿?vï¿½ï¿½ï¿½ï¿½ï¿?Jï¿½ï¿½ï¿½ï¿½ï¿½Ï???ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½????ï¿? */
					sheet_free(sht);	/* ï¿½Â?ï¿½ï¿½ï¿? */
				}
			}
			for (i = 0; i < 8; i++) {	/* ï¿?Nï¿½ï¿½ï¿?[ï¿?Yï¿½ï¿½ï¿½Ä???ï¿½ï¿½tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿?Nï¿½ï¿½ï¿?[ï¿?Y */
				if (task->fhandle[i].buf != 0) {
					memman_free_4k(memman, (int) task->fhandle[i].buf, task->fhandle[i].size);
					task->fhandle[i].buf = 0;
				}
			}
			timer_cancelall(&task->fifo);
			memman_free_4k(memman, (int) q, segsiz);
			task->langbyte1 = 0;
		} else {
			cons_putstr0(cons, ".hrb file format error.\n");
		}
		memman_free_4k(memman, (int) p, appsiz);
		cons_newline(cons);
		return 1;
	}
	/* ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Â?ï¿½ï¿½ï¿½È?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??? */
	return 0;
}

/**
 * use haribote api
 * @param edx defines type of command
 */
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct TASK *task = task_now();
	int ds_base = task->ds_base;
	struct CONSOLE *cons = task->cons;
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht;
	struct FIFO32 *sys_fifo = (struct FIFO32 *) *((int *) 0x0fec);
	int *reg = &eax + 1;	/* eaxï¿½Ì?ï¿½ï¿½????n */
	/* ï¿½Û?ï¿½ï¿½??ï¿½ï¿½??ï¿?PUSHADï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½É?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿? */
	/* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
	/* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */
	int i;
	struct FILEINFO *finfo;
	struct FILEHANDLE *fh;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	switch(edx){
	case 1:
		cons_putchar(cons, eax & 0xff, 1);
		break;
	case 2:
		cons_putstr0(cons, (char *) ebx + ds_base);
		break;
	case 3:
		cons_putstr1(cons, (char *) ebx + ds_base, ecx);
		break;
	case 4:
		return &(task->tss.esp0);
		break;
	case 5:
		sht = sheet_alloc(shtctl);
		sht->task = task;
		sht->flags |= 0x10;
		sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
		make_window8((char *) ebx + ds_base, esi, edi, (char *) ecx + ds_base, 0);
		sheet_slide(sht, ((shtctl->xsize - esi) / 2) & ~3, (shtctl->ysize - edi) / 2);
		sheet_updown(sht, shtctl->top); /* ï¿½ï¿½ï¿½Ì?}ï¿?Eï¿?Xï¿½Æ?ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½????ï¿½æ?¤ï¿½??wï¿½ï¿½F ï¿?}ï¿?Eï¿?Xï¿½Í?ï¿½ï¿½??ï¿½É???ï¿? */
		reg[7] = (int) sht;
		break;
	case 6:
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *) ebp + ds_base);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
		}
		break;
	case 7:
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
		break;
	case 8:
		memman_init((struct MEMMAN *) (ebx + ds_base));
		ecx &= 0xfffffff0;	/* 16ï¿?oï¿?Cï¿?gï¿?Pï¿½Ê?ï¿? */
		memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
		break;
	case 9:
		ecx = (ecx + 0x0f) & 0xfffffff0; /* 16ï¿?oï¿?Cï¿?gï¿?Pï¿½Ê?????ï¿½ã?? */
		reg[7] = memman_alloc((struct MEMMAN *) (ebx + ds_base), ecx);
		break;
	case 10:
		ecx = (ecx + 0x0f) & 0xfffffff0; /* 16ï¿?oï¿?Cï¿?gï¿?Pï¿½Ê?????ï¿½ã?? */
		memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
		break;
	case 11:
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		sht->buf[sht->bxsize * edi + esi] = eax;
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
		}
		break;
	case 12:
		sht = (struct SHEET *) ebx;
		sheet_refresh(sht, eax, ecx, esi, edi);
		break;
	case 13:
		sht = (struct SHEET *) (ebx & 0xfffffffe);
		hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
		if ((ebx & 1) == 0) {
			if (eax > esi) {
				i = eax;
				eax = esi;
				esi = i;
			}
			if (ecx > edi) {
				i = ecx;
				ecx = edi;
				edi = i;
			}
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
		break;
	case 14:
		sheet_free((struct SHEET *) ebx);
		break;
	case 15:
		for (;;) {
			io_cli();
			if (fifo32_status(&task->fifo) == 0) {
				if (eax != 0) {
					task_sleep(task);	/* FIFOï¿½ï¿½ï¿½ï¿½??????Qï¿½Ä???ï¿? */
				} else {
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons->sht != 0) { /* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿?pï¿?^ï¿?Cï¿?} */
				/* ï¿?Aï¿?vï¿½ï¿½ï¿½ï¿½ï¿?sï¿½ï¿½ï¿½Í?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½ï¿?oï¿½È?ï¿½ï¿½????Aï¿½ï¿½ï¿½Â?ï¿½ï¿½ï¿½ï¿½??\ï¿½ï¿½ï¿?pï¿½ï¿½1ï¿½ğ???ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿? */
				timer_init(cons->timer, &task->fifo, 1); /* ï¿½ï¿½ï¿½ï¿½1ï¿½ï¿½ */
				timer_settime(cons->timer, 50);
			}
			if (i == 2) {	/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ON */
				cons->cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½OFF */
				cons->cur_c = -1;
			}
			if (i == 4) {	/* ï¿?Rï¿½ï¿½ï¿?\ï¿?[ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿? */
				timer_cancel(cons->timer);
				io_cli();
				fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024);	/* 2024ï¿?`2279 */
				cons->sht = 0;
				io_sti();
			}
			if (i >= 256) { /* ï¿?Lï¿?[ï¿?{ï¿?[ï¿?hï¿?fï¿?[ï¿?^ï¿?iï¿?^ï¿?Xï¿?NAï¿?oï¿?Rï¿?jï¿½È?ï¿? */
				reg[7] = i - 256;
				return 0;
			}
		}
		break;
	case 16:
		reg[7] = (int) timer_alloc();
		((struct TIMER *) reg[7])->flags2 = 1;	/* ï¿½ï¿½ï¿½ï¿½ï¿?Lï¿½ï¿½ï¿½ï¿½ï¿?Zï¿½ï¿½ï¿?Lï¿½ï¿½ */
		break;
	case 17:
		timer_init((struct TIMER *) ebx, &task->fifo, eax + 256);
		break;
	case 18:
		timer_settime((struct TIMER *) ebx, eax);
		break;
	case 19:
		timer_free((struct TIMER *) ebx);
		break;
	case 20:
		if (eax == 0) {
			i = io_in8(0x61);
			io_out8(0x61, i & 0x0d);
		} else {
			i = 1193180000 / eax;
			io_out8(0x43, 0xb6);
			io_out8(0x42, i & 0xff);
			io_out8(0x42, i >> 8);
			i = io_in8(0x61);
			io_out8(0x61, (i | 0x03) & 0x0f);
		}

		break;

	case 21:
		/* int api_fopen(char *fname):ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½Ì?Iï¿?[ï¿?vï¿½ï¿½
		 * EDX = 21
		 * EBX = file name
		 * EAX = file handle (return 0 if file open failed.)
		 */
		for (i = 0; i < 8; i++) {
			if (task->fhandle[i].buf == 0) {
				break;
			}
		}
		fh = &task->fhandle[i];
		reg[7] = 0;
		if (i < 8) {
			finfo = file_search((char *) ebx + ds_base,
					(struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
			if (finfo != 0) {
				reg[7] = (int) fh;
				fh->size = finfo->size;
				fh->pos = 0;
				fh->buf = file_loadfile2(finfo->clustno, &fh->size, task->fat);
			}
		}
		break;
	case 22:
		/* void api_fclose(int fhandle): ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½Ì?Nï¿½ï¿½ï¿?[ï¿?Y
		 * EDX = 22
		 * EAX = file handle
		 */
		fh = (struct FILEHANDLE *) eax;
		memman_free_4k(memman, (int) fh->buf, fh->size);
		fh->buf = 0;
		break;
	case 23:
		/* api_fseek(int fhandle, int offset, int mode):ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿½Ì?Vï¿?[ï¿?N
		 * EDX = 23
		 * EAX = file handle
		 * ECX = seek mode
		 * 		0:ï¿?Vï¿?[ï¿?Nï¿½Ì?ï¿½ï¿½_ï¿½Í?tï¿?@ï¿?Cï¿½ï¿½ï¿½Ì????
		 * 		1:ï¿?Vï¿?[ï¿?Nï¿½Ì?ï¿½ï¿½_ï¿½Í?ï¿½ï¿½????Aï¿?Nï¿?Zï¿?Xï¿½Ê?u
		 * 		2:ï¿?Vï¿?[ï¿?Nï¿½Ì?ï¿½ï¿½_ï¿½Í?tï¿?@ï¿?Cï¿½ï¿½ï¿½ï¿½sï¿?Iï¿?[
		 * EBX = ï¿?Vï¿?[ï¿?Nï¿½ï¿½
		 */
		fh = (struct FILEHANDLE *) eax;
		if (ecx == 0) {
			fh->pos = ebx;
		} else if (ecx == 1) {
			fh->pos += ebx;
		} else if (ecx == 2) {
			fh->pos = fh->size + ebx;
		}
		if (fh->pos < 0) {
			fh->pos = 0;
		}
		if (fh->pos > fh->size) {
			fh->pos = fh->size;
		}
		break;
	case 24:
		/* int api_fsize(int fhandle, int mode): ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?Tï¿?Cï¿?Yï¿½Ì????
		 * EDX = 24
		 * EAX = filehandle
		 * ECX = ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?Tï¿?Cï¿?Yï¿½æ?¾ï¿½ï¿½ï¿½[ï¿?h
		 * 		0:ï¿½ï¿½ï¿½Ê???tï¿?@ï¿?Cï¿½ï¿½ï¿?Tï¿?Cï¿?Y
		 * 		1:ï¿½ï¿½ï¿½İ???????ï¿½ï¿½????uï¿½Í?tï¿?@ï¿?Cï¿½ï¿½ï¿½í??ï¿½ï¿½ï¿½ç?½ï¿½oï¿?Cï¿?gï¿½Ú?ï¿?
		 * 		2:ï¿?tï¿?@ï¿?Cï¿½ï¿½ï¿?Iï¿?[ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½????uï¿½Ü?????oï¿?Cï¿?gï¿½ï¿½
		 * EAX = return file size
		 */
		fh = (struct FILEHANDLE *) eax;
		if (ecx == 0) {
			reg[7] = fh->size;
		} else if (ecx == 1) {
			reg[7] = fh->pos;
		} else if (ecx == 2) {
			reg[7] = fh->pos - fh->size;
		}
		break;
	case 25:
		/* int api_fread():
		 * EDX = 25
		 * EAX = file handle
		 * EBX = ï¿?oï¿?bï¿?tï¿?@ï¿½Ì???n
		 * ECX = ï¿½Å?ï¿½Ç???ï¿½ï¿½??oï¿?Cï¿?gï¿½ï¿½
		 * EAX = return ï¿½ï¿½ï¿½ï¿½????ï¿½ï¿½??ï¿½ï¿½oï¿?Cï¿?gï¿½ï¿½ */
		fh = (struct FILEHANDLE *) eax;
		for (i = 0; i < ecx; i++) {
			if (fh->pos == fh->size) {
				break;
			}
			*((char *) ebx + ds_base + i) = fh->buf[fh->pos];
			fh->pos++;
		}
		reg[7] = i;
		break;
	case 26:
		/* int api_cmdline(char *buf, int maxsize):ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½ï¿½ï¿?Cï¿½ï¿½ï¿½Ì????
		 * EDX = 26
		 * EBX = ï¿?Rï¿?}ï¿½ï¿½ï¿?hï¿½ï¿½ï¿?Cï¿½ï¿½ï¿½ï¿½ï¿?iï¿?[ï¿½ï¿½ï¿½ï¿½??n
		 * ECX = ï¿½ï¿½ï¿?oï¿?Cï¿?gï¿½Ü???iï¿?[ï¿½Å?ï¿½ï¿½???
		 * EAX = return ï¿½ï¿½ï¿?oï¿?Cï¿?gï¿?iï¿?[ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
		i = 0;
		for (;;) {
			*((char *) ebx + ds_base + i) =  task->cmdline[i];
			if (task->cmdline[i] == 0) {
				break;
			}
			if (i >= ecx) {
				break;
			}
			i++;
		}
		reg[7] = i;
		break;
	case 27:
		/* inr api_getlang(void): langmodeï¿½Ì????
		 * EDX = 27
		 * EAX = return langmode;
		 */
		reg[7] = task->langmode;
		break;
case 28:
	    {
	    gdt_addr = ADR_GDT;
	    ldt_addr_offset = task->tss.ldtr;
	    temp =  *(struct SEGMENT_DESCRIPTOR *)(ADR_GDT + ldt_addr_offset);
	    ldt_addr = temp.base_low +(temp.base_mid << 16) + (temp.base_high<<24);
	    temp = *(struct SEGMENT_DESCRIPTOR *)(ldt_addr + 1*8);
	    ds_base = temp.base_low +(temp.base_mid << 16) + (temp.base_high<<24);
	    phy_addr = ds_base + eax;
	    sprintf(s,"gdt_base: %x\nldt_base: %x\nds_base: %x\nlog_addr: %x\nphy_addr: %x\nvalue: %d\n",gdt_addr,ldt_addr,ds_base,eax,phy_addr,*(int *)(phy_addr));
	    //int addr = task->ldt[1].base_low + (task->ldt[1].base_mid<<16) +(task->ldt[1].base_high<<24);
	    //sprintf(s,"%x %x %x %x %d\n",addr,task->ds_base,eax,addr+eax,*(int *)(addr+eax));
	    cons_putstr0(cons, s);
		//int i = ADR_GDT +task->tss.ldtr;
		//struct SEGMENT_DESCRIPTOR* des = *(ADR_GDT +task->tss.ldtr);
		//i = des->base_low + des->base_mid <<8 + des->base_high<<12;
	  	reg[7] = phy_addr;
		  break;
		}
		case 29:
        gdt = *(int *)(ds_base+eax+2);
        ldt_index = ecx >>3;
        ldt_des = gdt + ldt_index*8;
        temp =  *(struct SEGMENT_DESCRIPTOR *)(ldt_des);
        ldt_addr = temp.base_low +(temp.base_mid << 16) + (temp.base_high<<24);
        ds_index = ebx>>3;
        ds_des = ldt_addr + ds_index*8;
        temp = *(struct SEGMENT_DESCRIPTOR *)(ds_des);
        ds_addr = temp.base_low +(temp.base_mid << 16) + (temp.base_high<<24);
        phy_addr = ds_addr + ebp;
        sprintf(s,"gdt_base: %x  ldt_index: %d\nldt_des: %x  ldt_addr: %x\nds_index: %d  ds_des: %x\nds_addr: %x  log_addr: %x\nphy_addr:%x  value: %d\n",gdt,ldt_index,ldt_des,ldt_addr,ds_index,ds_des,ds_addr,ebp,phy_addr,*(int *)phy_addr);
        cons_putstr0(cons, s);
		break;
	
		case 30:
		cmd_reader();
		break;
		case 31:
		cmd_writer();
		break;
		case 32:
    	shareadd(cons);
		break;
		case 33:
		consume(cons);
		break;
		case 34:
		produce(cons);
		break;
		case 35:
		entrance(eax);
		break;
		case 36:
		exiting(eax);
		break;
		case 37:
		reg[7]=var_create((char*)ds_base+ebx,ecx);
		break;
		case 38:
		reg[7]=var_read((char*)ds_base+ebx,ecx);
		break;
		case 39:
		reg[7]=var_wrt((char*)ds_base+ebx,ecx,eax);
		break;
		case 40:
		reg[7]=var_free((char*)ds_base+ebx);
		break;
		case 41:
		avoid_sleep();
		break;
		case 42:
		Tlock();
		break;
		case 43:
		unTlock();
		break;
		default:
		break;
	}

	return 0;
}

/**
 * ??? interface handler
 */
int *inthandler0c(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOLE *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);	/* ï¿½Ù?ï¿?Iï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
}

/**
 * ??? interface handler
 */
int *inthandler0d(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOLE *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);	/* ï¿½Ù?ï¿?Iï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ */
}

/**
 * ??? interface handler
 */
void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col)
{
	int i, x, y, len, dx, dy;

	dx = x1 - x0;
	dy = y1 - y0;
	x = x0 << 10;
	y = y0 << 10;
	if (dx < 0) {
		dx = - dx;
	}
	if (dy < 0) {
		dy = - dy;
	}
	if (dx >= dy) {
		len = dx + 1;
		if (x0 > x1) {
			dx = -1024;
		} else {
			dx =  1024;
		}
		if (y0 <= y1) {
			dy = ((y1 - y0 + 1) << 10) / len;
		} else {
			dy = ((y1 - y0 - 1) << 10) / len;
		}
	} else {
		len = dy + 1;
		if (y0 > y1) {
			dy = -1024;
		} else {
			dy =  1024;
		}
		if (x0 <= x1) {
			dx = ((x1 - x0 + 1) << 10) / len;
		} else {
			dx = ((x1 - x0 - 1) << 10) / len;
		}
	}

	for (i = 0; i < len; i++) {
		sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
		x += dx;
		y += dy;
	}

	return;
}

void cmd_clearline(struct CONSOLE *cons, char *cmdline)
{
	if(*cmdline==0)
	return;
	else
	{for(;*cmdline!=0;cmdline++)
		*cmdline=0;  //æ¸?ç©?
	return;}

}

void cmd_stamp(struct CONSOLE *cons1, char *cmdline)
{
	struct TASK *task = task_now();
	cons_putstr0(cons1,"input 4 stamp values:");
	cons_newline(cons1);
	cmd_clearline(cons1,cmdline);//?????½æ??ç©?
	cmd_clearline(cons1,task->cmdline);//?????½æ??ç©?
	struct CONSOLE cons=*cons1;
	//cons_newline(&cons);
	char *str;
	str = get_1_line(cons,cmdline);
	// cons1->cur_y +=16;
	// cons_putstr0(cons1,str);
	// cons1->cur_y +=16;
	// cons1->cur_x = 8 ;

     char result[20], * cmd_str;
    int i, j, k, l, p, q;
    int a[4];
    static int s[1000];  /*ï¿??ï¿??*/
    int x, y, r = 0, count = 0; //r?????¥å??ç§»å????????å·¥ï¿½?,x????????°å??å¼?å¤´ï??y??????è¿?ç»???°å??ç»?å°?
    for (cmd_str = str; *cmd_str <= ' ' || *cmd_str == 0; cmd_str++) {}	/* ï¿?Xï¿?yï¿?[ï¿?Xï¿½ï¿½ï¿½ï¿½ï¿½ï¿½????????ï¿½Î?ï¿? */
    for (; cmd_str[r]!=0; )
    {

        if ('0' <= cmd_str[r] && cmd_str[r] <= '9')//ï¿????°å??
        {
            p = r;
            q = r + 1;
            a[count] = cmd_str[r] - '0';
            while ('0' <= cmd_str[q] && cmd_str[q] <= '9')
            {
                a[count] = 10 * a[count] + (cmd_str[q] - '0');
                q++;
            }
            r = q;  //??°èµ·ï¿??
            count++; //countåº?ï¿??ï¿½ä¸º4

        }
        else r++;

    }
    //scanf("%d %d %d %d", &a, &b, &c, &d);  /*è¾???¥å??ï¿??ï¿½é?¢å?¼é??ï¿??*/
    for(i=0; i<=5; i++)  /*ï¿??ï¿????????i???äº??§???a?????¢å?¼é??ç¥????å¼???°ï?????ï¿??5ï¿??*/
        for(j=0; i+j<=5; j++)  /*ï¿??ï¿????????j???äº??§???b?????¢å?¼é??ç¥????å¼???°ï??a??????ï¿??+b??????ç¥????ï¿??5ï¿??*/
            for(k=0; k+i+j<=5; k++)  /*ï¿??ï¿????????k???äº??§???c?????¢å?¼é??ç¥????å¼???°ï??a??????ï¿??+b??????ï¿??+c??????ç¥????ï¿??5ï¿??*/
                for(l=0; k+i+j+l<=5; l++)  /*ï¿??ï¿????????l???äº??§???d?????¢å?¼é??ç¥????å¼????,a??????ï¿??+b??????ï¿??+c??????ï¿??+d??????ç¥????ï¿??5ï¿??*/
                    if( a[0]*i+a[1]*j+a[2]*k+a[3]*l )
                        s[a[0]*i+a[1]*j+a[2]*k+a[3]*l]++;
    for(i=1; i<=1000; i++)
        if( !s[i] )
            break;
	cons1->cur_y +=16;
	cons1->cur_x = 8 ;
    sprintf(result, "The max is %d\n", --i);
    cons_putstr0(cons1,result);
	//cons_putstr0(cons1, cmdline);
	return;

// 		return;
}

char *get_1_line(struct CONSOLE cons, char *cmdline)
{
	struct TASK *task = task_now();
	int path_length = 0; // for calculating cmdline
	path_length = cons_putdir(&cons);
	int i;

	cons_putchar(&cons, '>', 1);
		for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons.sht != 0) { /* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿?pï¿?^ï¿?Cï¿?} */
				if (i != 0) {
					timer_init(cons.timer, &task->fifo, 0); /* ï¿½ï¿½ï¿½ï¿½0ï¿½ï¿½ */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(cons.timer, &task->fifo, 1); /* ï¿½ï¿½ï¿½ï¿½1ï¿½ï¿½ */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
			}
			if (i == 2) {	/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ON */
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½OFF */
				if (cons.sht != 0) {
					boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000,
						cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				cons.cur_c = -1;
			}
			if (i == 4) {	/* ï¿?Rï¿½ï¿½ï¿?\ï¿?[ï¿½ï¿½ï¿½Ì?uï¿?~ï¿?vï¿?{ï¿?^ï¿½ï¿½ï¿?Nï¿½ï¿½ï¿?bï¿?N */
				cmd_exit(&cons, task->fat);
			}
			if (256 <= i && i <= 511) { /* ï¿?Lï¿?[ï¿?{ï¿?[ï¿?hï¿?fï¿?[ï¿?^ï¿?iï¿?^ï¿?Xï¿?NAï¿?oï¿?Rï¿?j */
				if (i == 8 + 256) {
					/* ï¿?oï¿?bï¿?Nï¿?Xï¿?yï¿?[ï¿?X */
					if (cons.cur_x > 16 + path_length * 8) {
						/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½ï¿?Xï¿?yï¿?[ï¿?Xï¿½Å?ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿?Aï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½1ï¿½Â???ï¿? */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) {
					/* Enter */
					/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½ï¿?Xï¿?yï¿?[ï¿?Xï¿½Å?ï¿½ï¿½ï¿½ï¿½??ï¿½ï¿½ï¿½ï¿½ï¿?sï¿½ï¿½ï¿½ï¿½ */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - (path_length) - 2] = 0;
					cons_newline(&cons);
					if (cons.sht == 0) {
						cmd_exit(&cons, task->fat);
					}
					/* ï¿?vï¿½ï¿½ï¿½ï¿½ï¿?vï¿?gï¿?\ï¿½ï¿½ */
					//cons_putchar(&cons, '>', 1);
					//cons_putstr0(&cons, cmdline);
					return cmdline;
					//break;
				} else {
					/* ï¿½ï¿½??ï¿½ï¿½ï¿?? */
					if (cons.cur_x < 272) {
						/* ï¿½ê?¶ï¿½ï¿½ï¿½\ï¿½ï¿½ï¿½ï¿½ï¿½Ä?ï¿½ï¿½ï¿?Aï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½ï¿½1ï¿½Â?iï¿½ß?ï¿? */
						cmdline[cons.cur_x / 8 - (path_length) - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			/* ï¿?Jï¿?[ï¿?\ï¿½ï¿½ï¿½Ä?\ï¿½ï¿½ */
			if (cons.sht != 0) {
				if (cons.cur_c >= 0) {
					boxfill8(cons.sht->buf, cons.sht->bxsize, cons.cur_c, 
						cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				sheet_refresh(cons.sht, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
			}
		}
	}
	return cmdline;
}

void cmd_rps(struct CONSOLE *cons, char *cmdline)
{
	struct TASK *task = task_now();
	// cons_putstr0(cons,"input 4 stamp values:");
	// cons_newline(cons);
	// cmd_clearline(cons,cmdline);//?????½æ??ç©?
	// cmd_clearline(cons,task->cmdline);//?????½æ??ç©?
	// //cons_newline(&cons);
	// char *str;
	// str = get_1_line(cons,cmdline);
	int gamer;  // ?©å®¶å?ºæ??
    int computer;  // ??µè????ºæ??
    int result;  // æ¯?èµ?ç»????

	 // ä¸ºä????¿å???©ä¸?æ¬¡æ¸¸??å°±?????ºç??åº?ï¼????ä»¥å??ä»£ç????¾å??å¾??¯ä¸?
    while (1){
        cons_putstr0(cons,"Welcome to Rock-paper-scissors! Please choose your choice:\n");
        cons_putstr0(cons,"A:scissors\nB:Rock\nC:paper\nD:Exit\n");
		cmd_clearline(cons,cmdline);//?????½æ??ç©?
		cmd_clearline(cons,task->cmdline);//?????½æ??ç©?
		cons_newline(&cons);
		char *str;
		str = get_1_line(*cons,cmdline);  //?·å¾??©å®¶ç??è¾???¥æ?????
		char  result_str[40],* cmd_str;
		for (cmd_str = str; *cmd_str <= ' ' || *cmd_str == 0; cmd_str++) {}	
		//gamer = cmd_str[0];
		cons_newline(&cons);
		cons->cur_y +=16;
		cons->cur_x = 8 ;

		//cons_putstr1(cons,gamer,1);
		//cons_putstr0(cons,cmd_str);
		//cons_putstr1(cons,cmd_str[0],1);
		//cons_newline(&cons);
		// cons->cur_y +=16;
		// cons->cur_x = 8 ;

		if(strcmp(cmd_str,"A")==0||strcmp(cmd_str,"a")==0)
			gamer = 4;
		else if (strcmp(cmd_str,"B")==0||strcmp(cmd_str,"b")==0)
			gamer = 7;
		else if (strcmp(cmd_str,"C")==0||strcmp(cmd_str,"c")==0)
			gamer = 10;
		else if (strcmp(cmd_str,"D")==0||strcmp(cmd_str,"d")==0)
			return 0;
		else{
			sprintf(result_str,"Your choice %c is wrong,exit...\n",gamer);
				cons_putstr0(cons,result_str);
                return 0;
		} 

		// sprintf(result_str,"gamer: %d \n",gamer);
		// cons_putstr0(cons,result_str);

      
        my_srand(get_sec_hex());  // ?????ºæ?°ç??å­?
        computer=my_rand()%3;  // äº§ç???????ºæ?°å¹¶???ä½?ï¼?å¾???°ç?µè????ºæ??
        result=gamer+computer;  // gamer ä¸? char ç±»å??ï¼???°å?¦è??ç®???¶è??å¼ºå?¶è½¬??¢ç±»???
        cons_putstr0(cons,"Computer:");
        switch (computer)
        {
            case 0:cons_putstr0(cons,"scissors\n");break; //4    1
            case 1:cons_putstr0(cons,"Rock\n");break; //7  2
            case 2:cons_putstr0(cons,"paper\n");break;   //10 3
        }
        cons_putstr0(cons,"You:");
        switch (gamer)
        {
            case 4:cons_putstr0(cons,"scissors\n");break; 
            case 7:cons_putstr0(cons,"Rock\n");break; 
            case 10:cons_putstr0(cons,"paper\n");break;   
        }
        if (result==6||result==7||result==11) cons_putstr0(cons,"You win!");
        else if (result==5||result==9||result==10) cons_putstr0(cons,"Computer win!");
        else cons_putstr0(cons,"Tie");  //å¹³å??
     }
    return 0;
}

void cmd_guess(struct CONSOLE *cons, char *cmdline)
{
	int  a, z, t, i, c, m, g, s, j, k, l[4],r,p,q;  /*j:Êı×ÖÕıÈ·µÄÎ»Êı k:Î»ÖÃÕıÈ·µÄÎ»Êı*/
	char *cmd_str,result[30];
	struct TASK *task = task_now();
    my_srand(get_sec_hex());
    if( (my_rand()%10000) >= 1000 && (my_rand()%10000) <= 9999 )
        z=my_rand()%10000;  /*¼ÆËã»úÏëÒ»¸öËæ»úÊı*/
    cons_putstr0(cons,"»úÆ÷ÊäÈëËÄÎ»Êı****\n");
     for(c=1; ; c++) /*c: ²ÂÊı´ÎÊı¼ÆÊıÆ÷*/
     {
		 g=0;
		 r=0;
        cons_putstr0(cons,"ÇëÊäÈëÄã²ÂµÄËÄÎ»Êı:\n");

        //scanf("%d", &g); /*ÇëÈË²Â*/
		cmdline=0;
		cmd_clearline(cons,cmdline);//?????½æ??ç©?
		cmd_clearline(cons,task->cmdline);//?????½æ??ç©?
		cons_newline(&cons);
		char *str=0;

		str = get_1_line(*cons,cmdline);  //?·å¾??©å®¶ç??è¾???¥æ?????
		//cons_putstr0(cons,str);
		cons_newline(&cons);
		cons->cur_y +=16;
		cons->cur_x = 8 ;
	for (cmd_str = str; *cmd_str <= ' ' || *cmd_str == 0; cmd_str++) {}	
    for (; cmd_str[r]!=0; )
    {

        if ('0' <= cmd_str[r] && cmd_str[r] <= '9')
        {
            p = r;
            q = r + 1;
            g = cmd_str[r] - '0';
            while ('0' <= cmd_str[q] && cmd_str[q] <= '9')
            {
                g = 10 * g + (cmd_str[q] - '0');
                q++;
            }
            r = q;  //??°èµ·ï¿??

        }
	}
	

        a=z;
        j=0;
        k=0;
        l[0]=l[1]=l[2]=l[3]=0;
        for(i=1; i<5; i++) /*i:Ô­ÊıÖĞµÄµÚiÎ»Êı¡£¸öÎ»ÎªµÚÒ»Î»£¬Ç§Î»ÎªµÚ4Î»*/
        {
            s=g;
            m=1;
            for(t=1; t<5; t++)  /*ÈËËù²ÂÏëµÄÊı*/
            {
                if(a%10 == s%10)  /*ÈôµÚiÎ»ÓëÈË²ÂµÄµÚtÎ»ÏàÍ¬*/
                {
                    if(m && t!=l[0] && t!=l[1] && t!=l[2] && t!=l[3])
                    {
                        j++;
                        m=0;
                        l[j-1]=t;  /*Èô¸ÃÎ»ÖÃÉÏµÄÊı×ÖÉĞÎ´ÓëÆäËüÊı×Ö"ÏàÍ¬"*/
                    }  /*¼ÇÂ¼ÏàÍ¬Êı×ÖÊ±£¬¸ÃÊı×ÖÔÚËù²ÂÊı×ÖÖĞµÄÎ»ÖÃ*/
                    if(i==t)
                        k++; /*ÈôÎ»ÖÃÒ²ÏàÍ¬£¬Ôò¼ÆÊıÆ÷k¼Ó1*/
                }
                s/=10;
            }
            a/=10;
        }
        cons_putstr0(cons,"Äã²ÂµÄ½á¹ûÊÇ");
        sprintf(result,"%dA%dB\n", j, k);
		cons_putstr0(cons,result);
        if(k == 4)
        {
            cons_putstr0(cons,"****ÄãÓ®ÁË*****\n");
            cons_putstr0(cons,"\n~~********~~\n");
            break;  /*ÈôÎ»ÖÃÈ«²¿ÕıÈ·£¬ÔòÈË²Â¶ÔÁË£¬ÍË³ö*/
        }
    }
    sprintf(result,"Äã×Ü¹²²ÂÁË %d ´Î.\n",c);
	cons_putstr0(cons,result);
    return 0;
}

void wait(struct S *s,struct process *this_process,char * which_s){
	char buf[100];
	int eflags= io_load_eflags();
	io_cli();
	s->value--;
	if(s->value<0){
		//ï¿½ï¿½Ê¾Ë­ï¿½ÚµÈ´ï¿½
		sprintf(buf, "%s is waiting %s!\n", this_process->name, which_s);
	    cons_putstr0(this_process->task->cons, buf);

		//ï¿½ï¿½ï¿½ï¿½ï¿½È´ï¿½ï¿½Ğ±ï¿½
		if(s->list_last==NULL){//ï¿½Ğ±ï¿½ï¿½ï¿½
			s->list_first=this_process;
			s->list_last=this_process;
			s->list_last->next=NULL;
		}
		else{//ï¿½Ğ±ï¿½ï¿½ï¿½ï¿½ï¿½
			s->list_last->next=this_process;
			s->list_last=s->list_last->next;
			s->list_last->next=NULL;
		}
		//ï¿½ï¿½ï¿½ï¿½
		task_sleep(this_process->task);
	}
	io_store_eflags(eflags);
}

void signal(struct S *s,char * which_s){
	struct process *temp;
	char buf[100];
	int eflags= io_load_eflags();
	io_cli();
	s->value++;
	if(s->value<=0){
		//ï¿½Ğ±ï¿½
		temp=s->list_first;

		if(s->list_first==s->list_last){//Ö»ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ÚµÈ´ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ñºï¿½ï¿½È´ï¿½ï¿½Ğ±ï¿½ï¿½ï¿½
			s->list_first=s->list_last=NULL;
		}
		else
			s->list_first=s->list_first->next;
		//
		task_run(temp->task,-1,-1);//ï¿½ï¿½ï¿½ï¿½Ô­ï¿½ï¿½ï¿½ï¿½Ä¬ï¿½ï¿½ï¿½ï¿½ï¿½È¼ï¿½

		sprintf(buf, "%s already get %s.\n", temp->name, which_s);
	    cons_putstr0(temp->task->cons, buf);

	}
	io_store_eflags(eflags);
}

void init_S(){
	
	mutex.value=1;
	mutex.list_first=NULL;
	mutex.list_last=NULL;
	wrt.value=1;
	wrt.list_first=NULL;
	wrt.list_last=NULL;
	readcount=0;
	share_bupt=0;
}
#define END 1000
void cmd_reader(){
	char readbuf[100];
	int if_end=0;
	struct process this_process;
	this_process.next=NULL;
	this_process.task=task_now();
	while(1){
		sprintf(this_process.name,"reader %d",readcount+1);
		wait(&mutex,&this_process,"mutex");
		readcount++;
		if(readcount==1){
			wait(&wrt,&this_process,"wrt");
		}
		signal(&mutex,"mutex");
		//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ù½ï¿½ï¿½ï¿½
		sprintf(readbuf,"%s get share=%d|| %d\n",this_process.name,share_bupt,if_end+1);
		cons_putstr0(this_process.task->cons, readbuf);

		wait(&mutex,&this_process,"mutex");
		readcount--;
		if(readcount==0){
			signal(&wrt,"wrt");
		}
		signal(&mutex,"mutex");

		if_end++;
		if(if_end==END)
			break;
	}
}
void cmd_writer(){
	char writebuf[100];
	int if_end=0;
	struct process this_process;
	this_process.next=NULL;
	this_process.task=task_now();
	sprintf(this_process.name,"writer");

	while(1){
		wait(&wrt,&this_process,"wrt");

		share_bupt++;
		sprintf(writebuf,"%s have written share=%d|| %d\n",this_process.name,share_bupt,if_end+1);
		cons_putstr0(this_process.task->cons, writebuf);

		signal(&wrt,"wrt");

		if_end++;
		if(if_end==END)
			break;
	}
}

unsigned int char2int(char *cSize){//ï¿½ï¿½charï¿½ï¿½ï¿½ï¿½×ªï¿½ï¿½Îªintï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
	unsigned int iSize=0;
	char c;
	int i=0;
	while((c=cSize[i])!='\0'){
		iSize=iSize*10+c-'0';
		i++;
	}
	return iSize;
}
void cmd_mymem(char *cmdline){//Ä£Äâ·ÖÅäÄÚ´æ
	char cSize[10];

	unsigned int iSize;
	int i;
	unsigned int addr;

	char memsizebuf[100];

	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task=task_now();
	//
	for(i=0;cmdline[i]!=' ';i++);


	strcpy(cSize,&cmdline[i]);//°Ñ·ÖÅäµÄÄÚ´æ´óĞ¡¸³Öµ¸øcSize
	iSize=char2int(cSize);
	//allocÇ°ÄÚ´æÇé¿ö
	sprintf(memsizebuf,"before alloc %d:\n",iSize);
	cons_putstr0(task->cons,memsizebuf);
	for (i = 0; i < memman->frees; i++) {
		sprintf(memsizebuf,"NO.%d-size=%d  ",i,memman->free[i].size);
		cons_putstr0(task->cons,memsizebuf);
	}
	//alloc
	addr=memman_alloc((struct MEMMAN *) MEMMAN_ADDR,iSize);
	
	addrlist[num]=addr;
	sizelist[num]=iSize;
	num++;
	//allocÖ®ºóÄÚ´æÇé¿ö
	cons_putstr0(task->cons,"\nafter alloc:\n");
	for (i = 0; i < memman->frees; i++) {
		sprintf(memsizebuf,"NO.%d-size=%d  ",i,memman->free[i].size);
		cons_putstr0(task->cons,memsizebuf);
	}
	cons_putstr0(task->cons,"\n");
	//memman_free((struct MEMMAN *) MEMMAN_ADDR, addr,iSize);
}

void cmd_free(){//freeËùÓĞ·ÖÅäµÄÄÚ´æ
	int i;
	for(i=0;i<num;i++){
		memman_free((struct MEMMAN *) MEMMAN_ADDR, addrlist[i],sizelist[i]);
	}
}

int share=0;
int sharenum=0;//Ê¹ï¿½Ã¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ä½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
void shareadd(struct CONSOLE *cons)
{
	int i,j,x,temp,e;
	char s[60];
	struct TASK *now_task;

	if(sharenum==0)
	   share=0;
	sharenum+=1;
	while(sharenum<2)
	{
		now_task=task_now();
		now_task->flags=2;
	}
	for(;;)
	{
		temp=share;
		e=io_load_eflags();
		io_cli();
		share+=1;
		io_store_eflags(e);
		x=share;
		if((x-temp)>1)
		{
		sprintf(s,"share=%d,but share+1=%d\n",temp,x);
		cons_putstr0(cons,s);
		sharenum-=1;
		break;
		}
	}
	return ;
}

//ï¿½ï¿½ï¿½ï¿½ï¿½ßºï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ß³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½petersonï¿½ã·¨
#define BUFFER_SIZE 100
int producer=0,consumer=1;
int flag[2]={0,0};//ï¿½ï¿½Ê¾ï¿½Ä¸ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ù½ï¿½ï¿½ï¿½
int turn;//ï¿½ï¿½Ê¾ï¿½Ä¸ï¿½ï¿½ï¿½ï¿½Ì¿ï¿½ï¿½Ô½ï¿½ï¿½ï¿½ï¿½Ù½ï¿½ï¿½ï¿½
int in=0,out=0,counter=0;
int buffer[BUFFER_SIZE];//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
int xnum=0;

void produce(struct CONSOLE *cons)
{
	char *s;
	int temp,outcome,e;
	struct TASK *now_task;
	now_task=task_now();
	while(1){
		while (counter == BUFFER_SIZE)
		{
			now_task=task_now();//ï¿½ï¿½ï¿½ï¿½Ñ­ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ö¹ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
			now_task->flags=2;
		}
	    // xnum++;
		   flag[producer]=1;
		   turn=consumer;
	   	 while(flag[consumer]==1&&turn==consumer)
		   xnum++;
		  //ï¿½Ù½ï¿½ï¿½ï¿½
			 temp=in;
			 outcome=rand();
		   buffer[in]=outcome;
		   in = (in + 1)%BUFFER_SIZE;
			 counter++;
		   flag[producer]=0;
			 //Ê£ï¿½ï¿½ï¿½ï¿½
			 sprintf(s,"in buffer %d,produce %d\n",temp+1,outcome);
			 cons_putstr0(cons,s);
	}
	return ;
}

void consume(struct CONSOLE *cons)
{
	char *s;
	int temp,outcome,e;
	struct TASK *now_task;
	while(1){
		while (counter==0)
		{
			now_task=task_now();
			now_task->flags=2;
		}
		  flag[consumer]=1;
		  turn=producer;
		  while(flag[producer]==1&&turn==producer)
		  xnum++;
		  //ï¿½Ù½ï¿½ï¿½ï¿½
			temp=out;
			outcome=buffer[out];
			out=(out+1)%BUFFER_SIZE;
			counter--;
			flag[consumer]=0;
			//Ê£ï¿½ï¿½ï¿½ï¿½
			sprintf(s,"in buffer %d,consume %d\n",temp+1,outcome);
			cons_putstr0(cons,s);
	}
	return ;
}

//petersonç®?æ³????è¿???¥å?ºå???????ºå?ºï??è¿?2ä¸????ä»¥å?????äº???¥é????¥ä½¿???ï¼?ä½?ä»???½ç??äº?2ä¸?è¿?ç¨?ä¹????
void entrance(int x)
{
	if(x==1)
	{
		flag[consumer]=1;
		turn=producer;
		while(flag[producer]==1&&turn==producer)
		xnum++;
	}
	if(x==0)
	{
		flag[producer]=1;
		turn=consumer;
		while(flag[consumer]==1&&turn==consumer)
		xnum++;
	}
}

void exiting(int x)
{
   if(x==1)
	 {
		flag[consumer]=0;
	 }
	 if(x==0)
	 {
		 flag[producer]=0;
	 }
}

//cè¯????äº§ç???????ºæ?°ç??ä»£ç??
#define RANDOM_MAX 0x7FFFFFFF

static long my_do_rand(unsigned long *value)

{

   long quotient, remainder, t;
   quotient = *value / 127773L;

   remainder = *value % 127773L;

   t = 16807L * remainder - 2836L * quotient;


   if (t <= 0)

      t += 0x7FFFFFFFL;

   return ((*value = t) % ((unsigned long)RANDOM_MAX + 1));

}

static unsigned long next = 1;

int my_rand(void)

{

   return my_do_rand(&next);

}

 

void my_srand(unsigned int seed)

{

   next = seed;

}

