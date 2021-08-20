/* file.c, 文件读取程序接口 */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

/* file_readfat,
 * 从软盘映像起始处img读取文件分配表即FAT到fat所指内存从。*/
void file_readfat(int *fat, unsigned char *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4) & 0xfff;
		j += 3;
	}
	return;
}

/* file_loadfile,
 * 从软盘映像(缓存软盘内容内存段)img中从读取size字节文件内容
 * 到buf所指内存段,clustno为文件的起始簇号,fat所指内存段为FAT。*/
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	for (;;) {
		if (size <= 512) {
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	return;
}

/* file_search,
 * 在finfo所指软盘映像的文件信息区域(共max个文件信息)中搜索name所指
 * 目标命名文件,若搜索成功则返回目标文件的文件信息首地址,失败则返回0。*/
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) { return 0; }
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				/* 转换成大写字母*/
				s[j] -= 0x20;
			}
			j++;
		}
	}
	for (i = 0; i < max; ) {
		if (finfo->name[0] == 0x00) {
			break;
		}
		if ((finfo[i].type & 0x18) == 0) {
			for (j = 0; j < 11; j++) {
				if (finfo[i].name[j] != s[j]) {
					goto next;
				}
			}
			return finfo + i; 
		}
		next:
		i++;
	}
	return 0; 
}

/**
 * 仿照原有系统finfo_search进行编写
 * search my file in my filesystem using name.
 * @name: Ex. "hoge.txt", "foo.hrb"
 * return dinfo addr if it succeeded.
 * return 0 if it failed.
 */
struct MYFILEINFO *myfinfo_search(char *name, struct MYDIRINFO *dinfo, int max)
{
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) {
			debug_print("file was not found in myfinfo_search(): int j is too high.\n");
			return 0; 
		}
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				/* 转化为大写字母*/
				s[j] -= 'a'-'A';
			}
			j++;
		}
	}

	for (i = 0; i < max; ) {
		// 文件找不到
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}

		/* 只有ftype为0x20才是能打开的文件*/
		
		if (dinfo->finfo[i].type == 0x20) {
			for (j = 0; j < 11; j++) {
				if (dinfo->finfo[i].name[j] != s[j]) {
					goto next;
				}
			}
			//debug_print("file was found!\n");
			return dinfo->finfo + i; 

			/* 是一个路径 */
		}else if(dinfo->finfo[i].type == 0x10){
			
			for (j = 0; j < 8; j++) {
				if (dinfo->finfo[i].name[j] != s[j]) {
					goto next;
				}
			}
			//debug_print("directory was found!\n");
			return dinfo->finfo + i; 
		}
		next:
		i++;
	}

	debug_print("file/directory was not found in myfinfo_search()\n");
	return 0; 
}

/* file_loadfile2,
 * 读取起始簇号为clustno文件内容,共度*psize字节。该函数返回
 * 所读文件内容在内存中的首地址和成功读取的字节数(*psize中)。*/
char *file_loadfile2(int clustno, int *psize, int *fat)
{
	int size = *psize, size2;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char *buf, *buf2;
	buf = (char *) memman_alloc_4k(memman, size);
	file_loadfile(clustno, size, buf, fat, (char *) (ADR_DISKIMG + 0x003e00));
	if (size >= 17) {
		size2 = tek_getsize(buf);
		if (size2 > 0) {	
			buf2 = (char *) memman_alloc_4k(memman, size2);
			tek_decomp(buf, buf2, size2);
			memman_free_4k(memman, (int) buf, size);
			buf = buf2;
			*psize = size2;
		}
	}
	return buf;
}


/**
 * 获取新的路径信息
 */
struct MYDIRINFO *get_newdinfo(){
	char s[50];
	struct MYDIRINFO *dinfo = (struct MYDIRINFO *)ROOT_DIR_ADDR;
	struct MYDIRINFO *temp_dinfo;
	struct MYDIRINFO *this_dinfo;
	struct MYDIRINFO *new_dinfo;
	int i = 0, dir_num = -1;

	///* debug code: Viewing behavior of get_newdinfo().
	sprintf(s, "/*** IN FUNCTION get_newdinfo() ***/\n");
	debug_print(s);
	//*/

