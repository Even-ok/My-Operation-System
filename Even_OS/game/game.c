#include <stdio.h>
#include <string.h>	/* strlen */
#include "apilib.h"

void drawmap(int win, unsigned char* s1, char* s);
void trans(unsigned char* s1, char* s2);
int rand(void);
int step(int win);
void noteinfo(int win, char* note);

void HariMain(void)
{
	unsigned char i;
	unsigned char s1[66];
	char* buf, s2[146], note[30], note1[20], key;
	int flag = 0, A, B, theOne, theOther, temp1, temp2;
	int win;

	api_initmalloc();
	buf = api_malloc(336 * 261);
	win = api_openwin(buf, 336, 261, -1, "bingo");
	win = api_openwin(buf, 336, 261, -1, "bingo");
	api_boxfilwin(win, 6, 27, 329, 254, 0);

	sprintf(note, "��������\0");
	api_putstrwin(win, 6 + 16 * 8, 27 + 1 * 16, 7, 8, note);
	sprintf(note, "��:��ͨ��\0");
	api_putstrwin(win, 6, 27 + 2 * 16, 7, 10, note);
	sprintf(note, "����ը��-����5��\0");
	api_putstrwin(win, 6, 27 + 3 * 16, 7, 18, note);
	sprintf(note, "������ʯ-����5��\0");
	api_putstrwin(win, 6, 27 + 4 * 16, 7, 18, note);
	sprintf(note, "�ħ��-����λ��\0");
	api_putstrwin(win, 6, 27 + 5 * 16, 7, 18, note);
	sprintf(note, "�⣺Ů���\0");
	api_putstrwin(win, 6, 27 + 6 * 16, 7, 10, note);
	sprintf(note, "�᣺�����\0");
	api_putstrwin(win, 6, 27 + 7 * 16, 7, 10, note);
	sprintf(note, "�ǣ���һ��\0");
	api_putstrwin(win, 6, 27 + 8 * 16, 7, 12, note);
	sprintf(note, "���յ�\0");
	api_putstrwin(win, 6, 27 + 9 * 16, 7, 8, note);
	sprintf(note, "���س�����ʼ��Ϸ\0");
	api_putstrwin(win, 6, 27 + 10 * 16, 7, 16, note);
	for (;;) {
		key = api_getkey(1);
		if (key == 0x0a) {
			break;
		}
	}

	api_boxfilwin(win, 6, 27, 329, 254, 0);
	for (i = 0; i < 66; i++) {
		if (i == 10 || i == 20 || i == 30) {
			s1[i] = 3;
		}
		else if (i == 3 || i == 12 || i == 32 || i == 34 || i == 60) {
			s1[i] = 2;
		}
		else if (i == 6 || i == 33 || i == 41 || i == 51 || i == 64) {
			s1[i] = 1;
		}
		else {
			s1[i] = 0;
		}
	}
	s1[0] = 6;
	s1[65] = 9;
	drawmap(win, s1, s2);

again:
	A = 0;
	B = 0;
next:
	if (flag == 0) {
		theOne = A;
		theOther = B;
		flag = 1;
	}
	else {
		theOne = B;
		theOther = A;
		flag = 0;
	}
	temp1 = step(win);
	theOne += temp1;
check:
	switch (theOne) {
	case 10:
	case 20:
	case 30:
		temp2 = theOne;
		theOne = theOther;
		theOther = temp2;
		sprintf(note, "����λ�ã����س�������\0");
		break;
	case 3:
	case 12:
	case 32:
	case 34:
	case 60:
		theOne += 5;
		if (theOne >= 65) {
			theOne = 65;
		}
		sprintf(note, "ǰ��5�񣡰��س�������\0");
		break;
	case 6:
	case 33:
	case 41:
	case 51:
	case 64:
		theOne -= 5;
		if (theOne <= 0) {
			theOne = 0;
		}
		sprintf(note, "����5�񣡰��س�������\0");
		break;
	default:
		sprintf(note, "ǰ��%d�񣡰��س�������\0", temp1);
		break;
	}

	if (flag == 1) {
		A = theOne;
		B = theOther;
	}
	else {
		B = theOne;
		A = theOther;
	}
	for (i = 0; i < 66; i++) {
		if (i == 10 || i == 20 || i == 30) {
			s1[i] = 3;
		}
		else if (i == 3 || i == 12 || i == 32 || i == 34 || i == 60) {
			s1[i] = 2;
		}
		else if (i == 6 || i == 33 || i == 41 || i == 51 || i == 64) {
			s1[i] = 1;
		}
		else {
			s1[i] = 0;
		}
	}
	if (A == B) {
		s1[A] = 6;
	}
	else {
		s1[A] = 4;
		s1[B] = 5;
	}
	drawmap(win, s1, s2);

	noteinfo(win, note);


	sprintf(note1, "A:%d  B:%d\0", A, B);
	api_boxfilwin(win, 6, 27 + 10 * 16, 329, 27 + 11 * 16, 0);
	api_putstrwin(win, 6, 27 + 10 * 16, 7, strlen(note1), note1);


	for (;;) {
		key = api_getkey(1);
		if (key == 0x0a) {
			if (
				theOne == 10 || theOne == 20 || theOne == 30 ||
				theOne == 3 || theOne == 12 || theOne == 32 || theOne == 34 || theOne == 60 ||
				theOne == 6 || theOne == 33 || theOne == 41 || theOne == 51 || theOne == 64
				) {
				goto check;
			}
			else if (theOne == 65) {
				goto win;
			}
			else {
				goto next;
			}

		}
		else if (key == 'Q' || key == 'q') {
			goto end;
		}
	}
win:
	if (flag == 1) {
		sprintf(note, "Ů��ʤ�������س�������\0");
	}
	else {
		sprintf(note, "����ʤ�������س�������\0");
	}

	noteinfo(win, note);
	for (;;) {
		key = api_getkey(1);
		if (key == 0x0a) {
			goto again;
		}
		else if (key == 'Q' || key == 'q') {
			goto end;
		}
	}

end:
	api_closewin(win);
	api_end();
}





