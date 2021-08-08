/* --------------------------------
	B Y : S T O N
	HELO OS ϵͳר��Դ����
	HELO OS �����ļ�
	    ver. 1.0
         DATE : 2019-1-19  
----------------------------------- */
/* copyright(C) 2019 PZK . */

#include "bootpack.h"


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

struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max)
{
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) { return 0; /* ������Ȃ����� */ }
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				/* �������͑啶���ɒ��� */
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
			return finfo + i; /* �t�@�C������������ */
		}
next:
		i++;
	}
	return 0; /* ������Ȃ����� */
}

char *file_loadfile2(int clustno, int *psize, int *fat)
{
	int size = *psize, size2;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	char *buf, *buf2;
	buf = (char *) memman_alloc_4k(memman, size);
	file_loadfile(clustno, size, buf, fat, (char *) (ADR_DISKIMG + 0x003e00));
	if (size >= 17) {
		size2 = tek_getsize(buf);
		if (size2 > 0) {	/* tek���k���������Ă��� */
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
			return 0; /* ������Ȃ����� */
		}
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				/* �������͑啶���ɒ��� */
				s[j] -= 'a'-'A';
			}
			j++;
		}
	}

	for (i = 0; i < max; ) {
		// �t�@�C�����������ꍇ�A����ȏ��Ƀt�@�C�����Ȃ��̂ŏ������I��������B
		if (dinfo->finfo[i].name[0] == 0x00) {
			break;
		}

		/* �t�@�C����������ꍇ, finfo��filetype�̎��ʂ����� */
		/* finfo���t�@�C���̏ꍇ(���͂Q��ނ����Ȃ�) */
		if (dinfo->finfo[i].type == 0x20) {
			for (j = 0; j < 11; j++) {
				if (dinfo->finfo[i].name[j] != s[j]) {
					goto next;
				}
			}
			//debug_print("file was found!\n");
			return dinfo->finfo + i; /* �t�@�C������������ */

			/* finfo���f�B���N�g���̏ꍇ */
		}else if(dinfo->finfo[i].type == 0x10){
			// �f�B���N�g���͊g���q���Ȃ��̂Ńt�@�C����������r����
			for (j = 0; j < 8; j++) {
				if (dinfo->finfo[i].name[j] != s[j]) {
					goto next;
				}
			}
			//debug_print("directory was found!\n");
			return dinfo->finfo + i; /* �f�B���N�g������������ */
		}
		next:
		i++;
	}

	debug_print("file/directory was not found in myfinfo_search()\n");
	return 0; /* ������Ȃ����� */
}


/**
 * �t�@�C�����Ǘ��̈悩��A�g���Ă��Ȃ��f�B���N�g����Ԃ�T���A�����������̂��A���̏ꏊ��Ԃ�
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

	// �L����dinfo������/�\������
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

	/* �V�����f�B���N�g���̏����� */
	new_dinfo = (dinfo + dir_num + 1);
	sprintf(new_dinfo->name, "");
	new_dinfo->parent_dir = 0;
	new_dinfo->this_dir = 0;

	return new_dinfo;
}

/* �t�@�C�����f�[�^�Ǘ��̈悩��R�s�[���āA�R�s�[��̔Ԓn��������
 * return struct MYFILEDATA: �R�s�[��̔Ԓn�Ɋi�[����Ă���t�@�C���f�[�^
 * return 0: ���s
 */