	for(i=0, temp_dinfo = dinfo; temp_dinfo->this_dir != 0x00000000 ; i++, temp_dinfo++){
		dir_num++;
		this_dinfo = temp_dinfo->this_dir;

		///* debug code: Viewing behavior of get_newdinfo().
		sprintf(s, "dinfo[%d] addr = 0x%08x\n", i, this_dinfo);
		debug_print(s);
		//*/
	}
	///* debug code: Viewing behavior of get_newdinfo().
	sprintf(s, "dinfo[%d]->this_addr = 0x%08x\n", i, temp_dinfo->this_dir);
	debug_print(s);
	this_dinfo = temp_dinfo->this_dir;
	sprintf(s, "max available dir number is %d\n", dir_num);
	debug_print(s);
	sprintf(s, "/*********************************/\n");
	debug_print(s);
	//*/

	new_dinfo = (dinfo + dir_num + 1);
	sprintf(new_dinfo->name, "");
	new_dinfo->parent_dir = 0;
	new_dinfo->this_dir = 0;

	return new_dinfo;
}

/* 打开文件
 * return struct MYFILEDATA: successly find
 * return 0: can't find
 */
struct MYFILEDATA *myfopen(char *filename, struct MYDIRINFO *dinfo){
	struct MYFILEINFO *finfo = myfinfo_search(filename, dinfo, 224);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int mem_addr;
	int i;
	unsigned int *temp_addr;
	int block_count, alloc_size;
	char s[BODY_SIZE + 128];
	if(finfo == 0 || (finfo->type & FTYPE_DIR) != 0){
		//找不到文件，或者是该文件是路径
		debug_print("In function myfopen(): file was not found.\n");
		return 0;
	}else{
		/* 把文件的状态该成STAT_OPENED*/
		add_status_myfdata(finfo->fdata, STAT_OPENED);
		sprintf(s, "fdata = 0x%08x\n[debug] head.this_fdata = 0x%08x\n[debug] head.this_dir = 0x%08x\n[debug] head.stat = 0x%02x\n",
				finfo->fdata,
				finfo->fdata->head.this_fdata,
				finfo->fdata->head.this_dir,
				finfo->fdata->head.stat);
		debug_print(s);
		sprintf(s, "body = %s[EOF]\n", finfo->fdata->body);
		debug_print(s);

		// 获得该文件内存分配情况
		block_count = get_blocknum_myfdata(finfo->fdata);
		alloc_size = block_count * BLOCK_SIZE;	

		/*** debug ***/
		sprintf(s, "alloc_size = 0x%08x\n", alloc_size);
		debug_print(s);
		/*************/

		mem_addr = memman_alloc(memman, alloc_size);

		/* 获得文件内容所在地址 */
		temp_addr = (unsigned int *)mem_addr;
		sprintf(s ,"INIT temp_addr = 0x%08x\n", temp_addr);
		for(i = 0; i<alloc_size; i++){
			if(*temp_addr != 0){
				//sprintf(s, "temp_addr = 0x%08x: %d\n", temp_addr, *temp_addr);
				//debug_print(s);
			}
			*temp_addr = 0;
			temp_addr += 1;
		}
		debug_print("initializing memory domain was finished.\n");

		struct MYFILEDATA *opened_fdata = (struct MYFILEDATA *) mem_addr;

		sprintf(s, "opened fdata addr = 0x%08x\n", opened_fdata);	
		debug_print(s);

		myfcopy(opened_fdata, finfo->fdata);
		add_status_myfdata(opened_fdata, STAT_BUF);	

		sprintf(s, "allocated fdata addr = 0x%08x\n", mem_addr);
		debug_print(s);
		//sprintf(s, "allocated fdata length = 0x%08x + 0FFF[byte]\n", alloc_size);
		//debug_print(s);
		//*/
		return opened_fdata;
	}
	return 0;
}

/* 关闭文件，若之前未保存操作，则不会自动保存
 * return 0: if success
 * return -1: if failed
 * */
