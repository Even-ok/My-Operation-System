/* graphic.c, 将像素点阵转换为显存中的画面信息(调色板色号)的程序接口 */

#include "bootpack.h"

/* init_palette,
 * 为DAC调色板指定RGB颜色。*/
void init_palette(void)
{
	static unsigned char table_rgb[18 * 3] = {
		0x00, 0x00, 0x00,
		0xff, 0x00, 0x00,
		0x00, 0xff, 0x00,
		0xff, 0xcc, 0x33,
		0x00, 0x00, 0xff,
		0xff, 0x00, 0xff,
		0x00, 0xff, 0xff,
		0xff, 0xff, 0xff,
		0xc6, 0xc6, 0xc6,
		0x42, 0x73, 0xBF,
		0x00, 0x84, 0x00,
		0x84, 0x84, 0x00,
		0x00, 0x00, 0x84,
		0x84, 0x00, 0x84,
		0x00, 0x84, 0x84,
		0x84, 0x84, 0x84,
		0xFD, 0x5B, 0x46,
		0x43, 0x73, 0xBF
	};
	unsigned char table2[216 * 3];
	int r, g, b;
	set_palette(0, 17, table_rgb);
	for (b = 0; b < 6; b++) {
		for (g = 0; g < 6; g++) {
			for (r = 0; r < 6; r++) {
				table2[(r + g * 6 + b * 36) * 3 + 0] = r * 51;
				table2[(r + g * 6 + b * 36) * 3 + 1] = g * 51;
				table2[(r + g * 6 + b * 36) * 3 + 2] = b * 51;
			}
		}
	}
	set_palette(16, 231, table2);
	return;
}

/* set_palette,
 * 用rgb所指内存中包含的RGB颜色依次设置在调色板号[start, end]中。*/
void set_palette(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();
	io_cli(); 
	io_out8(0x03c8, start);
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);	
	return;
}

/* boxfill8,
 * 用色号c充当窗口[(x0,y0),(x1,y1)]区域画面信息,窗口x方向像素点数为xsize。
 * 
 * vram, 用于缓存窗口的画面信息;
 * [(x0,y0),(x1,y1)], 基于窗口左上角的窗口区域;
 * c, 填充窗口[(x0,y0),(x1,y1)]区域的色号。
 *
 * 当vram为显存基址,xsize为屏幕x箱数点时,屏幕[(x0,y0),(x1,y1)]区域将会直接显示色号c对应的RGB颜色。*/
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
			vram[y * xsize + x] = c;
	}
	return;
}

/* init_screen8,
 * 将自制屏幕背景窗口画面信息缓存到vram所指内存中。
 * x,y分别为屏幕横向(x)和纵向(y)的像素点数。*/
void init_screen8(char *vram, int x, int y)
{
	
	boxfill8(vram, x, COL8_008484,  0,     0,      x -  1, y - 29);
	set_picture(vram, x, y);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,     y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,     y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,     y - 24, 59,     y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,     y - 24,  2,     y -  4);
	boxfill8(vram, x, COL8_848484,  3,     y -  4, 59,     y -  4);
	boxfill8(vram, x, COL8_848484, 59,     y - 23, 59,     y -  5);
	boxfill8(vram, x, COL8_000000,  2,     y -  3, 59,     y -  3);
	boxfill8(vram, x, COL8_000000, 60,     y - 24, 60,     y -  3);
	
	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
	
	return;
}

