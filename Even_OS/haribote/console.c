/* --------------------------------
	B Y : S T O N
	HELO OS �����ļ�
	    ver. 1.0
         DATE : 2019-1-19  
----------------------------------- */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

void console_task(struct SHEET *sheet, int memtotal)
{
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	int i, *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	struct CONSOLE cons;
	struct FILEHANDLE fhandle[8];
	char cmdline[30];
	unsigned char *nihongo = (char *) *((int *) 0x0fe8);

	cons.sht = sheet;
	cons.cur_x =  8;
	cons.cur_y = 28;
	cons.cur_c = -1;
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
					cons_runcmd(cmdline, &cons, fat, memtotal);	/* �R�}���h���s */
					if (cons.sht == 0) {
						cmd_exit(&cons, fat);
					}
					/* �v�����v�g�\�� */
					cons_putchar(&cons, '>', 1);
				} else {
					/* ��ʕ��� */
					if (cons.cur_x < 240) {
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
			if (cons->cur_x == 8 + 240) {
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
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
		}
	}
	return;
}

void cons_newline(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	struct TASK *task = task_now();
	if (cons->cur_y < 28 + 112) {
		cons->cur_y += 16;
	} else {
		if (sheet != 0) {
			for (y = 28; y < 28 + 112; y++) {
				for (x = 8; x < 8 + 240; x++) {
					sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
				}
			}
			for (y = 28 + 112; y < 28 + 128; y++) {
				for (x = 8; x < 8 + 240; x++) {
					sheet->buf[x + y * sheet->bxsize] = COL8_000000;
				}
			}
			sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
		}
	}
	cons->cur_x = 8;
	if (task->langmode == 1 && task->langbyte1 != 0) {
		cons->cur_x = 16;
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
	} else if(strcmp(cmdline, "stamp") == 0&& cons->sht != 0)
	{
		cmd_stamp(cons, cmdline);
	}
	else if (strncmp(cmdline, "ls ", 3) == 0){
		cmd_open(cons, cmdline);  //打开文件内容
	}
	else if (cmdline[0] != 0) {
		if (cmd_app(cons, fat, cmdline) == 0) {
			cons_putstr0(cons, " û�д������\n Bad command !\n\n");
		}
	}
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

void cmd_cls(struct CONSOLE *cons)
{
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 128; y++) {
		for (x = 8; x < 8 + 240; x++) {
			sheet->buf[x + y * sheet->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cur_y = 28;
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
					if (cons.cur_x < 240) {
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

int get_rtc_register(char address)
{
	int value = 0;
	io_cli();
	io_out8(0x70, address);
	value = io_in8(0x71);
	io_sti();
	return value;
}

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
			reg[7] = memman_alloc((struct MEMMAN *) (ebx + ds_base), ecx);
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
		case 28:
			// Make sure an update isn't in progress
			while (get_rtc_register(RTC_REG_A) & 0x80);
			i = get_rtc_register(eax);
			// Convert BCD to binary values if necessary
			j = get_rtc_register(RTC_REG_B);
			if (!(j & 0x04)) 
				i = (i & 0x0F) + ((i / 16) * 10);
			// Convert 12 hour clock to 24 hour clock if necessary
			if (eax == RTC_HOURS && !(j & 0x02) && (i & 0x80))
				i = ((i & 0x7F) + 12) % 24;
			reg[7] = i;
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