struct MYFILEDATA *myfopen(char *filename, struct MYDIRINFO *dinfo){
	// �Ƃ肠�������[�g�f�B���N�g���ɂ���t�@�C���ɑ΂��Ă̂ݎ��s���邱�Ƃɂ���B
	struct MYFILEINFO *finfo = myfinfo_search(filename, dinfo, 224);
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	unsigned int mem_addr;
	int i;
	unsigned int *temp_addr;
	int block_count, alloc_size;
	char s[BODY_SIZE + 128];
	if(finfo == 0 || (finfo->type & FTYPE_DIR) != 0){
		/* �Y������t�@�C����ROOT�f�B���N�g���ɑ��݂��Ȃ������ꍇ
		 * �܂��́A�Y������t�@�C�����f�B���N�g���ł������ꍇ */
		debug_print("In function myfopen(): file was not found.\n");
		return 0;
	}else{
		/* open�̂Ƃ��ɂ�STAT_OPENED�͊m�F���Ȃ� (�v����)*/
		add_status_myfdata(finfo->fdata, STAT_OPENED);
		sprintf(s, "fdata = 0x%08x\n[debug] head.this_fdata = 0x%08x\n[debug] head.this_dir = 0x%08x\n[debug] head.stat = 0x%02x\n",
				finfo->fdata,
				finfo->fdata->head.this_fdata,
				finfo->fdata->head.this_dir,
				finfo->fdata->head.stat);
		debug_print(s);
		sprintf(s, "body = %s[EOF]\n", finfo->fdata->body);
		debug_print(s);

		// �m�ۂ��郁�����̃T�C�Y���v�Z
		block_count = get_blocknum_myfdata(finfo->fdata);	// �d�l���Ă���u���b�N�����擾
		alloc_size = block_count * BLOCK_SIZE;		// �S�u���b�N���̕��������������m��

		/*** debug ***/
		sprintf(s, "alloc_size = 0x%08x\n", alloc_size);
		debug_print(s);
		/*************/

		mem_addr = memman_alloc(memman, alloc_size);

		/* �m�ۂ����̈�̏����� */
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

		/* �m�ۂ����������Ԓn�̏o�� */
		sprintf(s, "opened fdata addr = 0x%08x\n", opened_fdata);	// �ŏ���mem_addr�Ɠ����l
		debug_print(s);

		/* �������̈�̃R�s�[ */	// read -> write �Ŏ����ł���H -> head�̏�񂪕ۑ�����Ȃ�
		myfcopy(opened_fdata, finfo->fdata);
		add_status_myfdata(opened_fdata, STAT_BUF);	//�X�e�[�^�X�r�b�g��ǉ�����

		///* debug: �R�s�[�����������Ǘ��̈�(�����ŐF�X�ȍ�Ƃ��s��)
		sprintf(s, "allocated fdata addr = 0x%08x\n", mem_addr);
		debug_print(s);
		//sprintf(s, "allocated fdata length = 0x%08x + 0FFF[byte]\n", alloc_size);
		//debug_print(s);
		//*/
		return opened_fdata;
	}
	return 0;
}

/* �f�[�^�Ǘ��̈�̊Y���t�@�C�����I�[�v������Ă�����A��������������Astatus bit��opened������������B
 * return 0: if success
 * return -1: if failed
 * */
int myfclose(struct MYFILEDATA *opened_fdata){
	// �f�[�^�̈�ɂ�������̃f�[�^��fdata�Ɋi�[����
	struct MYFILEDATA *fdata =(struct MYFILEDATA *)opened_fdata->head.this_fdata;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	if((fdata->head.stat & STAT_OPENED) == 0){
		/* �I�[�v������Ă��Ȃ��t�@�C���̏ꍇ */
		debug_print("In function myfclose(): this file data is already closed.\n");
		return -1;	// close���s
	}else{
		/* �I�[�v������Ă���t�@�C���̏ꍇ�A���f�[�^�̃X�e�[�^�X�ϐ���ύX������A
		 * �m�ۂ��Ă������������������ */

		/* �X�e�[�^�X�ϐ��̕ύX(open bit��܂�) */
		fdata->head.stat &= (STAT_ALL - STAT_OPENED);

		/* �g�p���Ă����o�b�t�@�̈�̃�������� */
		memman_free_fdata(memman, (unsigned int)opened_fdata);

		return 0;	// close����
	}

	return -1;	// close���s
}

/* �f�[�^�Ǘ��̈�̊Y���t�@�C�����Z�[�u�\�Ȃ�΁Afdata->body�̓��e��ۑ�����
 * return 0: success
 * return -1: failed
 */