int set_picture(unsigned char *vram, int x, int y)
{
	int i, j, x0, y0, fsize, info[4];
	unsigned char *filebuf, r, g, b;
	struct RGB *picbuf;
	struct MEMMAN *memman  = (struct MEMMAN *) MEMMAN_ADDR;
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FILEINFO *finfo;
	struct DLL_STRPICENV *env;
	int *fat;

	fat = (int *) memman_alloc_4k(memman, 4 * 2880);
	file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));
	finfo = file_search("heloos.jpg", (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 || (finfo->type & 0x18) != 0) {
		boxfill8(vram, x, COL8_FF0000, 100, 100, 300, 400);
		return -1;
	}
	fsize   = finfo->size;
	filebuf = (unsigned char *) memman_alloc_4k(memman, fsize);
	filebuf = file_loadfile2(finfo->clustno, &fsize, fat);

	env = (struct DLL_STRPICENV *) memman_alloc_4k(memman, sizeof(struct DLL_STRPICENV));
	if(info_JPEG(env, info, fsize, filebuf) == 0){
		return -1;			//		不是jpeg文件
	}
	picbuf  = (struct RGB *) memman_alloc_4k(memman, info[2] * info[3] * sizeof(struct RGB));
	decode0_JPEG(env, fsize, filebuf, 4, (unsigned char *) picbuf, 0);

	x0 = (int) ((x - info[2]) / 2);
	y0 = (int) ((y - info[3]) / 2);
	for (i = 0; i < info[3]; i++) {
		for (j = 0; j < info[2]; j++) {
			r = picbuf[i * info[2] + j].r;
			g = picbuf[i * info[2] + j].g;
			b = picbuf[i * info[2] + j].b;
			vram[(y0 + i) * x + (x0 + j)] =(unsigned char) rgb2pal(r, g, b, j, i, binfo->vmode);
		}
	}

	memman_free_4k(memman, (int) filebuf, fsize);
	memman_free_4k(memman, (int) picbuf , info[2] * info[3] * sizeof(struct RGB));
	memman_free_4k(memman, (int) env    , sizeof(struct DLL_STRPICENV));
	memman_free_4k(memman, (int) fat, 4 * 2880);
	return 0;
}

/* putfont8,
 * 将font所指像素点阵用c充当画面信息,
 * 并将画面信息从vram所指画面的(x,y)像素点位置处开始缓存。
 * 
 * font所指向的16字节是一个字符的像素点阵。
 * 
 * putfont8依次遍历font中的每一位,当遍历到bit位为1时,
 * 则将指定颜色号c写入vram所指内存的对应字节上。*/
void putfont8(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d /* data */;
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		d = font[i];
		if ((d & 0x80) != 0) { p[0] = c; }
		if ((d & 0x40) != 0) { p[1] = c; }
		if ((d & 0x20) != 0) { p[2] = c; }
		if ((d & 0x10) != 0) { p[3] = c; }
		if ((d & 0x08) != 0) { p[4] = c; }
		if ((d & 0x04) != 0) { p[5] = c; }
		if ((d & 0x02) != 0) { p[6] = c; }
		if ((d & 0x01) != 0) { p[7] = c; }
	}
	return;
}

/* putfonts8_asc,
 * 按照当前进程的语言模式(英文,日文等),根据s所指字符编码值
 * 从相应字库中取出字符的像素点阵, 将其转换为画面信息并从
 * vram所指画面(x,y)像素点处开始缓存;直到s所指字符编码值为0时结束。
 * 首先根据字符编码在字库中找到字符的像素点阵,然后根据字符像素点阵得到字符
 * 画面信息(色号c),最后将字符画面信息从vram所指画面(x,y)像素点处开始缓存。*/
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s)
{
	extern char hankaku[4096];
	struct TASK *task = task_now();
	char *nihongo = (char *) *((int *) 0x0fe8), *font;
	int k, t;

	if (task->langmode == 0) {
		for (; *s != 0x00; s++) {
			putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
			x += 8;
		}
	}
	if (task->langmode == 1) {
		for (; *s != 0x00; s++) {
			if (task->langbyte1 == 0) {
				if ((0x81 <= *s && *s <= 0x9f) || (0xe0 <= *s && *s <= 0xfc)) {
					task->langbyte1 = *s;
				} else {
					putfont8(vram, xsize, x, y, c, nihongo + *s * 16);
				}
			} else {
				if (0x81 <= task->langbyte1 && task->langbyte1 <= 0x9f) {
					k = (task->langbyte1 - 0x81) * 2;
				} else {
					k = (task->langbyte1 - 0xe0) * 2 + 62;
				}
				if (0x40 <= *s && *s <= 0x7e) {
					t = *s - 0x40;
				} else if (0x80 <= *s && *s <= 0x9e) {
					t = *s - 0x80 + 63;
				} else {
					t = *s - 0x9f;
					k++;
				}
				task->langbyte1 = 0;
				font = nihongo + 256 * 16 + (k * 94 + t) * 32;
				putfont8(vram, xsize, x - 8, y, c, font     );
				putfont8(vram, xsize, x    , y, c, font + 16);
			}
			x += 8;
		}
	}
	if (task->langmode == 2) {
		for (; *s != 0x00; s++) {
			if (task->langbyte1 == 0) {
				if (0x81 <= *s && *s <= 0xfe) {
					task->langbyte1 = *s;
				} else {
					putfont8(vram, xsize, x, y, c, nihongo + *s * 16);
				}
			} else {
				k = task->langbyte1 - 0xa1;
				t = *s - 0xa1;
				task->langbyte1 = 0;
				font = nihongo + 256 * 16 + (k * 94 + t) * 32;
				putfont8(vram, xsize, x - 8, y, c, font     );
				putfont8(vram, xsize, x    , y, c, font + 16);
			}
			x += 8;
		}
	}
		if (task->langmode == 3) {
		for (; *s != 0x00; s++) {
			if (task->langbyte1 == 0) {
				if (0xa1 <= *s && *s <= 0xfe) {
					task->langbyte1 = *s;
				}
				else {
					putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
				}
			}
			else {
				k = task->langbyte1 - 0xa1;
				t = *s - 0xa1;
				task->langbyte1 = 0;
				font = nihongo + (k * 94 + t) * 32;
				putfont32(vram, xsize, x - 8, y, c, font, font + 16);
			}
			x += 8;
		}
	}
	return;
}

