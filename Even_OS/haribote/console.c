/* --------------------------------
	B Y : S T O N
	HELO OS �����ļ�
	    ver. 1.0
         DATE : 2019-1-19  
----------------------------------- */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

struct CONSOLE *log;
int console_id=0;
struct MYFILEDATA *setfdata = 0;
unsigned int addrlist[100];
unsigned int sizelist[100];
int num = 0;

void debug_print(char *str){
	/*	�f�o�b�O�p�̏o�͂�����Ƃ���"//"������*/
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

void console_task(struct SHEET *sheet, int memtotal)
{
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i, *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	struct CONSOLE cons;
	struct FILEHANDLE fhandle[8];
	char cmdline[100];
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
	task->langmode = 3;//��ʾÿ�ζ�ѡ����

	if(cons.id == 1){
		//cmd_mkfs(&cons);	/* �����R���\�[���ɑ΂��ċ����I��mkfs���g�� */
	}else if(cons.id == 0){
		cmd_setlog(&cons);	/* ���O�p�R���\�[���ɑ΂��ă��O�o�͗p�̐ݒ���{�� */
	}
	path_length = cons_putdir(&cons);

	/* �v�����v�g�\�� */
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
			if (256 <= i && i <= 511) { /* �L�[�{�[�h�f�[�^�i�^�X�NA�o�R�j */
				if (i == 8 + 256) {
					/* �o�b�N�X�y�[�X */
					if (cons.cur_x > 16+path_length * 8 ) {
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
					cons_runcmd(cmdline, &cons, fat, memtotal);	/* �R�}���h���s */
					if (cons.sht == 0) {
						cmd_exit(&cons, fat);
					}
					path_length = cons_putdir(&cons);
					/* �v�����v�g�\�� */
					cons_putchar(&cons, '>', 1);
				} else {
					/* ��ʕ��� */
					if (cons.cur_x < 272) {
						/* �ꕶ���\�����Ă���A�J�[�\����1�i�߂� */
						cmdline[cons.cur_x / 8- (path_length)  - 2] = i - 256;
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
			if (cons->cur_x == 8 + 272) {
				cons_newline(cons);
			}
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;	/* 32�Ŋ���؂ꂽ��break */
			}
		}
	} else if (s[0] == 0x0a) {	/* ���s */
		cons_newline(cons);
	} else if (s[0] == 0x0d) {	/* ���A */
	} else {	/* ���ʂ̕��� */
		if (cons->sht != 0) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		}
		if (move != 0) {
			cons->cur_x += 8;
			if (cons->cur_x == 8 + 272) {
				cons_newline(cons);
			}
		}
	}
	return;
}

void cons_newline(struct CONSOLE *cons)
{
	int x, y, xmax, ymax;
	xmax = cons->sht->bxsize - 16;
	ymax = cons->sht->bysize - 37;
	struct SHEET *sheet = cons->sht;
	struct TASK *task = task_now();
	if (cons->cur_y < 28 + ymax - 16) {
		cons->cur_y += 16;
	} else {
		if (sheet != 0) {
			for (y = 28; y < 28 + ymax - 16; y++) {
				for (x = 8; x < 8 + xmax; x++) {
					sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
				}
			}
			for (y = 28 + ymax - 16; y < 28 + ymax; y++) {
				for (x = 8; x < 8 + 272; x++) {
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

void cons_putstr0(struct CONSOLE *cons, char *s)
{
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

void cons_putstr1(struct CONSOLE *cons, char *s, int l)
{
	int i;
	for (i = 0; i < l; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return;
}

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

//获得当前目录路径
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

void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, int memtotal)
{
	if (strcmp(cmdline, "mem") == 0 && cons->sht != 0) {
		cmd_mem(cons, memtotal);
	} else if (strcmp(cmdline, "cls") == 0 && cons->sht != 0) {
		cmd_cls(cons);
	} else if (strcmp(cmdline, "dir") == 0 && cons->sht != 0) {
		cmd_dir(cons);
	} else if (strcmp(cmdline, "exit") == 0) {
		cmd_exit(cons, fat);
	} else if (strncmp(cmdline, "start ", 6) == 0) {
		cmd_start(cons, cmdline, memtotal);
	} else if (strncmp(cmdline, "ncst ", 5) == 0) {
		cmd_ncst(cons, cmdline, memtotal);
	} else if (strncmp(cmdline, "langmode ", 9) == 0) {
		cmd_langmode(cons, cmdline);
	} 
	//以上是原有系统自带命令
	else if (strncmp(cmdline, "cat ", 4) == 0 && cons->sht != 0) {
		cmd_cat(cons, fat, cmdline);
	} else if (strncmp(cmdline, "cd ", 3) == 0){
		cmd_cd(cons, cmdline);
	} else if (strncmp(cmdline, "edit ", 5) == 0 && cons->sht != 0){
		cmd_edit(cons, cmdline);
	} else if (strcmp(cmdline, "fddir") == 0 && cons->sht != 0) {
		cmd_fddir(cons);
	} else if (strncmp(cmdline, "fview ", 6) == 0 && cons->sht != 0){
		cmd_fview(cons, cmdline);
	} else if (strcmp(cmdline, "log") == 0 && cons->sht != 0){
		cmd_log(cons);
	} else if (strcmp(cmdline, "logcls") == 0 && cons->sht != 0){
		cmd_logcls(cons);
	}
	else if(strcmp(cmdline, "stamp") == 0&& cons->sht != 0)
	{
		cmd_stamp(cons, cmdline);
	} else if (strcmp(cmdline, "mkfs") == 0 && cons->sht != 0){
		cmd_mkfs(cons);
	}else if (strncmp(cmdline, "mkdir ", 6) == 0){
		cmd_mkdir(cons, cmdline);
	} else if (strncmp(cmdline, "mkfile ", 7) == 0){
		cmd_mkfile(cons, cmdline);
	} else if (strcmp(cmdline, "memmap") == 0 && cons->sht != 0) {
		cmd_memmap(cons, memtotal);
	}
	else if (strcmp(cmdline, "setlog") == 0 && cons->sht != 0){
		cmd_setlog(cons);
	} else if (strncmp(cmdline, "show ", 5) == 0 && cons->sht != 0){
		cmd_show(cons, cmdline);
	} else if (strcmp(cmdline, "test") == 0 && cons->sht != 0){
		cmd_test(cons);
	}
	else if (strncmp(cmdline, "ls ", 3) == 0){
		cmd_open(cons, cmdline);  //打开文件内容
	}
	else if (strcmp(cmdline, "reader")== 0) {
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
	}
	else if (cmdline[0] != 0) {
		if (cmd_app(cons, fat, cmdline) == 0) {
			cons_putstr0(cons, " Bad command !\n\n");
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

	/* �f�B���N�g�����̕��������𒴂���/���ɓ����f�B���N�g���������݂��Ă���ꍇ�͉������Ȃ� */
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
	//strcpy(fdata->head.name, name);	// �t�@�C���f�[�^�ɂ����O���R�s�[("name"�ł��邱�Ƃɒ��ӁI)

	/***** debug *****/
	//sprintf(s, "fdata->head.name =\t\t%s[EOF]\n", fdata->head.name);
	//debug_print(s);
	//sprintf(s, "dinfo->finfo[i].name =\t%s[EOF]\n", dinfo->finfo[i].name);
	//debug_print(s);
	//struct MYFILEINFO *debug = myfinfo_search(fdata->head.name, dinfo, MAX_FINFO_NUM);
	//sprintf(s, "test(should not be 0) = %d\n", debug);
	//debug_print(s);
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


void cmd_mem(struct CONSOLE *cons, int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char s[60];
	sprintf(s, " �ܹ�   %dMB\n ���� %dKB\n\n", memtotal / (1024 * 1024), memman_total(memman) / 1024);
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

void cmd_cls(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 528; y++) {
		for (x = 8; x < 8 + 272; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 272, 28 + 528);
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

void cmd_dir(struct CONSOLE *cons)
{
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	int i, j;
	char s[30];
	for (i = 0; i < 224; i++) {
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
			}
		}
	}
	cons_newline(cons);
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

void cmd_ncst(struct CONSOLE *cons, char *cmdline, int memtotal)
{
	struct TASK *task = open_constask(0, memtotal);
	struct FIFO32 *fifo = &task->fifo;
	int i;
	for (i = 5; cmdline[i] != 0; i++) {
		fifo32_put(fifo, cmdline[i] + 256);
	}
	fifo32_put(fifo, 10 + 256);	/* Enter */
	cons_newline(cons);
	return;
}

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

void cmd_open(struct CONSOLE *cons, char *cmdline)
{
	struct TASK *task = task_now();
	char name[13];
	int p = 0,x;
	name[12] = 0;
	for(x=0;x<12;x++) //初始化
		name[x]=' ';

	for (x = 3; x < 17; x++) {  //控制第8个符号必须得是.
		if (cmdline[x] != 0) {
			if(cmdline[x]=='.'&&p<8)  //cmdline到了.，但是太短了，补空格
			{
				for(;p<8;p++)
					name[p]=' ';
				name[p]='.';
				p++;
			}
			else
			{
				name[p] = cmdline[x];
				if ('a' <= name[p] && name[p] <= 'z') {
                /* 将文件名转为大写字符 */
                	name[p] -= 0x20;
            } 
				p++;
			}
		}
		else 
		{break;}
	}
	name[p] = 0;
	//cons_putstr0(cons, name);

	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
	while (finfo->name[0] != 0) {
		char s[13];
		s[12] = 0;
		for(x=0;x<12;x++) //初始化
			s[x]=' ';
		int k;
		for (k = 0; k < 8; k++) {
			if (finfo->name[k] != 0) {
				s[k] = finfo->name[k];
			}else {
				break;
			}
		}

		int t = 0;
		s[k] = '.'; //k=8
		k++;
		for (t = 0; t < 3; t++) {
			s[k] = finfo->ext[t];
			k++;
		}

		//cons_putstr0(cons, s);

		if (strcmp(name, s) == 0) {
			//cons_putstr0(cons, name);
			int i;
			char *p = (char *) (finfo->clustno * 512 + 0x003e00 + ADR_DISKIMG);
			int sz = finfo->size;
			char exp[30];
			sprintf(exp,"clu:%d,size:%d",finfo->clustno,finfo->size);
			cons_putstr0(cons,exp);
			// for(i=0;i<10;i++)
			// 	cons_putchar(cons,p[i],1);
			//cons_putchar(cons,p[0],1);
			
			// int t = 0;
			// for (t = 0; t < sz; t++) {
			// 	cons_putchar(cons,p[t],1);  //需要移动
			// }

			break;
		}

		finfo++;
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
	cmd_clearline(cons1,cmdline);//全都清空
	cmd_clearline(cons1,task->cmdline);//全都清空
	struct CONSOLE cons=*cons1;
	cons_putchar(&cons, '>', 1);
	char *str;
	str = get_1_line(cons,cmdline);
	
	cons1->cur_y +=32;

     char result[20], * cmd_str;
    int i, j, k, l, p, q;
    int a[4];
    static int s[1000];  /*邮资*/
    int x, y, r = 0, count = 0; //r用来做移动指针工作,x指向数字开头，y指向连续数字结尾
    for (cmd_str = str; *cmd_str <= ' ' || *cmd_str == 0; cmd_str++) {}	/* �X�y�[�X������܂œǂݔ�΂� */
    for (; cmd_str[r]!=0; )
    {

        if ('0' <= cmd_str[r] && cmd_str[r] <= '9')//是数字
        {
            p = r;
            q = r + 1;
            a[count] = cmd_str[r] - '0';
            while ('0' <= cmd_str[q] && cmd_str[q] <= '9')
            {
                a[count] = 10 * a[count] + (cmd_str[q] - '0');
                q++;
            }
            r = q;  //新起点
            count++; //count应该为4

        }
        else r++;

    }
    //scanf("%d %d %d %d", &a, &b, &c, &d);  /*输入四种面值邮票*/
    for(i=0; i<=5; i++)  /*循环变量i用于控制a分面值邮票的张数，最多5张*/
        for(j=0; i+j<=5; j++)  /*循环变量j用于控制b分面值邮票的张数，a分邮票+b分邮票最多5张*/
            for(k=0; k+i+j<=5; k++)  /*循环变量k用于控制c分面值邮票的张数，a分邮票+b分邮票+c分邮票最多5张*/
                for(l=0; k+i+j+l<=5; l++)  /*循环变量l用于控制d分面值邮票的张数,a分邮票+b分邮票+c分邮票+d分邮票最多5张*/
                    if( a[0]*i+a[1]*j+a[2]*k+a[3]*l )
                        s[a[0]*i+a[1]*j+a[2]*k+a[3]*l]++;
    for(i=1; i<=1000; i++)
        if( !s[i] )
            break;

    sprintf(result, "The max is %d\n", --i);
    cons_putstr0(cons1,result);
	//cons_putstr0(cons1, cmdline);
	return;

// 		return;
}

char *get_1_line(struct CONSOLE cons, char *cmdline)
{
	struct TASK *task = task_now();
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
					if (cons.cur_x > 16) {
						/* �J�[�\�����X�y�[�X�ŏ����Ă���A�J�[�\����1�߂� */
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -= 8;
					}
				} else if (i == 10 + 256) {
					/* Enter */
					/* �J�[�\�����X�y�[�X�ŏ����Ă�����s���� */
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - 2] = 0;
					cons_newline(&cons);
					if (cons.sht == 0) {
						cmd_exit(&cons, task->fat);
					}
					/* �v�����v�g�\�� */
					//cons_putchar(&cons, '>', 1);
					//cons_putstr0(&cons, cmdline);
					break;
				} else {
					/* ��ʕ��� */
					if (cons.cur_x < 272) {
						/* �ꕶ���\�����Ă���A�J�[�\����1�i�߂� */
						cmdline[cons.cur_x / 8 - 2] = i - 256;
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
			task->ds_base = (int) q;
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
			cons_putstr0(cons, "Ӧ���ļ��򿪴��� ��\n.hrb file format error.\n");
		}
		memman_free_4k(memman, (int) p, appsiz);
		cons_newline(cons);
		return 1;
	}
	/* �t�@�C����������Ȃ������ꍇ */
	return 0;
}

#define RTC_SECOND			0x00
#define RTC_MINUTE			0x02
#define RTC_HOURS			0x04
#define RTC_WEEKDAY			0x06
#define RTC_DAY_OF_MONTH	0x07
#define RTC_MONTH 			0x08
#define RTC_YEAR			0x09
#define RTC_CENTURY			0x32
#define RTC_REG_A			0x0A
#define RTC_REG_B			0x0b

// int get_rtc_register(char address)
// {
// 	int value = 0;
// 	io_cli();
// 	io_out8(0x70, address);
// 	value = io_in8(0x71);
// 	io_sti();
// 	return value;
// }

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax)
{
	struct TASK *task = task_now();
	int ds_base = task->ds_base;
	struct CONSOLE *cons = task->cons;
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct SHEET *sht;
	struct FIFO32 *sys_fifo = (struct FIFO32 *) *((int *) 0x0fec);
	int *reg = &eax + 1; /* eax����ĵ�ַ*/
	/*ǿ�и�дͨ��PUSHAD�����ֵ*/
		/* reg[0] : EDI,   reg[1] : ESI,   reg[2] : EBP,   reg[3] : ESP */
		/* reg[4] : EBX,   reg[5] : EDX,   reg[6] : ECX,   reg[7] : EAX */
	int i, j;
	struct FILEINFO *finfo;
	struct FILEHANDLE *fh;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

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

	switch (edx) {
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
		case 5:
			sht = sheet_alloc(shtctl);
			sht->task = task;
			sht->flags |= 0x10;
			sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
			make_window8((char *) ebx + ds_base, esi, edi, (char *) ecx + ds_base, 0);
			sheet_slide(sht, ((shtctl->xsize - esi) / 2) & ~3, (shtctl->ysize - edi) / 2);
			sheet_updown(sht, shtctl->top); /*������ͼ��߶�ָ��Ϊ��ǰ�������ͼ��ĸ߶ȣ�����Ƶ��ϲ�*/
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
			ecx &= 0xfffffff0; /*��16�ֽ�Ϊ��λ*/
			memman_free((struct MEMMAN *) (ebx + ds_base), eax, ecx);
			break;
		case 9:
			ecx = (ecx + 0x0f) & 0xfffffff0; /*��16�ֽ�Ϊ��λ��λȡ��*/
			reg[7] = memman_alloc((struct MEMMAN *) (ebx + ds_base), ecx,0);
			break;
		case 10:
			ecx = (ecx + 0x0f) & 0xfffffff0; /*��16�ֽ�Ϊ��λ��λȡ��*/
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
						task_sleep(task); /* FIFOΪ�գ����߲��ȴ�*/
					} else {
						io_sti();
						reg[7] = -1;
						return 0;
					}
				}
				i = fifo32_get(&task->fifo);
				io_sti();
				if (i <= 1) { /*����ö�ʱ��*/
					/*Ӧ�ó�������ʱ����Ҫ��ʾ��꣬������ǽ��´���ʾ�õ�ֵ��Ϊ1*/
					timer_init(cons->timer, &task->fifo, 1); /*�´���Ϊ1*/
					timer_settime(cons->timer, 50);
				}
				if (i == 2) { /*���ON */
					cons->cur_c = COL8_FFFFFF;
				}
				if (i == 3) { /*���OFF */
					cons->cur_c = -1;
				}
				if (i == 4) { /*ֻ�ر������д���*/
					timer_cancel(cons->timer);
					io_cli();
					fifo32_put(sys_fifo, cons->sht - shtctl->sheets0 + 2024); /*2024��2279*/
					cons->sht = 0;
					io_sti();
				}
				if (i >= 256) { /*�������ݣ�ͨ������A����*/
					reg[7] = i - 256;
					return 0;
				}
			}
			break;
		case 16:
			reg[7] = (int) timer_alloc();
			((struct TIMER *) reg[7])->flags2 = 1; /*�����Զ�ȡ��*/
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
			fh = (struct FILEHANDLE *) eax;
			memman_free_4k(memman, (int) fh->buf, fh->size);
			fh->buf = 0;
			break;
		case 23:
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
			reg[7] = task->langmode;
			break;
		// case 28:
		// 	// Make sure an update isn't in progress
		// 	while (get_rtc_register(RTC_REG_A) & 0x80);
		// 	i = get_rtc_register(eax);
		// 	// Convert BCD to binary values if necessary
		// 	j = get_rtc_register(RTC_REG_B);
		// 	if (!(j & 0x04)) 
		// 		i = (i & 0x0F) + ((i / 16) * 10);
		// 	// Convert 12 hour clock to 24 hour clock if necessary
		// 	if (eax == RTC_HOURS && !(j & 0x02) && (i & 0x80))
		// 		i = ((i & 0x7F) + 12) % 24;
		// 	reg[7] = i;

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
	}
	return 0;
}

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

void wait(struct S *s,struct process *this_process,char * which_s){
	char buf[100];
	int eflags= io_load_eflags();
	io_cli();
	s->value--;
	if(s->value<0){
		//��ʾ˭�ڵȴ�
		sprintf(buf, "%s is waiting %s!\n", this_process->name, which_s);
	    cons_putstr0(this_process->task->cons, buf);

		//�����ȴ��б�
		if(s->list_last==NULL){//�б���
			s->list_first=this_process;
			s->list_last=this_process;
			s->list_last->next=NULL;
		}
		else{//�б�����
			s->list_last->next=this_process;
			s->list_last=s->list_last->next;
			s->list_last->next=NULL;
		}
		//����
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
		//�б�
		temp=s->list_first;

		if(s->list_first==s->list_last){//ֻ��һ�������ڵȴ������Ѻ��ȴ��б���
			s->list_first=s->list_last=NULL;
		}
		else
			s->list_first=s->list_first->next;
		//
		task_run(temp->task,-1,-1);//����ԭ����Ĭ�����ȼ�

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
		//�������������ٽ���
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

unsigned int char2int(char *cSize){//��char����ת��Ϊint������
	unsigned int iSize=0;
	char c;
	int i=0;
	while((c=cSize[i])!='\0'){
		iSize=iSize*10+c-'0';
		i++;
	}
	return iSize;
}
void cmd_mymem(char *cmdline){//������alloc�ڴ棺alloc mode size
	char cSize[10];
	int mode;//�����ڴ���ģʽ����0����1����

	unsigned int iSize;
	int i;
	unsigned int addr;

	char memsizebuf[100];

	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct TASK *task=task_now();
	//
	for(i=0;cmdline[i]!=' ';i++);

	mode=cmdline[i+1]-'0';//ȡ��ģʽ

	strcpy(cSize,&cmdline[i+3]);//ȡ����С
	iSize=char2int(cSize);
	//allocǰ��ʾ������С
	sprintf(memsizebuf,"before alloc %d:\n",iSize);
	cons_putstr0(task->cons,memsizebuf);
	for (i = 0; i < memman->frees; i++) {
		sprintf(memsizebuf,"NO.%d-size=%d  ",i,memman->free[i].size);
		cons_putstr0(task->cons,memsizebuf);
	}
	//alloc
	addr=memman_alloc((struct MEMMAN *) MEMMAN_ADDR,iSize,mode);
	//��alloc�Ŀ鶼�ǵ������freeʹ��
	addrlist[num]=addr;
	sizelist[num]=iSize;
	num++;
	//alloc֮����ʾ������С
	cons_putstr0(task->cons,"\nafter alloc:\n");
	for (i = 0; i < memman->frees; i++) {
		sprintf(memsizebuf,"NO.%d-size=%d  ",i,memman->free[i].size);
		cons_putstr0(task->cons,memsizebuf);
	}
	cons_putstr0(task->cons,"\n");
	//memman_free((struct MEMMAN *) MEMMAN_ADDR, addr,iSize);
}

void cmd_free(){//free֮ǰ��������alloc���ڴ�
	int i;
	for(i=0;i<num;i++){
		memman_free((struct MEMMAN *) MEMMAN_ADDR, addrlist[i],sizelist[i]);
	}
}

int share=0;
int sharenum=0;//ʹ�ù��������Ľ�������
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

//�����ߺ������߳���������������peterson�㷨
#define BUFFER_SIZE 100
int producer=0,consumer=1;
int flag[2]={0,0};//��ʾ�ĸ������������ٽ���
int turn;//��ʾ�ĸ����̿��Խ����ٽ���
int in=0,out=0,counter=0;
int buffer[BUFFER_SIZE];//������
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
			now_task=task_now();//����ѭ��������ֹ��������
			now_task->flags=2;
		}
	    // xnum++;
		   flag[producer]=1;
		   turn=consumer;
	   	 while(flag[consumer]==1&&turn==consumer)
		   xnum++;
		  //�ٽ���
			 temp=in;
			 outcome=rand();
		   buffer[in]=outcome;
		   in = (in + 1)%BUFFER_SIZE;
			 counter++;
		   flag[producer]=0;
			 //ʣ����
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
		  //�ٽ���
			temp=out;
			outcome=buffer[out];
			out=(out+1)%BUFFER_SIZE;
			counter--;
			flag[consumer]=0;
			//ʣ����
			sprintf(s,"in buffer %d,consume %d\n",temp+1,outcome);
			cons_putstr0(cons,s);
	}
	return ;
}

//peterson算法的进入区和退出区，这2个可以当成互斥锁来使用，但仅能用于2个进程之间
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