int myfsave(struct MYFILEDATA *opened_fdata){
	struct MYFILEDATA *fdata;
	struct MYDIRINFO *dinfo;
	struct MYFILEINFO *finfo;
	char s[1000];	// 1000�����ȏ�̃f�[�^�̏ꍇ�ǂ�����H

	/* �����̎擾 */
	fdata = opened_fdata->head.this_fdata;
	dinfo = fdata->head.this_dir;
	finfo = myfinfo_search(fdata->head.name, dinfo, MAX_FINFO_NUM);

	sprintf(s, "fdata->head.name = %s[EOF]\n", fdata->head.name);
	debug_print(s);
	if(finfo == 0){
		return -1;
	}


	if((fdata->head.stat & STAT_OPENED) == 0){
		/* �I�[�v������Ă��Ȃ��t�@�C���ɑ΂��ĕۑ����悤�Ƃ����ꍇ */
		sprintf(s, "In function myfsave():Can't save because this file data is not opened.\n");
		debug_print(s);
		return -1;	// close���s
	}else{
		myfread(s, opened_fdata);	// �o�b�t�@����t�@�C���f�[�^��ǂݍ���
		myfwrite(fdata, s);			// �ǂݍ��񂾃t�@�C���f�[�^����������
		finfo->size = get_size_myfdata(fdata);
		return 0;	// close����
	}

	return -1;	// close���s
}

/* �f�[�^�Ǘ��̈�̎g���Ă��Ȃ���Ԃ�T���A�����������̂��A���̏ꏊ��������
 * return ��������MYFILEDATA�̔Ԓn�A�h���X
 * [CAUTION!]���̊֐����ł̓t�@�C���f�[�^���m�̃����N�͓\��Ȃ�
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
		/* �V�K�t�@�C���쐬(mkfile�R�}���h)���̏��� */
		//debug_print("get_newfdata() was called by mkfile command.\n");
		goto MKFILE;

	}else if(ROOT_DATA_ADDR <= (unsigned int)fdata && (unsigned int)fdata < LAST_DATA_ADDR ){
		MKFILE:

		/* OPENED bit�����Ă��� = �f�[�^�Ǘ��̈�̂���t�@�C���V�K�f�[�^�̎擾 */
		/* �f�[�^�Ǘ��̈���̃t�@�C���f�[�^�ɑ΂��āA�Ăяo���ꂽ�ꍇ */
		/*
		debug_print("Getting new file data in data manage domain.\n");
		sprintf(s, "root_fdata = 0x%08x\n", root_fdata);
		debug_print(s);
		//*/

		temp_fdata = root_fdata;
		while((temp_fdata->head.stat & STAT_VALID) != 0){
			/* �f�[�^�Ǘ��̈����`�I�ɒT�����A�󂢂Ă���f�[�^�̈��T�� */
			/* temp_fdata��valid bit�������Ă���� */
			//sprintf(s, "fdata[%d] addr = 0x%08x\n", i, temp_fdata);
			//debug_print(s);
			temp_fdata += 1; // �ׂ̔Ԓn�Ɉړ�(���`�T���Ȃ̂ŁA�������x�͂߂��Ⴍ����x��[�v����])
			i++;
		}
		/* valid bit��0��file data���������� */
		new_fdata = temp_fdata;
		//sprintf(s, "found invalid fdata[%d] addr = 0x%08x\n", i, new_fdata);
		//debug_print(s);

		/* �t�@�C���f�[�^�̏����� */
		for(i=0; i< BODY_SIZE; i++)new_fdata->body[i] = '\0';
		new_fdata->head.stat = STAT_VALID;	// �����X�e�[�^�X��valid�̂ݗ����Ă�����
		new_fdata->head.this_fdata = new_fdata;	// �����̖{���̔Ԓn���L��(open���ɕK�v)
		new_fdata->head.this_dir = fdata->head.this_dir;
		new_fdata->head.next_fdata = 0;		// �ԕ��Ƃ��Ďg��
	}else{

		/* OPENED bit�������Ă��� = �o�b�t�@�̈�ɂ�����V�K�t�@�C���f�[�^�擾 */
		/* �������Ǘ��̈�ɂ���t�@�C���̏ꍇ�A
		 * �V�����������̈���m�ۂ��A�����ɐV�����t�@�C���f�[�^���쐬���� */
		//debug_print("Getting new file data in buffer domain.\n");

		// �m�ۂ��郁�����̃T�C�Y���v�Z�Ɗm��
		alloc_size = BLOCK_SIZE;	// �m�ۂ���T�C�Y���v�Z
		mem_addr = memman_alloc(memman, alloc_size);

		/* �m�ۂ����̈�̏����� */
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

		/* debug: �R�s�[�����������Ǘ��̈�(�����ŐF�X�ȍ�Ƃ��s��) */
		/* �m�ۂ����������Ԓn�̏o�� */
		//sprintf(s, "new fdata addr = 0x%08x\n", new_fdata);	// �ŏ���mem_addr�Ɠ����l
		//debug_print(s);
		//*/

		/* �t�@�C���f�[�^�̏����� */
		for(i=0; i< BODY_SIZE; i++)new_fdata->body[i] = '\0';
		new_fdata->head.stat = STAT_VALID | STAT_OPENED | STAT_BUF; // �����X�e�[�^�X��valid, opened, buf�������Ă�����
		new_fdata->head.this_fdata = new_fdata;	// �����̖{���̔Ԓn���L��(open���ɕK�v)
		new_fdata->head.this_dir = fdata->head.this_dir;
		new_fdata->head.next_fdata = 0;		// �ԕ��Ƃ��Ďg��
	}

	//sprintf(s, "/********************************/\n");
	//debug_print(s);
	return new_fdata;	/* �擾�����t�@�C���f�[�^��Ԃ� */
}