int myfclose(struct MYFILEDATA *opened_fdata){
	struct MYFILEDATA *fdata =(struct MYFILEDATA *)opened_fdata->head.this_fdata;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	if((fdata->head.stat & STAT_OPENED) == 0){
		/* 文件的状态已经是被关闭的状态 */
		debug_print("In function myfclose(): this file data is already closed.\n");
		return -1;	// close
	}else{
		
		/* 通过十六进制运算，设定其值为已关闭 */
		fdata->head.stat &= (STAT_ALL - STAT_OPENED);

		
		memman_free_fdata(memman, (unsigned int)opened_fdata);

		return 0;	// close成功
	}

	return -1;	// close失败
}

/* 
 * return 0: success
 * return -1: failed
 */
int myfsave(struct MYFILEDATA *opened_fdata){
	struct MYFILEDATA *fdata;
	struct MYDIRINFO *dinfo;
	struct MYFILEINFO *finfo;
	struct TASK *task = task_now();

	char s[200];	// 1000


	fdata = opened_fdata->head.this_fdata;
	dinfo = fdata->head.this_dir;
	finfo = myfinfo_search(fdata->head.name, dinfo, MAX_FINFO_NUM);

	sprintf(s, "fdata->head.name = %s[EOF]\n", fdata->head.name);
	debug_print(s);
	if(finfo == 0){
		return -1;
	}


	if((fdata->head.stat & STAT_OPENED) == 0){
		/* 检查该文件状态 */
		sprintf(s, "In function myfsave():Can't save because this file data is not opened.\n");
		debug_print(s);
		return -1;	
	}else{
		myfread(s, opened_fdata);	// 读取文件内容到s中
		myfwrite(fdata, s);			// 把s中内容写入到fdata中
		finfo->size = get_size_myfdata(fdata);
		return 0;	
	}

	return -1;	
}

/* 获取创建文件时新文件的内容
 * return MYFILEDATA
 * Ex. fdata->head.next_fdata = new_fdata;
 */
struct MYFILEDATA *get_newfdata(struct MYFILEDATA *fdata){
	struct MYFILEDATA *root_fdata = (struct MYFILEDATA *) ROOT_DATA_ADDR;
	struct MYFILEDATA *temp_fdata;
	struct MYFILEDATA *new_fdata;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	//char s[100];
	unsigned int mem_addr, alloc_size;
	unsigned int *temp_addr;
	int i=0;

	/*
	sprintf(s, "*** IN FUNCTION get_newfdata() ***\n");
	debug_print(s);
	sprintf(s, "fdata addr = 0x%08x\n", (unsigned int)fdata);
	debug_print(s);
	//*/

	if(fdata->head.stat == STAT_ALL){
		/* 现在是mkfile打开该文件的状态 */
		//debug_print("get_newfdata() was called by mkfile command.\n");
		goto MKFILE;

	}else if(ROOT_DATA_ADDR <= (unsigned int)fdata && (unsigned int)fdata < LAST_DATA_ADDR ){
		MKFILE:   //创建新文件

		/* OPENED bit */
		/*
		debug_print("Getting new file data in data manage domain.\n");
		sprintf(s, "root_fdata = 0x%08x\n", root_fdata);
		debug_print(s);
		//*/

		temp_fdata = root_fdata;
		while((temp_fdata->head.stat & STAT_VALID) != 0){
			/* 该文件是无效文件 */
			//sprintf(s, "fdata[%d] addr = 0x%08x\n", i, temp_fdata);
			//debug_print(s);
			temp_fdata += 1; 
			i++;
		}
		/* valid bit */
		new_fdata = temp_fdata;
		//sprintf(s, "found invalid fdata[%d] addr = 0x%08x\n", i, new_fdata);
		//debug_print(s);

		/* 到一个文件的最大体长度*/
		for(i=0; i< BODY_SIZE; i++)new_fdata->body[i] = '\0';
		new_fdata->head.stat = STAT_VALID;
		new_fdata->head.this_fdata = new_fdata;	
		new_fdata->head.this_dir = fdata->head.this_dir;
		new_fdata->head.next_fdata = 0;		// 该文件内容置空
	}else{

		/* OPENED bit*/
		//debug_print("Getting new file data in buffer domain.\n");

		
		alloc_size = BLOCK_SIZE;	
		mem_addr = memman_alloc(memman, alloc_size);

		temp_addr = (unsigned int *)mem_addr;
		for(i = 0; i<alloc_size; i++){
			if(*temp_addr != 0){
				//sprintf(s, "temp_addr = 0x%08x: %d\n", temp_addr, *temp_addr);
				//debug_print(s);
			}
			*temp_addr = 0;
			temp_addr += 1;
		}
		//debug_print("initlizing memory domain was finished.\n");

		new_fdata = (struct MYFILEDATA *) mem_addr;

		//sprintf(s, "new fdata addr = 0x%08x\n", new_fdata);	
		//debug_print(s);
		//*/

		for(i=0; i< BODY_SIZE; i++)new_fdata->body[i] = '\0';
		new_fdata->head.stat = STAT_VALID | STAT_OPENED | STAT_BUF; 
		new_fdata->head.this_fdata = new_fdata;
		new_fdata->head.this_dir = fdata->head.this_dir;
		new_fdata->head.next_fdata = 0;	
	}