void drawmap(int win, unsigned char* s1, char* s2) {

	trans(s1, s2);

	api_boxfilwin(win, 6, 27, 329, 254, 0);

	api_putstrwin(win, 6, 27, 7, 40, s2 + 0);

	api_putstrwin(win, 6 + 19 * 16, 27 + 1 * 16, 7, 2, s2 + 21 * 2);
	api_putstrwin(win, 6 + 19 * 16, 27 + 2 * 16, 7, 2, s2 + 23 * 2);
	api_putstrwin(win, 6 + 19 * 16, 27 + 3 * 16, 7, 2, s2 + 25 * 2);

	api_putstrwin(win, 6, 27 + 4 * 16, 7, 40, s2 + 27 * 2);

	api_putstrwin(win, 6, 27 + 5 * 16, 7, 2, s2 + 48 * 2);
	api_putstrwin(win, 6, 27 + 6 * 16, 7, 2, s2 + 50 * 2);
	api_putstrwin(win, 6, 27 + 7 * 16, 7, 2, s2 + 52 * 2);

	api_putstrwin(win, 6, 27 + 8 * 16, 7, 40, s2 + 54 * 2);

	return;

}

void noteinfo(int win, char* note) {
	int i = strlen(note);
	api_boxfilwin(win, 6, 27 + 9 * 16, 329, 27 + 10 * 16, 0);
	api_putstrwin(win, 6, 27 + 9 * 16, 7, i, note);
}

void trans(unsigned char* s1, char* s2) {
	int i, n = 0, temp;
	for (i = 0; i < 74; i++) {

		if (i == 20 || i == 22 || i == 24 || i == 26 || i == 47 || i == 49 || i == 51 || i == 53) {
			s2[i * 2] = '\0';

		}
		else {
			temp = n;
			if (temp >= 23 && temp <= 42) {

				temp = 23 + (42 - temp);

			}
			switch (s1[temp]) {

			case 0:
				s2[i * 2] = 0xa1;
				s2[i * 2 + 1] = 0xf0;
				break;
			case 1:
				s2[i * 2] = 0xa1;
				s2[i * 2 + 1] = 0xf8;
				break;
			case 2:
				s2[i * 2] = 0xa1;
				s2[i * 2 + 1] = 0xf4;
				break;
			case 3:
				s2[i * 2] = 0xa1;
				s2[i * 2 + 1] = 0xef;
				break;
			case 4:
				s2[i * 2] = 0xa1;
				s2[i * 2 + 1] = 0xe2;
				break;
			case 5:
				s2[i * 2] = 0xa1;
				s2[i * 2 + 1] = 0xe1;
				break;
			case 6:
				s2[i * 2] = 0xa1;
				s2[i * 2 + 1] = 0xc7;
				break;
			default:
				s2[i * 2] = 0xa1;
				s2[i * 2 + 1] = 0xf2;
				break;
			}

			n++;
		}

	}


	return;
}

int step(int win) {
	//srand( (unsigned)time( NULL ) ); 
	int i = rand() % 7;
	char s[2], key;
	sprintf(s, "%d\0", i);
	api_boxfilwin(win, 6, 27 + 9 * 16, 329, 27 + 10 * 16, 0);
	api_putstrwin(win, 6, 27 + 9 * 16, 7, 2, s);
	for (;;) {
		key = api_getkey(1);
		if (key == 0x0a) {
			break;
		}
	}
	return i;
}
