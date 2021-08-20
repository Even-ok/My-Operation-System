/* console.c,命令行(控制台)窗口管理程序接口 */

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

/* console_task,
 * 命令行窗口任务程序代码。*/
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
	if (nihongo[4096] != 0xff) {
		task->langmode = 1;
	} else {
		task->langmode = 0;
	}
	task->langbyte1 = 0;
	task->langmode = 3;

	if(cons.id == 1){
		cmd_mkfs(&cons);
	}else if(cons.id == 0){
		cmd_setlog(&cons);
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
			if (i <= 1 && cons.sht != 0) {
				if (i != 0) {
					timer_init(cons.timer, &task->fifo, 0);
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(cons.timer, &task->fifo, 1);
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
			}
			if (i == 2) {
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {
				if (cons.sht != 0) {
					boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000,
							cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				cons.cur_c = -1;
			}
			if (i == 4) {
				cmd_exit(&cons, fat);
			}
			if (i == 5) {
				//cmd_app(&cons, fat, "lines");
			}
			if (i == 6) {
				cmd_app(&cons, fat, "noodle");
			}
			if (i == 7) {
		    	//cmd_app(&cons, fat, "star1"); 
			}
			if (i == 8) {
				//cmd_app(&cons, fat, "color2"); 
			}
			if (i == 9) {
				//cmd_app(&cons, fat, "walk");
			}
			if (i == 10) {
				
			}
			if (i == 11) {
			   // cmd_app(&cons, fat, "bball");
			}
			if (i == 12) {	//  task->cmdline
				cmdline[0]='G'; cmdline[1]='V'; cmdline[2]='I'; cmdline[3]='E'; cmdline[4]='W';  cmdline[5]=' '; 
				cmdline[6]='5'; cmdline[7]='.'; cmdline[8]='J'; cmdline[9]='P'; cmdline[10]='G'; cmdline[11]=0; 
				cons_runcmd(cmdline, &cons, fat, memtotal);
			}
			if (i == 13) {	 //  task->cmdline
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
			if (256 <= i && i <= 511) {
				if (i == 8 + 256) { 
					if (cons.cur_x > 16 + path_length * 8) {
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) { /* if press Enter */
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

					// sprintf(s, "original cmdline = %s[EOF]\n", cmdline);
					// cons_putstr(&cons, s);

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

					cons_runcmd(cmdline, &cons, fat, memtotal);
					if (cons.sht == 0) {
						cmd_exit(&cons, fat);
					}
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
					if (cons.cur_x < 240) {
						cmdline[cons.cur_x / 8 - (path_length) - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
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

/* cons_putchar,
 * 在cons所指命令行窗口光标当前位置显示chr字符,move标识光标是否移动。*/
void cons_putchar(struct CONSOLE *cons, int chr, char move)
{
	char s[2];
	s[0] = chr;
	s[1] = 0;
	if (s[0] == 0x09) {
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
				break;
			}
		}
	} else if (s[0] == 0x0a) {
		cons_newline(cons);
	} else if (s[0] == 0x0d) {
	} else {
		if (cons->sht != 0) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		}
		if (move != 0) {
			cons->cur_x += 8;
			if (cons->cur_x == 8 + cons->sht->bxsize-16) {
				cons_newline(cons);
			}
		}
	}
	return;
}

/* cons_newline,
 * 在cons所指命令行窗口中换行。*/
void cons_newline(struct CONSOLE *cons)
{
	int x, y, xmax, ymax;
	xmax = cons->sht->bxsize - 16;
	ymax = cons->sht->bysize - 37;
	struct SHEET *sheet = cons->sht;
	struct TASK *task = task_now();
	//if (cons->cur_y < 28 + 112) {
	if (cons->cur_y < 28 + ymax - 16){
		cons->cur_y += 16;
	} else {
		if (sheet != 0) {
			for (y = 28; y < 28 + ymax - 16; y++) {
				for (x = 8; x < 8 + xmax; x++) {
					sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
				}
			}

			for (y = 28 + ymax - 16; y < 28 + ymax; y++) {
				for (x = 8; x < 8 + xmax; x++) {
					sheet->buf[x + y * sheet->bxsize] = COL8_000000;
				}
			}
			sheet_refresh(sheet, 8, 28, 8 + xmax, 28 + ymax);
		}
	}
	cons->cur_x = 8;
	if (task->langmode == 1 && task->langbyte1 != 0) {
		cons->cur_x = 16;
	}
	return;
}


void cons_putstr(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

/* cons_putstr0,
 * 在cons所指命令行窗口中显示s所指字符串(0结尾)。*/
void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

/* cons_putstr1,
 * 在cons所指命令行窗口中显示s所指字符的前l个。*/
void cons_putstr1(struct CONSOLE *cons, char *s, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return;
}

int cons_putdir(struct CONSOLE *cons){
	struct MYDIRINFO *dinfo = cons->current_dir;
	char pathname[MAX_CMDLINE];
	int i;
	int pathname_length = 0;

	get_pathname(pathname, dinfo);
	for(i=0; pathname[i]!='\0';i++) pathname_length++;

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

		sprintf(pathname, "%s", tempname);
		sprintf(dirname, "");
	}

	sprintf(s, "/%s", pathname);
	sprintf(pathname, "%s", s);

	return;
}


/* cons_runcmd,
 * 在cons窗口中执行cmdline中所包含的命令(或可执行程序)。*/
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
		cons->current_dir = dest_dinfo;
	}
	return;
}

/**
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
				sprintf(s, "change directory using relative path\n");
				debug_print(s);
				dinfo = cons->current_dir;
				get_pathname(prev_pname, dinfo); //debug
			}
		}else if(cdline[cp] == '/'){
			debug_print("'/' has found.\n");
			cp++;
			if(cp == 1){
				sprintf(s, "change directory using absolute path\n");
				debug_print(s);
				dinfo = (struct MYDIRINFO *)ROOT_DIR_ADDR;
			}
		}else if(isDOUBLEPOINT()){
			debug_print("\"..\" has found.\n");
			cp += 2;
			if(cp == 2){
				sprintf(s, "change directory using relative path\n");
				debug_print(s);
				dinfo = cons->current_dir;
				get_pathname(prev_pname, dinfo);
			}

			if(dinfo->parent_dir == 0){
				cd_error(cons, "Can't move because here is ROOT directory.\n");
				debug_print("*********************************\n");
				return 0; 
			}
			dinfo = dinfo->parent_dir;
			goto PARSE_NEXT;
		}else{
			cd_error(cons, "Incorrect initial character.\n");
			debug_print("*********************************\n");
			return 0;
		}

		if(isDOUBLEPOINT() || cdline[cp] == '\0'){
		}else{
			get_dirname(dirname, cdline);
			finfo = myfinfo_search(dirname, dinfo, MAX_FINFO_NUM);
			if(finfo == 0){
				cd_error(cons, "Can't find this directory.\n");
				debug_print("*********************************\n");
				return 0; 
			}else{
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

	//cons->current_dir = dinfo;
	debug_print("*********************************\n");
	cons_newline(cons);
	return dinfo;
}

void cd_error(struct CONSOLE *cons, char *message){
	char s[50];
	int i, j, k;

	get_pathname(s, cons->current_dir);
	for(i=0; s[i]!='\0'; i++)s[i] = ' ';
	for(j=0; j<3; j++) s[i+j] = ' ';
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


#define MODE_DEF	0x00
#define MODE_CLS	0x01
#define MODE_INS	0x02
#define MODE_ADD	0x04
#define MODE_OPEN	0x08
#define MODE_ALL	0xFF
void cmd_edit(struct CONSOLE *cons, char *cmdline){
	struct MYDIRINFO *dinfo = cons->current_dir;
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
	while(cmdline[p] == ' ') p++;

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
			debug_print("EDIT:open mode\n");
			mode = MODE_OPEN;
		}

		if(setfdata == 0 && mode != MODE_OPEN){
			sprintf(s, "can't edit: There is no file being opened.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}

		if(strcmp(option, "cls") == 0){
			debug_print("EDIT:clear mode\n");
			mode = MODE_CLS;
			myfwrite(setfdata, "");
		}else if(strcmp(option, "ins") == 0){
			debug_print("EDIT:insert mode\n");
			mode = MODE_INS;
		}else if(strcmp(option, "add") == 0){
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

	for (i = 0; i < MAX_FINFO_NUM; i++) {
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}
	}

	for(j=0; dir_name[j] != 0; j++) {
		if ('a' <= dir_name[j] && dir_name[j] <= 'z') {
			dir_name[j] -= 'a'-'A';
		}
		dinfo->finfo[i].name[j] = dir_name[j];
	}

	for(; j<8 ;j++) dinfo->finfo[i].name[j] = ' ';


	dinfo->finfo[i].child_dir = get_newdinfo();	
	dinfo->finfo[i].clustno = 0;
	dinfo->finfo[i].date = 0;
	dinfo->finfo[i].type = 0x10;
	dinfo->finfo[i].size = sizeof(dinfo->finfo[i]);
	dinfo->finfo[i].fdata = 0;

	sprintf(s, "created directory: name = %s\n", dinfo->finfo[i].name);
	cons_putstr(cons, s);
	sprintf(s, "\tchild dir address = 0x%08x\n", dinfo->finfo[i].child_dir);
	debug_print(s);
	sprintf(s, "\ttype=0x%02x\n", dinfo->finfo[i].type);
	debug_print(s);

	struct MYDIRINFO *child_dinfo = (struct MYDIRINFO *)dinfo->finfo[i].child_dir;
	child_dinfo->this_dir = dinfo->finfo[i].child_dir; 
	sprintf(child_dinfo->name, dir_name); 
	child_dinfo->parent_dir = dinfo->this_dir; 

	sprintf(s, "\tchild dinfo addr = 0x%08x\n", child_dinfo->this_dir);
	debug_print(s);
	sprintf(s, "\tchild dinfo name = %s\n", child_dinfo->name);
	debug_print(s);
	sprintf(s, "\tchild dinfo parent addr = 0x%08x\n", child_dinfo->parent_dir);
	debug_print(s);

	cons_newline(cons);
	return;
}

/**
 * make file in my filesystem
 */
void cmd_mkfile(struct CONSOLE *cons, char *cmdline){
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

	for (i = 0; i < MAX_FINFO_NUM; i++) {
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}
	}

	for(j=0; j<8; j++) dinfo->finfo[i].name[j] = filename[j];
	for(j=0; j<3; j++) dinfo->finfo[i].ext[j] = filename[8+j];
	dinfo->finfo[i].clustno = 0;
	dinfo->finfo[i].date = 0;
	dinfo->finfo[i].type = 0x20;
	dinfo->finfo[i].size = 0;

	struct MYFILEDATA test;
	test.head.stat = STAT_ALL;	
	test.head.this_dir = dinfo->this_dir;
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
	fdata->head.stat = 0x01;
	fdata->head.this_fdata = fdata; 
	fdata->head.this_dir = dinfo->this_dir;
	dinfo->finfo[i].fdata = fdata;

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

/* cmd_mem,
 * mem 命令对应子程序,
 * 在cons所指窗口显示内存使用情况。*/
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

/* cmd_cls,
 * cls 命令对应子程序,将cons所指命令行窗口清屏。*/
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
	// show detail file type info
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
			cons_putstr(cons, "[EOF]\n");
			sprintf(s, "size: %d\n", get_size_myfdata(finfo->fdata));
			cons_putstr(cons, s);
			cons_newline(cons);
		}else{
			sprintf(s, "\tthis is not file.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
		}
	}

	/* show the strings in char s[100] */
	return;
}


void cmd_mkfs(struct CONSOLE * cons){
	struct MYDIRINFO dinfo;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];

	sprintf(dinfo.name, "ROOT    ");
	dinfo.parent_dir = 0x00; // 0x00: root directory
	dinfo.this_dir = (struct MYDIRINFO *)ROOT_DIR_ADDR;
	cons->current_dir = (struct MYDIRINFO *)dinfo.this_dir;

	for (i = 0; i < 10; i++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j]; /* "filename" */
				}
				s[ 9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];

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

/* cmd_dir,
 * dir 命令对应子程序,在cons所指窗口中列表显示当前系统的文件信息。*/
void cmd_dir(struct CONSOLE *cons){
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
	// dir_num = get_newdinfo(cons);	

	//sprintf(s, "dinfo number = %d\n", dir_num);
	//cons_putstr(cons, s);

	//display files in current dir
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}

		if (dinfo->finfo[i].name[0] != 0xe5) {
			if ((dinfo->finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext\t%7d [FILE]\n", dinfo->finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = dinfo->finfo[i].name[j]; /* "filename" */
				}
				s[ 9] = dinfo->finfo[i].ext[0];
				s[10] = dinfo->finfo[i].ext[1];
				s[11] = dinfo->finfo[i].ext[2];	
				cons_putstr(cons, s);

			}else if((dinfo->finfo[i].type & 0x10) == 0x10){
				sprintf(s, "filename    \t%7d [DIR]\n", dinfo->finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = dinfo->finfo[i].name[j]; 
				}
				cons_putstr(cons, s);
				//sprintf(s, "test %s\t%d\t[DIR]",dinfo->finfo[i].name, dinfo->finfo[i].size);
				//cons_putstr(cons, s);
			}
		}
	}

	if(i == 0){
		sprintf(s, "this directory has no file...\n");
		cons_putstr(cons, s);
	}
	cons_newline(cons);
	return;
}