/* init_mouse_cursor8,
 * 将鼠标画面信息(色号)缓存在mouse所指内存段中,
 * bc为鼠标所在背景的背景色号。*/
void init_mouse_cursor8(char *mouse, char bc)
{
	static char cursor[16][16] = {
		"**..............",
		"***.............",
		"*O**............",
		"*OO**...........",
		"*OOO**..........",
		"*OOOO**.........",
		"*OOOOO**........",
		"*OOOOOO**.......",
		"*OOOOOOO**......",
		"*OOOOOOOO**.....",
		"*OOOOOOOOO**....",
		"*OOOO********...",
		"*OOO**..........",
		"*O**............",
		"***.............",
		"**.............."
	};
	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}


/* putblock8_8,
 * 将buf所指内存段中的画面信息(如鼠标图标的色号)
 * 缓存在vram所指画面的[(px0, py0),(px0+pxsize, py0+pysize)]区域。*/
void putblock8_8(char *vram, int vxsize, int pxsize,
	int pysize, int px0, int py0, char *buf, int bxsize)
{
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}


unsigned short rgb2pal(int r, int g, int b, int x, int y, int cb)
{
	if (cb == 8) {
	static int table[4] = { 3, 1, 0, 2 };
	int i;
	x &= 1; 
	y &= 1;
	i = table[x + y * 2];
	r = (r * 21) / 256;
	g = (g * 21) / 256;
	b = (b * 21) / 256;
	r = (r + i) / 4;
	g = (g + i) / 4;
	b = (b + i) / 4;
	return((unsigned short) (16 + r + g * 6 + b * 36));
	} else {
	return((unsigned short) (((r << 8) & 0xf800) | ((g << 3) & 0x07e0) | (b >> 3)));
	}
}

