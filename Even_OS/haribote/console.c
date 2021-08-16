/* �R���\�[���֌W */
#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define CMDHIS_NR 32
#define CMDBUF 1024


struct CMDHISTORY {
int first, last, head, tail, now;
char ** argv;
char *buf;
}s;

/* Global�ϐ� */
struct CONSOLE *log;
//unsigned int *g_current_dir;
// int g_pathname_length = 0;
int console_id=0;
struct MYFILEDATA *setfdata = 0;


struct CMDHISTORY cmdhis;
char cmd_buf[CMDBUF];
/* 代码中略*/

cmdhisbuf = cmd_buf;


/* ���O�R���\�[���ɕ�����str���o�͂��� */
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

	// // 150�����ȏ�͏o�͂��Ȃ�
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
		fhandle[i].buf = 0;	/* ���g�p�}�[�N */
	}
	task->fhandle = fhandle;
	task->fat = fat;
	if (nihongo[4096] != 0xff) {	/* ���{��t�H���g�t�@�C����ǂݍ��߂����H */
		task->langmode = 1;
	} else {
		task->langmode = 0;
	}
	task->langbyte1 = 0;

	/* �v�����v�g�\�� */
	if(cons.id == 1){
		cmd_mkfs(&cons);	/* �����R���\�[���ɑ΂��ċ����I��mkfs���g�� */
	}else if(cons.id == 0){
		cmd_setlog(&cons);	/* ���O�p�R���\�[���ɑ΂��ă��O�o�͗p�̐ݒ���{�� */
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
			if (i <= 1 && cons.sht != 0) { /* �J�[�\���p�^�C�} */
				if (i != 0) {
					timer_init(cons.timer, &task->fifo, 0); /* ����0�� */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(cons.timer, &task->fifo, 1); /* ����1�� */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
			}
			if (i == 2) {	/* �J�[�\��ON */
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* �J�[�\��OFF */
				if (cons.sht != 0) {
					boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000,
							cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				cons.cur_c = -1;
			}
			if (i == 4) {	/* �R���\�[���́u�~�v�{�^���N���b�N */
				cmd_exit(&cons, fat);
			}
			if (i == 5) {	//ȡ����5  
				//cmd_app(&cons, fat, "lines");
			}
			if (i == 6) {	//ȡ����6  
				cmd_app(&cons, fat, "noodle");
			}
			if (i == 7) {	//ȡ����7  
		    	//cmd_app(&cons, fat, "star1"); 
			}
			if (i == 8) {	//ȡ����8  
				//cmd_app(&cons, fat, "color2"); 
			}
			if (i == 9) {	//ȡ����9  
				//cmd_app(&cons, fat, "walk");
			}
			if (i == 10) {	//ȡ����10    ���� 
				
			}
			if (i == 11) {	//ȡ����11 
			   // cmd_app(&cons, fat, "bball");
			}
			if (i == 12) {	//ͼƬ������  task->cmdline
				cmdline[0]='G'; cmdline[1]='V'; cmdline[2]='I'; cmdline[3]='E'; cmdline[4]='W';  cmdline[5]=' '; 
				cmdline[6]='5'; cmdline[7]='.'; cmdline[8]='J'; cmdline[9]='P'; cmdline[10]='G'; cmdline[11]=0; 
				cons_runcmd(cmdline, &cons, fat, memtotal);
			}
			if (i == 13) {	 //ͼƬ������  task->cmdline
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
			if (256 <= i && i <= 511) { /* �L�[�{�[�h�f�[�^�i�^�X�NA�o�R�j */
				if (i == 8 + 256) { /* �o�b�N�X�y�[�X */
					if (cons.cur_x > 16 + path_length * 8) {
						/* �J�[�\�����X�y�[�X�ŏ����Ă���A�J�[�\����1�߂� */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) { /* if press Enter */
					/* �J�[�\�����X�y�[�X�ŏ����Ă�����s���� */
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

					// *****�R�}���h���C���̃f�o�b�O�R�[�h*****
					// sprintf(s, "original cmdline = %s[EOF]\n", cmdline);
					// cons_putstr(&cons, s);

											/* 在以下添加如下代码 */
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

					cons_runcmd(cmdline, &cons, fat, memtotal);	/* �R�}���h���s */
					if (cons.sht == 0) {
						cmd_exit(&cons, fat);
					}
					/* �v�����v�g�\�� */
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
					/* ��ʕ��� */
					if (cons.cur_x < 240) {
						/* �ꕶ���\�����Ă���A�J�[�\����1�i�߂� */
						/* ����,
						 * cons.cur_x / 8 = �w�����ڂ̕���
						 * -2 = 0������('>')�͊܂߂Ȃ�
						 */
						cmdline[cons.cur_x / 8 - (path_length) - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			/* �J�[�\���ĕ\�� */
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
	if (s[0] == 0x09) {	/* �^�u */
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
				break;	/* 32�Ŋ���؂ꂽ��break */
			}
		}
	} else if (s[0] == 0x0a) {	/* ���s */
		cons_newline(cons);
	} else if (s[0] == 0x0d) {	/* ���A */
		/* �Ƃ肠�����Ȃɂ����Ȃ� */
	} else {	/* ���ʂ̕��� */
		if (cons->sht != 0) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		}
		if (move != 0) {
			/* move��0�̂Ƃ��̓J�[�\����i�߂Ȃ� */
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
		cons->cur_y += 16; /* ���̍s�� */
	} else {
		/* �X�N���[�� */
		if (sheet != 0) {
			/* VRAM���̊e1�s���A���̏ꏊ�ɃR�s�[���� */
			for (y = 28; y < 28 + ymax - 16; y++) {
				for (x = 8; x < 8 + xmax; x++) {
					sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
				}
			}

			/* �Ō�̍s�����œh��Ԃ� */
			for (y = 28 + ymax - 16; y < 28 + ymax; y++) {
				for (x = 8; x < 8 + xmax; x++) {
					sheet->buf[x + y * sheet->bxsize] = COL8_000000;
				}
			}
			/* �V�[�g����8<x<248, 28<y<156�͈̔͂��ĕ`�悷�� */
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
 * console�ɕ�����s���o�͂���
 */
void cons_putstr(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}


/**
 * console�ɕ�����s���o�͂���
 */
void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

/**
 * console�ɕ�����s���A����len�܂ŏo�͂���
 */
void cons_putstr1(struct CONSOLE *cons, char *s, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return;
}

/* ���݂̃f�B���N�g���̐��Path��\������.
 * @return pathname�̕�����̒�����Ԃ�(�R�}���h���C���̒������v�Z���邽��)
 */
int cons_putdir(struct CONSOLE *cons){
	struct MYDIRINFO *dinfo = cons->current_dir;
	char pathname[MAX_CMDLINE];
	int i;
	int pathname_length = 0;

	get_pathname(pathname, dinfo);	// �p�X����T�����Apathname�Ɋi�[����
	for(i=0; pathname[i]!='\0';i++) pathname_length++;

	/* path���R���\�[���ɕ\�� (���ӁF���̂Ƃ��͉��s�����Ȃ�)*/
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

		// ������
		sprintf(pathname, "%s", tempname);
		sprintf(dirname, "");
	}

	// pathname��"/"(ROOT)��t��������B
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
	} else if (strcmp(cmdline, "log") == 0 && cons->sht != 0){
		cmd_log(cons);
	} else if (strcmp(cmdline, "logcls") == 0 && cons->sht != 0){
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
	} else if (strncmp(cmdline, "ncst ", 5) == 0) {
		cmd_ncst(cons, cmdline, memtotal);
	} else if (strncmp(cmdline, "start ", 6) == 0) {
		cmd_start(cons, cmdline, memtotal);
	} else if (strcmp(cmdline, "setlog") == 0 && cons->sht != 0){
		cmd_setlog(cons);
	} else if (strncmp(cmdline, "show ", 5) == 0 && cons->sht != 0){
		cmd_show(cons, cmdline);
	} else if (strcmp(cmdline, "test") == 0 && cons->sht != 0){
		cmd_test(cons);
	} else if (strcmp(cmdline, "top") == 0 && cons->sht != 0) {
		cmd_top(cons);
	}
	else if(strcmp(cmdline, "stamp") == 0&& cons->sht != 0)
	{
		cmd_stamp(cons, cmdline);
	}else if (cmdline[0] != 0) {
		if (cmd_app(cons, fat, cmdline) == 0) {
			/* �R�}���h�ł͂Ȃ��A�A�v���ł��Ȃ��A����ɋ�s�ł��Ȃ� */
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
		/* �\����͂ɐ������Ă�����A�ړI�n�Ɉړ����� */
		cons->current_dir = dest_dinfo;
	}
	return;
}

/**
 * cd�R�}���h�ɗ^����ꂽ��������͂���(���݊J����)
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

/* cd�̈�������͂���֐�*/
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
				/*���΃p�X�Ƃ��ď���*/
				sprintf(s, "change directory using relative path\n");
				debug_print(s);
				dinfo = cons->current_dir;
				get_pathname(prev_pname, dinfo); //debug
			}
		}else if(cdline[cp] == '/'){
			debug_print("'/' has found.\n");
			cp++;
			if(cp == 1){
				/*��΃p�X�Ƃ��ď���*/
				sprintf(s, "change directory using absolute path\n");
				debug_print(s);
				dinfo = (struct MYDIRINFO *)ROOT_DIR_ADDR;
			}
		}else if(isDOUBLEPOINT()){
			debug_print("\"..\" has found.\n");
			cp += 2;
			if(cp == 2){
				/*���΃p�X�Ƃ��ď���*/
				sprintf(s, "change directory using relative path\n");
				debug_print(s);
				dinfo = cons->current_dir;
				get_pathname(prev_pname, dinfo);
			}

			if(dinfo->parent_dir == 0){
				cd_error(cons, "Can't move because here is ROOT directory.\n");
				debug_print("*********************************\n");
				return 0; // parse���s
			}
			dinfo = dinfo->parent_dir;
			goto PARSE_NEXT;
		}else{
			/*�G���[����*/
			cd_error(cons, "Incorrect initial character.\n");
			debug_print("*********************************\n");
			return 0;	// parse���s
		}

		if(isDOUBLEPOINT() || cdline[cp] == '\0'){
			// ".."�܂��͎����k�������Ȃ牽�����Ȃ�
		}else{
			/* �w�肳�ꂽ�f�B���N�g����T�� */
			get_dirname(dirname, cdline);
			finfo = myfinfo_search(dirname, dinfo, MAX_FINFO_NUM);
			if(finfo == 0){
				/* �Y������f�B���N�g����������Ȃ����� */
				cd_error(cons, "Can't find this directory.\n");
				debug_print("*********************************\n");
				return 0; // parse���s
			}else{
				/* �Y������f�B���N�g������������ */
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

	/*�f�B���N�g���̈ړ�*/
	//cons->current_dir = dinfo;
	debug_print("*********************************\n");
	cons_newline(cons);
	return dinfo;
}

/* �G���[�̏o�� */
void cd_error(struct CONSOLE *cons, char *message){
	char s[50];
	int i, j, k;

	get_pathname(s, cons->current_dir);
	for(i=0; s[i]!='\0'; i++)s[i] = ' ';	// �p�X�����̋�
	for(j=0; j<3; j++) s[i+j] = ' ';		// "cd "���̋�
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


/* �R�}���h���C����ŊȒP�ȃt�@�C���ҏW���s����֐�
 * (myfopen/myfread/myfwrite/myfclose�֐��̃e�X�g�p) */
/* �ҏW���[�h */
#define MODE_DEF	0x00
#define MODE_CLS	0x01
#define MODE_INS	0x02
#define MODE_ADD	0x04
#define MODE_OPEN	0x08
#define MODE_ALL	0xFF
void cmd_edit(struct CONSOLE *cons, char *cmdline){
	struct MYDIRINFO *dinfo = cons->current_dir;	// open�p
	struct MYFILEDATA *fdata;	// open�p
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
	while(cmdline[p] == ' ') p++; // �󔒂�ǂݎ̂Ă�

	if(cmdline[p] == '-'){
		/* option�t���̏ꍇ�Aoption���擾���� */
		temp_p=0;
		p++; // '-'��ǂݎ̂Ă�
		while(cmdline[p] != ' ' && cmdline[p] != 0){
			if(temp_p >= 10){
				/* option�̒������傫������ꍇ, �����I�� */
				sprintf(s, "option is too long.\n");
				cons_putstr(cons, s);
				cons_newline(cons);
				return;
			}
			option[temp_p] = cmdline[p];
			temp_p++;
			p++;
		}

		option[temp_p] = '\0';	// �I�[�L���̕t�^

		//sprintf(s, "option=%s[EOF]\n", option);
		//debug_print(s);

		/* OPEN MODE�̂ݍŏ��ɕ]������(�����if���ŕ���H���啝�Ɍ��邩��) */
		if(strcmp(option, "open") == 0){
			/* �w�肳�ꂽ�t�@�C�����J�� */
			debug_print("EDIT:open mode\n");
			mode = MODE_OPEN;
		}

		if(setfdata == 0 && mode != MODE_OPEN){
			/* �܂��ҏW�p�̃t�@�C���������ꍇ�A���������ɏI�� */
			sprintf(s, "can't edit: There is no file being opened.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}

		if(strcmp(option, "cls") == 0){
			/* �t�@�C�����N���A������Aeditline����͂��� */
			debug_print("EDIT:clear mode\n");
			mode = MODE_CLS;
			myfwrite(setfdata, "");	// �������Ȃ��ꍇ������̂ŁA�����ł��N���A����
		}else if(strcmp(option, "ins") == 0){
			/* �t�@�C���̏d�������̂ݏ㏑������ */
			debug_print("EDIT:insert mode\n");
			mode = MODE_INS;
		}else if(strcmp(option, "add") == 0){
			/* �t�@�C���̖�����editline��ǉ����� */
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
			// ���������ɏI��
		}else{
			/* ��O����(�����I��) */
			sprintf(s, "invalid option.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}
		if(cmdline[p] == ' ') p++; //�󔒂Ȃ�|�C���^�����ɐi�߂�

	}else if(setfdata == 0 && mode != MODE_OPEN){
		/* option���Ȃ��ꍇ�ɂ���O������K�p���� */
		/* �܂��ҏW�p�̃t�@�C���������ꍇ�A���������ɏI�� */
		sprintf(s, "Can't edit: There is no file being opened.\n");
		cons_putstr(cons, s);
		cons_newline(cons);
		return;
	}

	/* edit line�̎擾 */
	temp_p = 0;
	while(cmdline[p] != 0 && cmdline[p] != '\0'){
		if(temp_p >=48){
			/* �ҏW�����񂪒����ꍇ(����ł�cmdline����������50�����Ȃ�) */
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
	editline[temp_p] = '\0';	// �I�[�L���̕t�^
	/**** end of parser ****/

	/* �擾����option�̉����āAeditline�̕ҏW���[�h��ς��� */
	char temp[1024];	//�ҏW�p�̕���������1024����(�v����)
	if(mode == MODE_DEF || mode == MODE_ADD){
		/* default��add���[�h�Ɠ��� */
		myfread(s, setfdata);
		strcat(s, editline);
		debug_print("char *s out of myfwrite() is shown below.\n");
		debug_print(s);
		debug_print("\n");
		myfwrite(setfdata, s);

	}else if(mode == MODE_CLS){
		/* CLEAR SCREEN MODE
		 * �o�b�t�@�̓��e���������Aeditline�̓��e���������ށB*/
		myfread(temp, setfdata);
		sprintf(temp, "");
		strcat(temp, editline);
		myfwrite(setfdata, temp);	// �N���A&�㏑��

	}else if(mode == MODE_INS){
		int nullFlag = 0;
		/* myfwrite�ŔC�ӂ̈ꕶ����u��������֐���p�ӂ���K�v������(�I�I�I�v�����I�I�I) */
		myfread(temp, setfdata);
		for(i=0; i<length_editline; i++){
			if(editline[i] != ' '){
				/* �ҏW������ɉ��炩�̕��������͂���Ă���ꍇ */
				temp[i] = editline[i];
			}else if(temp[i] == '\0'){
				/* �t�@�C���f�[�^��EOF��������󔒂ɒu�������� */
				temp[i] = ' ';
				nullFlag = 1;
			}
		}
		if(nullFlag == 1)temp[i] = '\0';	// ������̖����Ƀk��������ǉ�
		sprintf(s, "INS MODE RESULT: %s\n", temp);
		debug_print(s);
		myfwrite(setfdata, temp);

	}else if(mode == MODE_OPEN){
		/* �t�@�C�����J�� */
		if(setfdata != 0){
			/* ���ɕҏW���̃t�@�C��������ꍇ�A���������ɏI�� */
			sprintf(s, "can't open: There is a file being opened.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
			return;
		}
		fdata = myfopen(editline, dinfo);
		if(fdata == 0){
			/* �t�@�C�����J���Ȃ������ꍇ */
			sprintf(s, "can't open \"%s\".\n", editline);
			cons_putstr(cons, s);
			cons_newline(cons);

		}else{
			/* �t�@�C��������ɊJ�����ꍇ */
			sprintf(s, "opened \"%s\" file.\n", editline);
			cons_putstr(cons, s);
			cons_newline(cons);
			setfdata = fdata;
		}
		return;
	}else{
		/* ��O���� */
		debug_print("unexpected error at edit mode.\n");
	}

	myfread(temp_body, setfdata);
	sprintf(s, "edit result:\n%s[EOF]\n", temp_body);
	cons_putstr(cons, s);

	/* �t�@�C���T�C�Y�̎擾 */
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

	/* �t�@�C���̍Ō���܂�i��i�߂� */
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		/* ����File������ȏ㑶�݂��Ȃ��ꍇ, break���� */
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}
	}

	/* �������͑啶���ɒ��� */
	for(j=0; dir_name[j] != 0; j++) {
		if ('a' <= dir_name[j] && dir_name[j] <= 'z') {
			dir_name[j] -= 'a'-'A';
		}
		// dir_name�̕����񕪂��ꕶ�����i�[����
		dinfo->finfo[i].name[j] = dir_name[j];
	}

	// �c��̕�������󔒂Ŗ��߂�
	for(; j<8 ;j++) dinfo->finfo[i].name[j] = ' ';

	// dir_name�̕����񕪂��ꕶ�����i�[����
	//for(j=0; dir_name[j] != 0; j++) dinfo->finfo[i].name[j] = dir_name[j];

	/* �t�@�C�����̐V�K�쐬 */
	dinfo->finfo[i].child_dir = get_newdinfo();	// ����mydir�̏ꏊ���i�[
	dinfo->finfo[i].clustno = 0;
	dinfo->finfo[i].date = 0;
	dinfo->finfo[i].type = 0x10;
	dinfo->finfo[i].size = sizeof(dinfo->finfo[i]);
	dinfo->finfo[i].fdata = 0;	// �O�F�t�@�C���f�[�^�����݂��Ȃ�

	/* �쐬�����t�@�C�����̏o�� */
	sprintf(s, "created directory: name = %s\n", dinfo->finfo[i].name);
	cons_putstr(cons, s);
	sprintf(s, "\tchild dir address = 0x%08x\n", dinfo->finfo[i].child_dir);
	debug_print(s);
	sprintf(s, "\ttype=0x%02x\n", dinfo->finfo[i].type);
	debug_print(s);

	/* �q�f�B���N�g����MYDIRINFO�Ń}�E���g����B(dinfo��child_dinfo�̐e�f�B���N�g��) */
	struct MYDIRINFO *child_dinfo = (struct MYDIRINFO *)dinfo->finfo[i].child_dir;
	child_dinfo->this_dir = dinfo->finfo[i].child_dir; // �q��addr �� �e��dir addr
	sprintf(child_dinfo->name, dir_name); // �f�B���N�g�������i�[
	child_dinfo->parent_dir = dinfo->this_dir; // �q�̐eaddr �� �eaddr

	/* �q�f�B���N�g�����̏o��(�f�o�b�O�p) */
	sprintf(s, "\tchild dinfo addr = 0x%08x\n", child_dinfo->this_dir);
	debug_print(s);
	sprintf(s, "\tchild dinfo name = %s\n", child_dinfo->name);
	debug_print(s);
	sprintf(s, "\tchild dinfo parent addr = 0x%08x\n", child_dinfo->parent_dir);
	debug_print(s);

	cons_newline(cons);	// ���s
	return;
}

/**
 * make file in my filesystem
 */
void cmd_mkfile(struct CONSOLE *cons, char *cmdline){
	/* (0x0010 + 0x0026)�̏ꏊ�ɂ������, FILEINFO�̃f�[�^�\���Ƃ���, ��������ɃR�s�[����. */
	struct MYDIRINFO *dinfo = cons->current_dir;
	int i, j;
	char s[50];
	char *name = cmdline + 7;
	char filename[12];
	struct MYFILEINFO *finfo;

	/* filename�̐��` */
	for (j = 0; j < 11; j++) {
		filename[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		/* �t�@�C�����̕��������𒴂����ꍇ�͉������Ȃ� */
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
				/* �������͑啶���ɒ��� */
				filename[j] -= 'a'-'A';
			}
			j++;
		}
	}

	/* ���ɓ����f�B���N�g���������݂��Ă���ꍇ, �������Ȃ�*/
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

	/***** �V�����t�@�C�����쐬����(����text�t�@�C���������Ȃ�) *****/
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		/* ����File������ȏ㑶�݂��Ȃ��ꍇ, break���� */
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}
	}

	/* �t�@�C�������������� */
	for(j=0; j<8; j++) dinfo->finfo[i].name[j] = filename[j];
	for(j=0; j<3; j++) dinfo->finfo[i].ext[j] = filename[8+j];
	dinfo->finfo[i].clustno = 0;
	dinfo->finfo[i].date = 0;
	dinfo->finfo[i].type = 0x20;	/* �t�@�C�������͓K����0x20�Ƃ���(���̂��Ȃ���0x20������)*/
	dinfo->finfo[i].size = 0;

	/* �t�@�C���f�[�^�̐��� */
	struct MYFILEDATA test;	//�v����
	test.head.stat = STAT_ALL;	// �V�K�t�@�C���쐬���̓X�e�[�^�X�r�b�g��S�ė��Ă�(�v����)
	test.head.this_dir = dinfo->this_dir;//�v����
	struct MYFILEDATA *fdata = get_newfdata(&test); // �V�K�f�[�^���擾�@//�v����
	strcpy(fdata->head.name, name);	// �t�@�C���f�[�^�ɂ����O���R�s�[("name"�ł��邱�Ƃɒ��ӁI)

	/***** debug *****/
	sprintf(s, "fdata->head.name =\t\t%s[EOF]\n", fdata->head.name);
	debug_print(s);
	sprintf(s, "dinfo->finfo[i].name =\t%s[EOF]\n", dinfo->finfo[i].name);
	debug_print(s);
	struct MYFILEINFO *debug = myfinfo_search(fdata->head.name, dinfo, MAX_FINFO_NUM);
	sprintf(s, "test(should not be 0) = %d\n", debug);
	debug_print(s);
	/*****************/
	fdata->head.stat = 0x01;	// valid bit�𗧂Ă�
	fdata->head.this_fdata = fdata; // �f�[�^�̈�̈�ԖڂɊm�ۂ���
	fdata->head.this_dir = dinfo->this_dir;
	dinfo->finfo[i].fdata = fdata;	// �f�t�H���g�͂O(�v�����I)

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
		/* �t�@�C�������������ꍇ */
		p = (char *) memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
		cons_putstr1(cons, p, finfo->size);
		memman_free_4k(memman, (int) p, finfo->size);
	} else {
		/* �t�@�C����������Ȃ������ꍇ */
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
 * show  status about a specific file.
 * @param cmdline is "show [file name]"
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
	// show detail file type info(������s�̏o�͕����͂����Ă���i�m�F�ς݁j)
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
			/* �t�@�C���f�[�^�����݂����ꍇ */
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
			cons_putstr(cons, "[EOF]\n");	// body���̍Ō�ɂ͉��s���Ȃ��̂ő}��
			sprintf(s, "size: %d\n", get_size_myfdata(finfo->fdata));
			cons_putstr(cons, s);
			cons_newline(cons);
		}else{
			/* �t�@�C���f�[�^���Ȃ��ꍇ(Ex. �f�B���N�g��) */
			sprintf(s, "\tthis is not file.\n");
			cons_putstr(cons, s);
			cons_newline(cons);
		}
	}

	/* show the strings in char s[100] */
	return;
}


/* FD�Ɋi�[����Ă���S�Ẵt�@�C���𒲂�, �\��������֐� */
void cmd_mkfs(struct CONSOLE * cons){
	struct MYDIRINFO dinfo;
	/* (0x0010 + 0x0026)�̏ꏊ�ɂ������, FILEINFO�̃f�[�^�\���Ƃ���, ��������ɃR�s�[����. */
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];

	sprintf(dinfo.name, "ROOT    ");
	dinfo.parent_dir = 0x00; // 0x00: root directory
	dinfo.this_dir = (struct MYDIRINFO *)ROOT_DIR_ADDR;
	cons->current_dir = (struct MYDIRINFO *)dinfo.this_dir;

	/* �Ƃ肠����10�t�@�C�������R�s�[���Ă݂� */
	for (i = 0; i < 10; i++) {
		/* ����File������ȏ㑶�݂��Ȃ��ꍇ, break���� */
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		/* 0xe5:���ɏ������ꂽ�t�@�C�� */
		if (finfo[i].name[0] != 0xe5) {
			/* 0x18 = 0x10 + 0x18
			 * 0x10:�f�B���N�g��
			 * 0x08:�t�@�C���ł͂Ȃ����(�f�B�X�N�̖��O�Ƃ�)
			 * ����āAFile Type��"�t�@�C��"�ł������ꍇ */
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j]; /* "filename"���t�@�C���l�[���ŏ㏑�� */
				}
				s[ 9] = finfo[i].ext[0];	/* "e"���㏑�� */
				s[10] = finfo[i].ext[1];	/* "x"���㏑�� */
				s[11] = finfo[i].ext[2];	/* "t"���㏑�� */

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
	/* (0x0010 + 0x0026)�̏ꏊ�ɂ������, FILEINFO�̃f�[�^�\���Ƃ���, ��������ɃR�s�[����. */
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
	// dir_num = get_newdinfo(cons);	// �f�o�b�O�̂��߂�&cons�����Ă���B

	// debug code: ���Ƀ}�E���g����MYDIRINFO�̔ԍ���\��
	//sprintf(s, "dinfo number = %d\n", dir_num);
	//cons_putstr(cons, s);

	//display files in current dir
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		/* ����File������ȏ㑶�݂��Ȃ��ꍇ, break���� */
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}

		/* 0xe5:���ɏ������ꂽ�t�@�C�� */
		if (dinfo->finfo[i].name[0] != 0xe5) {
			/* 0x18 = 0x10 + 0x18
			 * 0x10:�f�B���N�g��
			 * 0x08:�t�@�C���ł͂Ȃ����(�f�B�X�N�̖��O�Ƃ�)
			 */
			/* File Type��"�t�@�C��"�̏ꍇ */
			if ((dinfo->finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext\t%7d [FILE]\n", dinfo->finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = dinfo->finfo[i].name[j]; /* "filename"���t�@�C���l�[���ŏ㏑�� */
				}
				s[ 9] = dinfo->finfo[i].ext[0];	/* "e"���㏑�� */
				s[10] = dinfo->finfo[i].ext[1];	/* "x"���㏑�� */
				s[11] = dinfo->finfo[i].ext[2];	/* "t"���㏑�� */
				cons_putstr(cons, s);

				/* File Type��"�f�B���N�g��"�̏ꍇ */
			}else if((dinfo->finfo[i].type & 0x10) == 0x10){
				sprintf(s, "filename    \t%7d [DIR]\n", dinfo->finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = dinfo->finfo[i].name[j]; /* "filename"���t�@�C���l�[���ŏ㏑�� */
				}
				cons_putstr(cons, s);
				//sprintf(s, "test %s\t%d\t[DIR]",dinfo->finfo[i].name, dinfo->finfo[i].size);
				//cons_putstr(cons, s);
			}
		}
	}

	/* �ЂƂ��t�@�C����������Ȃ��������̏��� */
	if(i == 0){
		sprintf(s, "this directory has no file...\n");
		cons_putstr(cons, s);
	}
	cons_newline(cons);
	return;
}

/**
 * show file information in old filesystem
 */
void cmd_fddir(struct CONSOLE *cons)
{
	/* (0x0010 + 0x0026)�̏ꏊ�ɂ������, FILEINFO�̃f�[�^�\���Ƃ���, ��������ɃR�s�[����. */
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < MAX_FINFO_NUM; i++) {
		/* ����File������ȏ㑶�݂��Ȃ��ꍇ, break���� */
		if (finfo[i].name[0] == 0x00) {
			break;
		}

		/* 0xe5:���ɏ������ꂽ�t�@�C�� */
		if (finfo[i].name[0] != 0xe5) {
			/* 0x18 = 0x10 + 0x18
			 * 0x10:�f�B���N�g��
			 * 0x08:�t�@�C���ł͂Ȃ����(�f�B�X�N�̖��O�Ƃ�)
			 * ����āAFile Type��"�t�@�C��"�ł������ꍇ */
			if ((finfo[i].type & 0x18) == 0) {
				sprintf(s, "filename.ext   %7d\n", finfo[i].size);
				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j]; /* "filename"���t�@�C���l�[���ŏ㏑�� */
				}
				s[ 9] = finfo[i].ext[0];	/* "e"���㏑�� */
				s[10] = finfo[i].ext[1];	/* "x"���㏑�� */
				s[11] = finfo[i].ext[2];	/* "t"���㏑�� */
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
 * ??? delete console
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
	memman_free_4k(memman, (int) fat, 4 * 2880);
	io_cli();
	if (cons->sht != 0) {
		fifo32_put(fifo, cons->sht - shtctl->sheets0 + 768);	/* 768�`1023 */
	} else {
		fifo32_put(fifo, task - taskctl->tasks0 + 1024);	/* 1024�`2023 */
	}
	io_sti();
	for (;;) {
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
	/* �R�}���h���C���ɓ��͂��ꂽ��������A�ꕶ�����V�����R���\�[���ɓ��� */
	for (i = 6; cmdline[i] != 0; i++) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);	/* Enter */
	cons_newline(cons);
	return;
}

/**
 * ???
 */
void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal)
{
	struct TASK *task = open_constask(0, memtotal);
	struct FIFO32 *fifo = &task->fifo;
	int i;
	/* �R�}���h���C���ɓ��͂��ꂽ��������A�ꕶ�����V�����R���\�[���ɓ��� */
	for (i = 5; cmdline[i] != 0; i++) {
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
 * �R�}���h���C���̓��͕�����
 * API�ɂ��R�[���ł��邩�ǂ����𒲂ׂ�֐�
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

	/* �R�}���h���C������t�@�C�����𐶐� */
	for (i = 0; i < 13; i++) {
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0; /* �Ƃ肠�����t�@�C�����̌���0�ɂ��� */

	/* �t�@�C����T�� */
	finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 && name[i - 1] != '.') {
		/* ������Ȃ������̂Ō���".HRB"�����Ă�����x�T���Ă݂� */
		name[i    ] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	}

	if (finfo != 0) {
		/* �t�@�C�������������ꍇ */
		appsiz = finfo->size;	// �A�v���P�[�V�����̃T�C�Y���擾
		p = file_loadfile2(finfo->clustno, &appsiz, fat);
		if (appsiz >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
			/* �T�C�Y��36byte�ȏ� && Header == "Hari" && �t�@�C���̃��[�h�����������ꍇ */
			segsiz = *((int *) (p + 0x0000));	// �Z�O�����g�̃T�C�Y���擾
			esp    = *((int *) (p + 0x000c));	// ���W�X�^ESP���擾
			datsiz = *((int *) (p + 0x0010));	// �f�[�^�T�C�Y���擾
			dathrb = *((int *) (p + 0x0014));	// �f�[�^�̓��e���擾
			q = (char *) memman_alloc_4k(memman, segsiz);	// �������̈���m��
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
					/* �A�v�����J�����ςȂ��ɂ����������𔭌� */
					sheet_free(sht);	/* ���� */
				}
			}
			for (i = 0; i < 8; i++) {	/* �N���[�Y���ĂȂ��t�@�C�����N���[�Y */
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
	/* �t�@�C����������Ȃ������ꍇ */
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
	int *reg = &eax + 1;	/* eax�̎��̔Ԓn */
	/* �ۑ��̂��߂�PUSHAD�������ɏ��������� */
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
		sheet_updown(sht, shtctl->top); /* ���̃}�E�X�Ɠ��������ɂȂ�悤�Ɏw��F �}�E�X�͂��̏�ɂȂ� */
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
		ecx &= 0xfffffff0;	/* 16�o�C�g�P�ʂ� */
		memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
		break;
	case 9:
		ecx = (ecx + 0x0f) & 0xfffffff0; /* 16�o�C�g�P�ʂɐ؂�グ */
		reg[7] = memman_alloc((struct MEMMAN *) (ebx + ds_base), ecx);
		break;
	case 10:
		ecx = (ecx + 0x0f) & 0xfffffff0; /* 16�o�C�g�P�ʂɐ؂�グ */
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
					task_sleep(task);	/* FIFO����Ȃ̂ŐQ�đ҂� */
				} else {
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons->sht != 0) { /* �J�[�\���p�^�C�} */
				/* �A�v�����s���̓J�[�\�����o�Ȃ��̂ŁA�������͕\���p��1�𒍕����Ă��� */
				timer_init(cons->timer, &task->fifo, 1); /* ����1�� */
				timer_settime(cons->timer, 50);
			}
			if (i == 2) {	/* �J�[�\��ON */
				cons->cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* �J�[�\��OFF */
				cons->cur_c = -1;
			}
			if (i == 4) {	/* �R���\�[����������� */
				timer_cancel(cons->timer);
				io_cli();
				fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024);	/* 2024�`2279 */
				cons->sht = 0;
				io_sti();
			}
			if (i >= 256) { /* �L�[�{�[�h�f�[�^�i�^�X�NA�o�R�j�Ȃ� */
				reg[7] = i - 256;
				return 0;
			}
		}
		break;
	case 16:
		reg[7] = (int) timer_alloc();
		((struct TIMER *) reg[7])->flags2 = 1;	/* �����L�����Z���L�� */
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
		/* int api_fopen(char *fname):�t�@�C���̃I�[�v��
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
		/* void api_fclose(int fhandle): �t�@�C���̃N���[�Y
		 * EDX = 22
		 * EAX = file handle
		 */
		fh = (struct FILEHANDLE *) eax;
		memman_free_4k(memman, (int) fh->buf, fh->size);
		fh->buf = 0;
		break;
	case 23:
		/* api_fseek(int fhandle, int offset, int mode):�t�@�C���̃V�[�N
		 * EDX = 23
		 * EAX = file handle
		 * ECX = seek mode
		 * 		0:�V�[�N�̌��_�̓t�@�C���̐擪
		 * 		1:�V�[�N�̌��_�͌��݂̃A�N�Z�X�ʒu
		 * 		2:�V�[�N�̌��_�̓t�@�C����s�I�[
		 * EBX = �V�[�N��
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
		/* int api_fsize(int fhandle, int mode): �t�@�C���T�C�Y�̎擾
		 * EDX = 24
		 * EAX = filehandle
		 * ECX = �t�@�C���T�C�Y�擾���[�h
		 * 		0:���ʂ̃t�@�C���T�C�Y
		 * 		1:���݂̓ǂݍ��݈ʒu�̓t�@�C���퓬���牽�o�C�g�ڂ�
		 * 		2:�t�@�C���I�[����݂����݈ʒu�܂ł̃o�C�g��
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
		 * EBX = �o�b�t�@�̔Ԓn
		 * ECX = �ő�ǂݍ��݃o�C�g��
		 * EAX = return ����ǂ݂��߂��o�C�g�� */
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
		/* int api_cmdline(char *buf, int maxsize):�R�}���h���C���̎擾
		 * EDX = 26
		 * EBX = �R�}���h���C�����i�[����Ԓn
		 * ECX = ���o�C�g�܂Ŋi�[�ł��邩
		 * EAX = return ���o�C�g�i�[������ */
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
		/* inr api_getlang(void): langmode�̎擾
		 * EDX = 27
		 * EAX = return langmode;
		 */
		reg[7] = task->langmode;
		break;
	case 28:
		break;
	case 29:
		break;
	case 30:
		break;
	default :
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
	return &(task->tss.esp0);	/* �ُ�I�������� */
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
	return &(task->tss.esp0);	/* �ُ�I�������� */
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
		*cmdline=0;  //清空
	return;}

}

void cmd_stamp(struct CONSOLE *cons1, char *cmdline)
{
	struct TASK *task = task_now();
	cons_putstr0(cons1,"input 4 stamp values:");
	cons_newline(cons1);
	cmd_clearline(cons1,cmdline);//全都清空
	cmd_clearline(cons1,task->cmdline);//全都清空
	struct CONSOLE cons=*cons1;
	//cons_newline(&cons);
	cons_putchar(cons1, '>', 1);
	char *str;
	str = get_1_line(cons,cmdline);
	cons_putstr0(cons1,str);
	cons1->cur_y +=16;

    //  char result[20], * cmd_str;
    // int i, j, k, l, p, q;
    // int a[4];
    // static int s[1000];  /*�?�?*/
    // int x, y, r = 0, count = 0; //r用来做移动指针工�?,x指向数字开头，y指向连续数字结尾
    // for (cmd_str = str; *cmd_str <= ' ' || *cmd_str == 0; cmd_str++) {}	/* �X�y�[�X������܂œǂݔ�΂� */
    // for (; cmd_str[r]!=0; )
    // {

    //     if ('0' <= cmd_str[r] && cmd_str[r] <= '9')//�?数字
    //     {
    //         p = r;
    //         q = r + 1;
    //         a[count] = cmd_str[r] - '0';
    //         while ('0' <= cmd_str[q] && cmd_str[q] <= '9')
    //         {
    //             a[count] = 10 * a[count] + (cmd_str[q] - '0');
    //             q++;
    //         }
    //         r = q;  //新起�?
    //         count++; //count应�?�为4

    //     }
    //     else r++;

    // }
    // //scanf("%d %d %d %d", &a, &b, &c, &d);  /*输入四�?�面值邮�?*/
    // for(i=0; i<=5; i++)  /*�?�?变量i用于控制a分面值邮票的张数，最�?5�?*/
    //     for(j=0; i+j<=5; j++)  /*�?�?变量j用于控制b分面值邮票的张数，a分邮�?+b分邮票最�?5�?*/
    //         for(k=0; k+i+j<=5; k++)  /*�?�?变量k用于控制c分面值邮票的张数，a分邮�?+b分邮�?+c分邮票最�?5�?*/
    //             for(l=0; k+i+j+l<=5; l++)  /*�?�?变量l用于控制d分面值邮票的张数,a分邮�?+b分邮�?+c分邮�?+d分邮票最�?5�?*/
    //                 if( a[0]*i+a[1]*j+a[2]*k+a[3]*l )
    //                     s[a[0]*i+a[1]*j+a[2]*k+a[3]*l]++;
    // for(i=1; i<=1000; i++)
    //     if( !s[i] )
    //         break;

    // sprintf(result, "The max is %d\n", --i);
    // cons_putstr0(cons1,result);
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
		for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1 && cons.sht != 0) { /* �J�[�\���p�^�C�} */
				if (i != 0) {
					timer_init(cons.timer, &task->fifo, 0); /* ����0�� */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				} else {
					timer_init(cons.timer, &task->fifo, 1); /* ����1�� */
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
			}
			if (i == 2) {	/* �J�[�\��ON */
				cons.cur_c = COL8_FFFFFF;
			}
			if (i == 3) {	/* �J�[�\��OFF */
				if (cons.sht != 0) {
					boxfill8(cons.sht->buf, cons.sht->bxsize, COL8_000000,
						cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				}
				cons.cur_c = -1;
			}
			if (i == 4) {	/* �R���\�[���́u�~�v�{�^���N���b�N */
				cmd_exit(&cons, task->fat);
			}
			if (256 <= i && i <= 511) { /* �L�[�{�[�h�f�[�^�i�^�X�NA�o�R�j */
				if (i == 8 + 256) {
					/* �o�b�N�X�y�[�X */
					if (cons.cur_x > 16 + path_length * 8) {
						/* �J�[�\�����X�y�[�X�ŏ����Ă���A�J�[�\����1�߂� */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) {
					/* Enter */
					/* �J�[�\�����X�y�[�X�ŏ����Ă�����s���� */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - (path_length) - 2] = 0;
					cons_newline(&cons);
					if (cons.sht == 0) {
						cmd_exit(&cons, task->fat);
					}
					/* �v�����v�g�\�� */
					//cons_putchar(&cons, '>', 1);
					//cons_putstr0(&cons, cmdline);
					return cmdline;
					//break;
				} else {
					/* ��ʕ���? */
					if (cons.cur_x < 272) {
						/* �ꕶ���\�����Ă���A�J�[�\����1�i�߂� */
						cmdline[cons.cur_x / 8 - (path_length) - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			/* �J�[�\���ĕ\�� */
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