void cmd_fddir(struct CONSOLE *cons)
{

	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}

		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j]; 
				}
				s[ 9] = finfo[i].ext[0];
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

/* cmd_exit,
 * eixt 命令或命令4对应子程序,退出cons所指命令行窗口。*/
void cmd_exit(struct CONSOLE *cons, int *fat)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task = task_now();
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct FIFO32 *fifo = (struct FIFO32 *) *((int *) 0x0fec);
	if (cons->sht != 0) {
		timer_cancel(cons->timer);
	}
	memman_free_4k(memman, (int) fat, 4 * 2880);
	io_cli();
	if (cons->sht != 0) {
		fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);	
	} else {
		fifo32_put(fifo, task - taskctl->tasks0 + 1024);	
	}
	io_sti();
	for (;;) {
		task_sleep(task);
	}
}

/* cmd_start,
 * "start cmd"命令对应子程序,打开新的命令行窗口并执行cmd。*/
void cmd_start(struct CONSOLE *cons, char *cmdline, int memtotal)
{
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht = open_console(shtctl, memtotal);
	struct FIFO32 *fifo = &sht->task->fifo;
	int i;
	sheet_slide(sht, 32, 4);
	sheet_updown(sht, shtctl->top);
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
 * change language mode，设置当前任务(cons所指命令行窗口任务)的语言模式
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

/* cmd_app,
 * 在cons所指命令行窗口中运行可执行文件的处理程序。*/
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo;
	char name[18], *p, *q;
	struct TASK *task = task_now();
	int i, segsiz, datsiz, esp, dathrb, appsiz;
	struct SHTCTL *shtctl;
	struct SHEET *sht;

	for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0;

	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		name[i    ] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	if (finfo != 0) {
		appsiz = finfo->size;
		p = file_loadfile2(finfo->clustno, &appsiz, fat);
		if (appsiz >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
			segsiz = *((int *) (p + 0x0000));
			esp    = *((int *) (p + 0x000c));
			datsiz = *((int *) (p + 0x0010));
			dathrb = *((int *) (p + 0x0014));
			q = (char *) memman_alloc_4k(memman, segsiz);
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
					sheet_free(sht);
				}
			}
			for (i = 0; i < 8; i++) {
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
	return 0;
}

/* hrb_api,
 * 系统调用C处理函数,由系统调用入口程序 _asm_hrb_api 调用(见dsctbl.c/init_gdtidt)。
 *
 * edx为应用程序中所传递的系统调用号;
 * 其余参数充当对应地充当各系统调用的参数。*/
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct TASK *task = task_now();
	int ds_base = task->ds_base;
	struct CONSOLE *cons = task->cons;
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht;
	struct FIFO32 *sys_fifo = (struct FIFO32 *) *((int *) 0x0fec);
	int *reg = &eax + 1;
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
		sheet_updown(sht, shtctl->top);
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
		ecx &= 0xfffffff0;
		memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
		break;
	case 9:
		ecx = (ecx + 0x0f) & 0xfffffff0;
		reg[7] = memman_alloc((struct MEMMAN *) (ebx + ds_base), ecx);
		break;
	case 10:
		ecx = (ecx + 0x0f) & 0xfffffff0;
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
					task_sleep(task);	/* FIFO */
				} else {
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons->sht != 0) {
				timer_init(cons->timer, &task->fifo, 1); 
				timer_settime(cons->timer, 50);
			}
			if (i == 2) {
				cons->cur_c = COL8_FFFFFF;
			}
			if (i == 3) {
				cons->cur_c = -1;
			}
			if (i == 4) {
				timer_cancel(cons->timer);
				io_cli();
				fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024);
				cons->sht = 0;
				io_sti();
			}
			if (i >= 256) {
				reg[7] = i - 256;
				return 0;
			}
		}
		break;
	case 16:
		reg[7] = (int) timer_alloc();
		((struct TIMER *) reg[7])->flags2 = 1;
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
		/* int api_fopen(char *fname)
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
		/* void api_fclose(int fhandle)
		 * EDX = 22
		 * EAX = file handle
		 */
		fh = (struct FILEHANDLE *) eax;
		memman_free_4k(memman, (int) fh->buf, fh->size);
		fh->buf = 0;
		break;
	case 23:
		/* api_fseek(int fhandle, int offset, int mode)
		 * EDX = 23
		 * EAX = file handle
		 * ECX = seek mode
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
		/* int api_fsize(int fhandle, int mode)
		 * EDX = 24
		 * EAX = filehandle
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
		 * EAX = file handle */
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
		/* int api_cmdline(char *buf, int maxsize)
		 * EDX = 26 */
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
		/* inr api_getlang(void)
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

/* inthandler0c,
 * 栈异常C处理程序: 提示栈异常后结束应用程序。
 * 在发生栈异常时由栈异常处理入口程序 _asm_inthandler0c 调用。*/