/**
 * interface to write data into file
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

	/* ���������� */
	i = j = 0;
	block_num = 1;
	prev_block_num = get_blocknum_myfdata(fdata);

	///* debugging
	sprintf(s, "myfwrite() has been called.\n");
	debug_print(s);
	//sprintf(s, "str = %s[EOF]\n", str);
	//debug_print(s);
	//*/

	while(str[i] != '\0'){
		fdata->body[j] = str[i];	// �t�@�C���f�[�^�Ɉꕶ����������
		i++;
		j++;

		if(i == (BODY_SIZE-1) * block_num){	// Ex. (108 * 1)-1 = 107
			/* �u���b�N�T�C�Y�̏���ɓ��B�����ꍇ */
			fdata->body[j] = '\0';	// fdata->body[107]�̍Ō�Ƀk������('\0')����͂���B
			debug_fdata = fdata;
			if(fdata->head.next_fdata == 0){
				/* �u���b�N��������Ȃ��Ƃ��͐V�����t�@�C���f�[�^�����A�g������ */
				new_fdata = get_newfdata(fdata);
				fdata->head.next_fdata = new_fdata;	// ���̃f�[�^�̔Ԓn���i�[
				fdata->head.stat |= STAT_CONT;		// �X�e�[�^�X�r�b�g��CONT�𗧂Ă�
			}
			fdata = fdata->head.next_fdata;	// ���̃f�[�^�ɐi��
			block_num++;
			sprintf(s, "block moved: 0x%08x -> 0x%08x\n", debug_fdata, fdata);
			debug_print(s);
			j=0;
		}
	}
	fdata->body[j] = str[i];	// fdata->body�̍Ō�Ƀk������('\0')����͂���B

	if(block_num < prev_block_num){
		/* ���̃u���b�N�T�C�Y���A�������񂾃u���b�N�T�C�Y���������ꍇ�A
		 * �g���Ȃ��Ȃ����u���b�N�������� or �������B */
		debug_print("***IN BLOCK DELETE FUNCTION***\n");

		temp_fdata = fdata->head.next_fdata;	//���̃t�@�C���f�[�^��ۑ��B
		/* ���[�̃t�@�C���f�[�^���C�� */
		fdata->head.stat &= (STAT_ALL - STAT_CONT);	//STAT_CONT�r�b�g��܂�
		fdata->head.next_fdata = 0;	//���̃t�@�C���f�[�^�Ԓn���㏑��

		if((fdata->head.stat & STAT_BUF) != 0){	//�������͗v�����I->�X�e�[�^�X�ϐ������΂����ƊȒP�ɂł���͂�
			/* �o�b�t�@�̈�ɂ���t�@�C���f�[�^�̏ꍇ�A�����t�@�C���f�[�^�̃���������� */
			debug_print("[Free fdata mode]\n");
			memman_free_fdata(memman, (unsigned int)temp_fdata);
		}else{
			/* �f�[�^�̈�̃t�@�C���f�[�^�̏ꍇ�A�����t�@�C���f�[�^�������� */
			debug_print("[Init fdata mode]\n");
			while(temp_fdata->head.next_fdata != 0){
				temp_fdata->head.stat = 0;	// �S�ẴX�e�[�^�X�r�b�g��܂�
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
 * interface to read data from file
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
	sprintf(str, "");	// str�̏�����

	do{ /* ���̃t�@�C���f�[�^�����݂����,�t�@�C���f�[�^��ǂݍ��ݑ����� */
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
 * interface to copy data
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
		memcpy(fdata1, fdata2, sizeof(struct MYFILEDATA));
		sprintf(s, "fdata->body[%d] was copied.\n", i);
		debug_print(s);
		i++;

		if(fdata2->head.next_fdata == 0){
			break;
		}else{
			/* �o�b�t�@�ł����g���Ȃ��Ƃ����O��(�v����) */
			fdata2 = fdata2->head.next_fdata;

			fdata1->head.next_fdata = fdata1 + 1;
			fdata1++;

			//if(fdata1->head.next_fdata == 0){
			//	/* �R�s�[����鑤�̃u���b�N�������E���}�����ꍇ */
			//	fdata1->head.next_fdata = get_newfdata(fdata1);
			//}
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

	/* �A�Ȃ��Ă���u���b�N�̐��𐔂��� */
	while((fdata->head.stat & STAT_CONT) != 0){
		/* �t�@�C���f�[�^�ɑ���������ꍇ */
		if(fdata->body[BODY_SIZE-2] != '\0'){
			/* �u���b�N�̍Ō������2�Ԗڂɕ���������̂�(1�Ԗڂ̓k������), �u���b�N�͖��t�Ɣ��f */
			fdata = fdata->head.next_fdata;
			block_count++;
		}else{
			/* �u���b�N�̍Ō���ɕ������Ȃ��̂�, ���̃u���b�N��EOF������Ɣ��f */
			break; // ��t�@�C����������J�E���g���Ȃ�(�o�b�t�@�v�Z�p)
		}
	}
	rest_size = get_size_str(fdata->body); // �Ō�̃u���b�N�̕�����T�C�Y���擾

	sprintf(s, "fdata->body = %s\n", fdata->body);
	debug_print(s);
	sprintf(s , "p = %d\n", rest_size);
	debug_print(s);

	filesize = (BODY_SIZE * block_count) + rest_size;
	return filesize;	// (�u���b�N�T�C�Y�~�u���b�N��) + �]�� [byte]
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
	return p;	// �P�ʂ̓o�C�g
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
	block_num = (data_size / BODY_SIZE) + 1;	// �u���b�N�̐����v�Z
	sprintf(s, "used block number = %d\n", block_num);
	debug_print(s);
	return block_num;
}

/**
 * �t�@�C���f�[�^�ɃX�e�[�^�X�r�b�g��ǉ�����B
 * �ǉ�����X�e�[�^�X�r�b�g�����ɗ����Ă����ꍇ�͎��s����B
 * @param fdata: �ǉ������t�@�C���f�[�^
 * @param stat: �ǉ��������X�e�[�^�X�r�b�g
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
			/* �ǉ�����X�e�[�^�X�r�b�g�����ɗ����Ă����ꍇ, ���s*/
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