void boxfilly(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x, y,r,xz,yz;
	xz=(x0+x1)/2;
	yz=(y0+y1)/2;
	r=(x1-x0)/2;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
		{
			if((x-xz)*(x-xz)+(y-yz)*(y-yz)<=r*r)
            vram[y * xsize + x] = c;
		}
			
	}
	//sheet_refresh(x0,y0,x1,y1);
	return;
}
void san(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
	int x,y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++)
		{
			if(y>=(y1-y0)*(x-x0)/(x1-x0)+y0)
            vram[y * xsize + x] = c;
		}
			
	}
	return;
}
void line(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1, int l)
{
	int x,y,flag=0;
	if(x0<x1&&y0<y1)
	{
		for (y = y0; y <= y1; y++) {
			for (x = x0; x <= x1; x++)
			{
				/*if(y>=y0&&x<=x1&&(y-y0)/(y1-5-y0)<=(x-x0-5)/(x1-x0-5)) flag=0;
				else if(y<=y1&&x>=x0&&(y-y0-5)/(y1-y0-5)<=(x-x0)/(x1-x0-5)) flag=0;
				else{
					
				}*/
				int y11=y1-l,x00=x0+l,y00=y0+l,x11=x1-l;
				if(y>=(y11-y0)*(x-x00)/(x1-x00)+y0&&y<=(y1-y00)*(x-x0)/(x11-x0)+y00)
				{
					vram[y * xsize + x] = c;
				}
				
			}
				
		}
	}
	if(x0<x1&&y0>y1)
	{
		for(y=y1;y<=y0;y++){
			for(x=x0;x<x1;x++)
			{
				int y00=y0-l,x11=x1-l,x00=x0+l,y11=y1+l;
				if(y>=(y1-y00)*(x-x0)/(x11-x0)+y00&&y<=(y11-y0)*(x-x00)/(x1-x00)+y0)
				{
					vram[y * xsize + x] = c;
				}
			}
		}
	}
	if(x0==x1)
	{
       if(y1>=y0)
	   {   
			for(y=y0;y<=y1;y++)
			{
				for(x=x0-l;x<=x0;x++)
				vram[y * xsize + x] = c;
			}
	   }
	   if(y1<y0)
	   {
		   for(y=y1;y<=y0;y++)
		   {
			   for(x=x0-l;x<=x0;x++)
			   vram[y * xsize + x] = c;
		   }
	   }
	}
	if(y0==y1)
	{
       if(x1>=x0)
	   { 
		   for(y=y0-l;y<=y0+l;y++)
		   {
				for(x=x0;x<=x1;x++)
				{
					vram[y * xsize + x] = c;
				}
		   }
	   }
	   if(x1<x0)
	   {
		   for(y=y0-l;y<=y0+l;y++)
		   {
				for(x=x1;x<=x0;x++)
				{
					vram[y * xsize + x] = c;
				}
		   }
	   }
	}
	return;
}


void putfont8_ch(char *vram, int xsize, int x, int y, char c, char *font)
{
	int i;
	char *p, d;
	for (i = 0; i < 16; i++)
	{
		p = vram + (y + i) * xsize + x;
		d = font[i * 2];
		if ((d & 0x80) != 0) p[0] = c;
		if ((d & 0x40) != 0) p[1] = c;
		if ((d & 0x20) != 0) p[2] = c;
		if ((d & 0x10) != 0) p[3] = c;
		if ((d & 0x08) != 0) p[4] = c;
		if ((d & 0x04) != 0) p[5] = c;
		if ((d & 0x02) != 0) p[6] = c;
		if ((d & 0x01) != 0) p[7] = c;
	}
	return;
}

void putfont32(char* vram, int xsize, int x, int y, char c, char* font1, char* font2)
{
	int i, k, j, f;
	char* p, d;
	j = 0;
	p = vram + (y + j) * xsize + x;
	j++;
	//上半部分
	for (i = 0; i < 16; i++)
	{
		for (k = 0; k < 8; k++)
		{
			if (font1[i] & (0x80 >> k))
			{
				p[k + (i % 2) * 8] = c;
			}
		}
		for (k = 0; k < 8 / 2; k++)
		{
			f = p[k + (i % 2) * 8];
			p[k + (i % 2) * 8] = p[8 - 1 - k + (i % 2) * 8];
			p[8 - 1 - k + (i % 2) * 8] = f;
		}
		if (i % 2)
		{
			p = vram + (y + j) * xsize + x;
			j++;
		}
	}
	//下半部分
	for (i = 0; i < 16; i++)
	{
		for (k = 0; k < 8; k++)
		{
			if (font2[i] & (0x80 >> k))
			{
				p[k + (i % 2) * 8] = c;
			}
		}
		for (k = 0; k < 8 / 2; k++)
		{
			f = p[k + (i % 2) * 8];
			p[k + (i % 2) * 8] = p[8 - 1 - k + (i % 2) * 8];
			p[8 - 1 - k + (i % 2) * 8] = f;
		}
		if (i % 2)
		{
			p = vram + (y + j) * xsize + x;
			j++;
		}
	}
	return;
}


