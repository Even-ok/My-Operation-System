#include "bootpack.h"
//#include "apilib.h"

int newBackGround(char *cmdline, int *fat, struct CONSOLE *cons)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl = (struct SHTCTL *) *((int *) 0x0fe4);
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct RGB *picbuf;
	struct FILEINFO *finfo;
	struct DLL_STRPICENV *env;
	unsigned char *vram = (shtctl->sheets[0])->buf;
	int x = binfo->scrnx;
	int y = binfo->scrny;
	int i, j, fsize, info[4],x0,y0;
	unsigned char *filebuf, r, g, b;
	char fileName[24];
	
	for (i = 6, j = 0; cmdline[i] != 0; i++) {
		fileName[j++] = cmdline[i];
	}
	fileName[j] = 0;
	
	finfo = file_search(fileName, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 || (finfo->type & 0x18) != 0) {
		fileName[j] = '.';
		fileName[j + 1] = 'J';
		fileName[j + 2] = 'P';
		fileName[j + 3] = 'G';
		fileName[j + 4] = 0;
		finfo = file_search(fileName, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
		if (finfo == 0 || (finfo->type & 0x18) != 0) 
		  return -1;
	}
	
	// fsize = finfo->size;
	// filebuf = (unsigned char *) memman_alloc_4k(memman, fsize);
	// filebuf = file_loadfile2(finfo->clustno, &fsize, fat);
	
	// env = (struct DLL_STRPICENV *) memman_alloc_4k(memman, sizeof(struct DLL_STRPICENV));
	// info_JPEG(env, info, fsize, filebuf);
	// picbuf = (struct RGB *) memman_alloc_4k(memman, info[2] * info[3] * sizeof(struct RGB));
	// decode0_JPEG(env, fsize, filebuf, 4, (unsigned char *) picbuf, 0);
	
	// for (i = 0; i < info[3] - 28; i++) {
	// 	for (j = 0; j < info[2]; j++) {
	// 		r = picbuf[i * info[2] +j].r;
	// 		g = picbuf[i * info[2] +j].g;
	// 		b = picbuf[i * info[2] +j].b;
	// 		vram[(i) * x + (j)] = rgb2pal(r, g, b, j, i);
	// 	}
	// }
	
	// memman_free_4k(memman, (int) filebuf, fsize);
	// memman_free_4k(memman, (int)picbuf, info[2] * info[3] * sizeof(struct RGB));
	// memman_free_4k(memman, (int)env, sizeof(struct DLL_STRPICENV));
	// sheet_refresh(shtctl->sheets[0], 0, 0, x, y);
	
	// return 0;

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

	// memman_free_4k(memman, (int) filebuf, fsize);
	// memman_free_4k(memman, (int) picbuf , info[2] * info[3] * sizeof(struct RGB));
	// memman_free_4k(memman, (int) env    , sizeof(struct DLL_STRPICENV));
	// memman_free_4k(memman, (int) fat, 4 * 2880);
	memman_free_4k(memman, (int) filebuf, fsize);
	memman_free_4k(memman, (int)picbuf, info[2] * info[3] * sizeof(struct RGB));
	memman_free_4k(memman, (int)env, sizeof(struct DLL_STRPICENV));
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
	putfonts8_asc_sht(shtctl->sheets[0], 8, binfo->scrny -20, COL8_000000, COL8_C6C6C6, "menu", 5);
	sheet_refresh(shtctl->sheets[0], 0, 0, x, binfo->scrny);
	return 0;
}
int InitIcon_xxx(struct SHEET *sht, int *fat, int number)
{
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char fileName[10];
	struct DLL_STRPICENV *env;
	struct FILEINFO *finfo;
	unsigned char *vram ;
	struct RGB *picbuf;
	int i, j, x, y, fsize, info[4];
	unsigned char *filebuf, r, g, b;
	
	if(number == 2){
		fileName[0] = 'I'; fileName[1] = 'C';
		fileName[2] = 'O'; fileName[3] = 'N';
		fileName[4] = '2'; fileName[5] = '.';
		fileName[6] = 'J'; fileName[7] = 'P';
		fileName[8] = 'G'; fileName[9] = 0;
	}else if(number == 3){
		fileName[0] = 'I'; fileName[1] = 'C';
		fileName[2] = 'O'; fileName[3] = 'N';
		fileName[4] = '3'; fileName[5] = '.';
		fileName[6] = 'J'; fileName[7] = 'P';
		fileName[8] = 'G'; fileName[9] = 0;
	}else if(number == 4){
		fileName[0] = 'I'; fileName[1] = 'C';
		fileName[2] = 'O'; fileName[3] = 'N';
		fileName[4] = '6'; fileName[5] = '.';
		fileName[6] = 'J'; fileName[7] = 'P';
		fileName[8] = 'G'; fileName[9] = 0;
	}

	finfo = file_search(fileName, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
	if (finfo == 0 || (finfo->type & 0x18) != 0) {
		return -1;
	}
	
	fsize = finfo->size;
	filebuf = (unsigned char *) memman_alloc_4k(memman, fsize);
	filebuf = file_loadfile2(finfo->clustno, &fsize, fat);
	
	env = (struct DLL_STRPICENV *) memman_alloc_4k(memman, sizeof(struct DLL_STRPICENV));
	i = info_JPEG(env, info, fsize, filebuf);
	picbuf = (struct RGB *) memman_alloc_4k(memman, info[2] * info[3] * sizeof(struct RGB));
	i = decode0_JPEG(env, fsize, filebuf, 4, (unsigned char *) picbuf, 0);
	
	vram = sht->buf;
	x = sht->bxsize; y = sht->bysize;
	for (i = 0; i < y; i++) {
		for (j = 0; j < x; j++) {
			r = picbuf[i * info[2] +j].r;
			g = picbuf[i * info[2] +j].g;
			b = picbuf[i * info[2] +j].b;
			vram[(i) * x + (j)] = rgb2pal(r, g, b, j, i);
		}
	}
	
	memman_free_4k(memman, (int) filebuf, fsize);
	memman_free_4k(memman, (int)picbuf, info[2] * info[3] * sizeof(struct RGB));
	memman_free_4k(memman, (int)env, sizeof(struct DLL_STRPICENV));
	sheet_refresh(sht, 0, 0, sht->bxsize, sht->bysize-35);
	return 1;
} 















