	//sprintf(s, "/********************************/\n");
	//debug_print(s);
	return new_fdata;	
}

/**
 * @param fdata: string data to be written
 * @param str: string data to write
 * return 1 if it succeeded
 * return 0 if it failed
 */
int myfwrite(struct MYFILEDATA *fdata, char *str){
	char s[BODY_SIZE + BODY_SIZE_OFFSET];
	int i,j;
	unsigned int block_num, prev_block_num;
	unsigned int file_size;
	struct MYFILEDATA *debug_fdata;
	struct MYFILEDATA *temp_fdata;
	struct MYFILEDATA *new_fdata;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	
	i = j = 0;
	block_num = 1;
	prev_block_num = get_blocknum_myfdata(fdata);//获得其以前所在的位置

	///* debugging
	sprintf(s, "myfwrite() has been called.\n");
	debug_print(s);
	//sprintf(s, "str = %s[EOF]\n", str);
	//debug_print(s);
	//*/

	while(str[i] != '\0'){
		fdata->body[j] = str[i];
		i++;
		j++;

		if(i == (BODY_SIZE-1) * block_num){	// Ex. (108 * 1)-1 = 107
			fdata->body[j] = '\0';	// fdata->body[107]
			debug_fdata = fdata;
			if(fdata->head.next_fdata == 0){
				new_fdata = get_newfdata(fdata);
				fdata->head.next_fdata = new_fdata;
				fdata->head.stat |= STAT_CONT;
			}
			fdata = fdata->head.next_fdata;
			block_num++;
			sprintf(s, "block moved: 0x%08x -> 0x%08x\n", debug_fdata, fdata);
			debug_print(s);
			j=0;
		}
	}
	fdata->body[j] = str[i];

	if(block_num < prev_block_num){
		debug_print("***IN BLOCK DELETE FUNCTION***\n");

		temp_fdata = fdata->head.next_fdata;
		fdata->head.stat &= (STAT_ALL - STAT_CONT);
		fdata->head.next_fdata = 0;	

		if((fdata->head.stat & STAT_BUF) != 0){	
			debug_print("[Free fdata mode]\n");
			memman_free_fdata(memman, (unsigned int)temp_fdata);
		}else{
			debug_print("[Init fdata mode]\n");
			while(temp_fdata->head.next_fdata != 0){
				temp_fdata->head.stat = 0;
				temp_fdata = temp_fdata->head.next_fdata;
			}
		}
		debug_print("******************************\n");
	}

	file_size = get_size_myfdata(fdata);
	sprintf(s, "In current block, %d/%d is used.\n", file_size, BODY_SIZE);
	debug_print(s);
	return 1;
}

/**
 * read data from file
 * @param str: string data to read
 * @param body: string data to be read
 * return 1: succeeded
 * return 0: failed
 */
