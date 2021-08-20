#include "apilib.h"

void HariMain(void)
{
	int fileOpenResult;
	char content, cmdline[30], *p;
	api_cmdline(cmdline, 30);
	p = cmdline;
	while (*p > ' ') {
		p++;
	}
	while (*p == ' ') {
		p++;
	}
	fileOpenResult = api_fopen(p);
	if (fileOpenResult != 0) {
		while (1) {
			if (api_fread(&content, 1, fileOpenResult) == 0) {
				break;
			}
			api_putchar(content);
		}
	} else {
		api_putstr0("\n δ�ҵ��ļ�\n File not found.\n");
	}
	api_end();
}