int *inthandler0c(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOLE *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0C :\n Stack Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);
}

/* inthandler0d,
 * 保护异常C处理函数:提示保护异常并结束应用程序。
 * 在发生保护异常时由保护异常入口程序_asm_inthandler0d调用。*/
int *inthandler0d(int *esp)
{
	struct TASK *task = task_now();
	struct CONSOLE *cons = task->cons;
	char s[30];
	cons_putstr0(cons, "\nINT 0D :\n General Protected Exception.\n");
	sprintf(s, "EIP = %08X\n", esp[11]);
	cons_putstr0(cons, s);
	return &(task->tss.esp0);
}

/* hrb_api_linewin,
 * 在sht所指画面中描绘(x0,y0)和(x1,y1)两点之间线段的画面信息。*/
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
		*cmdline=0; 
	return;}

}

void cmd_stamp(struct CONSOLE *cons1, char *cmdline)
{
	struct TASK *task = task_now();
	cons_putstr0(cons1,"input 4 stamp values:");
	cons_newline(cons1);
	cmd_clearline(cons1,cmdline);
	cmd_clearline(cons1,task->cmdline);
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
    static int s[1000]; 
    int x, y, r = 0, count = 0; 
    for (cmd_str = str; *cmd_str <= ' ' || *cmd_str == 0; cmd_str++) {}
    for (; cmd_str[r]!=0; )
    {

        if ('0' <= cmd_str[r] && cmd_str[r] <= '9')
        {
            p = r;
            q = r + 1;
            a[count] = cmd_str[r] - '0';
            while ('0' <= cmd_str[q] && cmd_str[q] <= '9')
            {
                a[count] = 10 * a[count] + (cmd_str[q] - '0');
                q++;
            }
            r = q; 
            count++; 

        }
        else r++;

    }
    //scanf("%d %d %d %d", &a, &b, &c, &d);  
    for(i=0; i<=5; i++) 
        for(j=0; i+j<=5; j++)
            for(k=0; k+i+j<=5; k++) 
                for(l=0; k+i+j+l<=5; l++) 
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
			if (i <= 1 && cons.sht != 0) {
				if (i != 0) {
					timer_init(cons.timer, &task->fifo, 0); 
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(cons.timer, &task->fifo, 1); 
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
			}
			if (i == 2) {
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	
				if (cons.sht != 0) {
					boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000,
						cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				cons.cur_c = -1;
			}
			if (i == 4) {	
				cmd_exit(&cons, task->fat);
			}
			if (256 <= i && i <= 511) { 
				if (i == 8 + 256) {
					if (cons.cur_x > 16 + path_length * 8) {
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) {
					/* Enter */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - (path_length) - 2] = 0;
					cons_newline(&cons);
					if (cons.sht == 0) {
						cmd_exit(&cons, task->fat);
					}
					//cons_putchar(&cons, '>', 1);
					//cons_putstr0(&cons, cmdline);
					return cmdline;
					//break;
				} else {
					if (cons.cur_x < 272) {
						cmdline[cons.cur_x / 8 - (path_length) - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
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
	// cmd_clearline(cons,cmdline);
	// cmd_clearline(cons,task->cmdline);
	// //cons_newline(&cons);
	// char *str;
	// str = get_1_line(cons,cmdline);
	int gamer; 
    int computer; 
    int result; 

    while (1){
        cons_putstr0(cons,"Welcome to Rock-paper-scissors! Please choose your choice:\n");
        cons_putstr0(cons,"A:scissors\nB:Rock\nC:paper\nD:Exit\n");
		cmd_clearline(cons,cmdline);
		cmd_clearline(cons,task->cmdline);
		cons_newline(&cons);
		char *str;
		str = get_1_line(*cons,cmdline);
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

      
        my_srand(get_sec_hex()); 
        computer=my_rand()%3;  
        result=gamer+computer; 
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
        else cons_putstr0(cons,"Tie"); 
     }
    return 0;
}

void cmd_guess(struct CONSOLE *cons, char *cmdline)
{
	int  a, z, t, i, c, m, g, s, j, k, l[4],r,p,q; 
	char *cmd_str,result[30];
	struct TASK *task = task_now();
    my_srand(get_sec_hex());
    if( (my_rand()%10000) >= 1000 && (my_rand()%10000) <= 9999 )
        z=my_rand()%10000;  
    cons_putstr0(cons,"����������λ��****\n");
     for(c=1; ; c++)
     {
		 g=0;
		 r=0;
        cons_putstr0(cons,"��������µ���λ��:\n");

        //scanf("%d", &g); 
		cmdline=0;
		cmd_clearline(cons,cmdline);
		cmd_clearline(cons,task->cmdline);
		cons_newline(&cons);
		char *str=0;

		str = get_1_line(*cons,cmdline); 
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
            r = q; 

        }
	}
	

        a=z;
        j=0;
        k=0;
        l[0]=l[1]=l[2]=l[3]=0;
        for(i=1; i<5; i++) 
        {
            s=g;
            m=1;
            for(t=1; t<5; t++)  
            {
                if(a%10 == s%10)  
                {
                    if(m && t!=l[0] && t!=l[1] && t!=l[2] && t!=l[3])
                    {
                        j++;
                        m=0;
                        l[j-1]=t; 
                    }  
                    if(i==t)
                        k++;
                }
                s/=10;
            }
            a/=10;
        }
        cons_putstr0(cons,"��µĽ����");
        sprintf(result,"%dA%dB\n", j, k);
		cons_putstr0(cons,result);
        if(k == 4)
        {
            cons_putstr0(cons,"****��Ӯ��*****\n");
            cons_putstr0(cons,"\n~~********~~\n");
            break; 
        }
    }
    sprintf(result,"���ܹ����� %d ��.\n",c);
	cons_putstr0(cons,result);
    return 0;
}

void wait(struct S *s,struct process *this_process,char * which_s){
	char buf[100];
	int eflags= io_load_eflags();
	io_cli();
	s->value--;
	if(s->value<0){
		sprintf(buf, "%s is waiting %s!\n", this_process->name, which_s);
	    cons_putstr0(this_process->task->cons, buf);

		if(s->list_last==NULL){
			s->list_first=this_process;
			s->list_last=this_process;
			s->list_last->next=NULL;
		}
		else{
			s->list_last->next=this_process;
			s->list_last=s->list_last->next;
			s->list_last->next=NULL;
		}
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
		temp=s->list_first;

		if(s->list_first==s->list_last){
			s->list_first=s->list_last=NULL;
		}
		else
			s->list_first=s->list_first->next;
		//
		task_run(temp->task,-1,-1);

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

unsigned int char2int(char *cSize){
	unsigned int iSize=0;
	char c;
	int i=0;
	while((c=cSize[i])!='\0'){
		iSize=iSize*10+c-'0';
		i++;
	}
	return iSize;
}
void cmd_mymem(char *cmdline){
	char cSize[10];

	unsigned int iSize;
	int i;
	unsigned int addr;

	char memsizebuf[100];

	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task=task_now();
	for(i=0;cmdline[i]!=' ';i++);


	strcpy(cSize,&cmdline[i]);
	iSize=char2int(cSize);
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
	//alloc
	cons_putstr0(task->cons,"\nafter alloc:\n");
	for (i = 0; i < memman->frees; i++) {
		sprintf(memsizebuf,"NO.%d-size=%d  ",i,memman->free[i].size);
		cons_putstr0(task->cons,memsizebuf);
	}
	cons_putstr0(task->cons,"\n");
	//memman_free((struct MEMMAN *) MEMMAN_ADDR, addr,iSize);
}

void cmd_free(){//free
	int i;
	for(i=0;i<num;i++){
		memman_free((struct MEMMAN *) MEMMAN_ADDR, addrlist[i],sizelist[i]);
	}
}

int share=0;
int sharenum=0;
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

#define BUFFER_SIZE 100
int producer=0,consumer=1;
int flag[2]={0,0};
int turn;
int in=0,out=0,counter=0;
int buffer[BUFFER_SIZE];
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
			now_task=task_now();
			now_task->flags=2;
		}
	    // xnum++;
		   flag[producer]=1;
		   turn=consumer;
	   	 while(flag[consumer]==1&&turn==consumer)
		   xnum++;
			 temp=in;
			 outcome=rand();
		   buffer[in]=outcome;
		   in = (in + 1)%BUFFER_SIZE;
			 counter++;
		   flag[producer]=0;
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
			temp=out;
			outcome=buffer[out];
			out=(out+1)%BUFFER_SIZE;
			counter--;
			flag[consumer]=0;
			sprintf(s,"in buffer %d,consume %d\n",temp+1,outcome);
			cons_putstr0(cons,s);
	}
	return ;
}

//peterson
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