int myfread(char *str, struct MYFILEDATA *fdata){
	struct MYFILEDATA *prev_fdata;
	char s[BODY_SIZE + BODY_SIZE_OFFSET];
	int i=0;
	sprintf(s, "myfread() has been called.\n");
	debug_print(s);
	sprintf(str, "");	// str初始化

	do{ /* 把文件body中的内容都读出到str中 */
		prev_fdata = fdata;
		sprintf(s, "fdata->body[%d] = %s[EOF]\n", i, fdata->body);
		debug_print(s);
		strcat(str, fdata->body);
		fdata = fdata->head.next_fdata;
		i++;
	}while(prev_fdata->head.next_fdata != 0);

	sprintf(s, "read data = %s[EOF]\n", str);
	debug_print(s);
	return 1;
}

/**
 * copy data
 * @param fdata1: file data to be copied
 * @param fdata2: file data to copy
 * return 1: succeeded
 * return 0: failed
 */
int myfcopy(struct MYFILEDATA *fdata1, struct MYFILEDATA *fdata2){
	char s[50];
	int i=0;
	sprintf(s, "myfcopy() has been called.\n");
	debug_print(s);

	for(;;){
		memcpy(fdata1, fdata2, sizeof(struct MYFILEDATA));//内存拷贝函数
		//即从源fdata1中拷贝n个字节到目标fdata2中
		sprintf(s, "fdata->body[%d] was copied.\n", i);
		debug_print(s);
		i++;

		if(fdata2->head.next_fdata == 0){
			break;
		}else{
			/* 指针移动 */
			fdata2 = fdata2->head.next_fdata;

			fdata1->head.next_fdata = fdata1 + 1;
			fdata1++;

		}
	}

	return 1;
}




/**
 * calculate size of file data, and return a result[byte].
 * @param fdata: file data
 */
unsigned int get_size_myfdata(struct MYFILEDATA *fdata){
	char s[BODY_SIZE + BODY_SIZE_OFFSET];
	unsigned int block_count = 0;
	unsigned int filesize;
	int rest_size = 0;

	while((fdata->head.stat & STAT_CONT) != 0){
		if(fdata->body[BODY_SIZE-2] != '\0'){
			fdata = fdata->head.next_fdata;
			block_count++;
		}else{
			break;
		}
	}
	rest_size = get_size_str(fdata->body);

	sprintf(s, "fdata->body = %s\n", fdata->body);
	debug_print(s);
	sprintf(s , "p = %d\n", rest_size);
	debug_print(s);

	filesize = (BODY_SIZE * block_count) + rest_size;
	return filesize;
}

/**
 * calculate size of char*, and return a result[byte].
 * @param str: char *
 * return data size of "char *str"
 */
unsigned int get_size_str(char *str){
	int p;
	p=0;
	while(str[p] != '\0') p++;
	return p;
}

/**
 * calculate block number, and return the number.
 * @param fdata
 * return block number
 */
unsigned int get_blocknum_myfdata(struct MYFILEDATA *fdata){
	unsigned int block_num, data_size;
	char s[50];	// for debugging
	data_size = get_size_myfdata(fdata);
	block_num = (data_size / BODY_SIZE) + 1;	
	sprintf(s, "used block number = %d\n", block_num);
	debug_print(s);
	return block_num;
}

/**
 * 
 * @param fdata: 文件流
 * @param stat: 要更改的状态
 * return 1 if it succeeded.
 * return 0 if it failed.
 */
unsigned int add_status_myfdata(struct MYFILEDATA *fdata, unsigned char stat){
	struct MYFILEDATA *temp_fdata, *prev_temp_fdata;
	char s[100];
	unsigned char debug_char;
	temp_fdata = fdata;
	do{
		prev_temp_fdata = temp_fdata;
		if((temp_fdata->head.stat & stat) == stat){
			/* 已经是这个状态了，无需改变s*/
			debug_print("***adding status bit is already valid.***\n");
			return 0;
		}
		debug_char = temp_fdata->head.stat;
		temp_fdata->head.stat |= stat;

		///* debug: output status bits */
		sprintf(s, "head.stat is changed: 0x%02x -> 0x%02x\n", debug_char, temp_fdata->head.stat);
		debug_print(s);
		//*/
		temp_fdata = temp_fdata->head.next_fdata;
	}while(prev_temp_fdata->head.next_fdata != 0);

	debug_print("status bit was added correctly.\n");
	return 1;
}
