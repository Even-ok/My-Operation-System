/* �E�B���h�E�֌W */

#include "bootpack.h"

void make_window6(unsigned char *buf, int xsize, int ysize, char *title) //窗口绘制
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);

	boxfill8(buf, xsize, 0, 2, 22, 16, 36);
	boxfill8(buf, xsize, 1, 16, 22, 30, 36);
	boxfill8(buf, xsize, 2, 30, 22, 44, 36);
	boxfill8(buf, xsize, 3, 44, 22, 58, 36);
	boxfill8(buf, xsize, 4, 58, 22, 72, 36);
	boxfill8(buf, xsize, 5, 72, 22, 86, 36);
	boxfill8(buf, xsize, 6, 86, 22, 100, 36);
	boxfill8(buf, xsize, 9, 100, 22, 114, 36);
	boxfill8(buf, xsize, 10, 114, 22, 128, 36);
	boxfill8(buf, xsize, 11, 128, 22, 142, 36);
	boxfill8(buf, xsize, 12, 142, 22, 156, 36);
	//boxfill8(buf, xsize, 13, 156, 22, 170, 36);

	boxfill8(buf, xsize, 14, 2, 36, 16, 50);
	boxfill8(buf, xsize, 17, 16, 36, 30, 50);
	boxfill8(buf, xsize, 18, 30, 36, 44, 50);
	boxfill8(buf, xsize, 19, 44, 36, 58, 50);
	boxfill8(buf, xsize, 20, 58, 36, 72, 50);
    boxfill8(buf, xsize, 21, 72, 36, 86, 50);
	boxfill8(buf, xsize, 22, 86, 36, 100, 50);
	boxfill8(buf, xsize, 23, 100, 36, 114, 50);
	boxfill8(buf, xsize, 24, 114, 36, 128, 50);
	boxfill8(buf, xsize, 25, 128, 36, 142, 50);
	//boxfill8(buf, xsize, 26, 142, 36, 156, 50);
	boxfill8(buf, xsize, 7, 142, 36, 156, 50);
	//boxfill8(buf, xsize, 19, 33, 80, 49, 96);
	//boxfill8(buf, xsize, 20, 54, 80, 70, 96);
	//boxfill8(buf, xsize, 25, 75, 80, 91, 96);

	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

void make_window7(unsigned char *buf, int xsize, int ysize, char *title) //窗口绘制
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize, 24, 4, COL8_FFFFFF, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}


void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act)
{
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
	boxfill8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
	make_wtitle8(buf, xsize, title, act);
	return;
}

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act)
{
	static char closebtn[14][16] = {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};
	int x, y;
	char c, tc, tbc;
	if (act != 0) {
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	} else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize, 24, 4, tc, title);
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			} else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l)
{
	struct TASK *task = task_now();
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	if (task->langmode != 0 && task->langbyte1 != 0) {
		putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
		sheet_refresh(sht, x - 8, y, x + l * 8, y + 16);
	} else {
		putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
		sheet_refresh(sht, x, y, x + l * 8, y + 16);
	}
	return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void change_wtitle8(struct SHEET *sht, char act)
{
	int x, y, xsize = sht->bxsize;
	char c, tc_new, tbc_new, tc_old, tbc_old, *buf = sht->buf;
	if (act != 0) {
		tc_new  = COL8_FFFFFF;
		tbc_new = COL8_000084;
		tc_old  = COL8_C6C6C6;
		tbc_old = COL8_848484;
	} else {
		tc_new  = COL8_C6C6C6;
		tbc_new = COL8_848484;
		tc_old  = COL8_FFFFFF;
		tbc_old = COL8_000084;
	}
	for (y = 3; y <= 20; y++) {
		for (x = 3; x <= xsize - 4; x++) {
			c = buf[y * xsize + x];
			if (c == tc_old && x <= xsize - 22) {
				c = tc_new;
			} else if (c == tbc_old) {
				c = tbc_new;
			}
			buf[y * xsize + x] = c;
		}
	}
	sheet_refresh(sht, 3, 3, xsize, 21);
	return;
}

//制作菜单图层
void make_menu(struct SHEET *sht, int n)
{
	if(n == 1){
		//只保留了time  ball  flight  photo
		putfonts8_asc_sht(sht, 1, sht->bysize-18, COL8_000000, COL8_C6C6C6, "  reboot", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-36, COL8_000000, COL8_C6C6C6, "  shutdown", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-54, COL8_000000, COL8_C6C6C6, "background>", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-72, COL8_000000, COL8_C6C6C6, "  program >", 11);
		//putfonts8_asc_sht(sht, 1, sht->bysize-90, COL8_000000, COL8_C6C6C6, "  app     >", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-90, COL8_000000, COL8_C6C6C6, "  console", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-108, COL8_000000, COL8_C6C6C6, " user info ", 11);
	}else if(n == 2){
		putfonts8_asc_sht(sht, 1, sht->bysize-18, COL8_000000, COL8_C6C6C6, "  line", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-36, COL8_000000, COL8_C6C6C6, "  time", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-54, COL8_000000, COL8_C6C6C6, "  star", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-72, COL8_000000, COL8_C6C6C6, "  color", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-90, COL8_000000, COL8_C6C6C6, "  walk", 11);
	}else if(n == 3){
		putfonts8_asc_sht(sht, 1, sht->bysize-18, COL8_000000, COL8_C6C6C6, "  ...", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-36, COL8_000000, COL8_C6C6C6, "  ball", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-54, COL8_000000, COL8_C6C6C6, "  photo", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-72, COL8_000000, COL8_C6C6C6, "  flight", 11);
	}else if(n == 4){
		putfonts8_asc_sht(sht, 1, sht->bysize-18, COL8_000000, COL8_C6C6C6, "background1", 11);
		putfonts8_asc_sht(sht, 1, sht->bysize-36, COL8_000000, COL8_C6C6C6, "background2", 11);
	}
} 