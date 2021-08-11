; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; �I�u�W�F�N�g�t�@�C������郂�[�h	
[INSTRSET "i486p"]				; 486�̖��߂܂Ŏg�������Ƃ����L�q
[BITS 32]						; 32�r�b�g���[�h�p�̋@�B�����点��
[FILE "naskfunc.nas"]			; �\�[�X�t�@�C�������

		GLOBAL	_io_hlt,_write_mem8, _io_cli, _io_sti, _io_stihlt
		GLOBAL	_io_in8,  _io_in16,  _io_in32
		GLOBAL	_io_out8, _io_out16, _io_out32
		GLOBAL	_io_load_eflags, _io_store_eflags
		GLOBAL	_load_gdtr, _load_idtr
		GLOBAL	_load_cr0, _store_cr0
		GLOBAL	_load_tr
		GLOBAL	_asm_inthandler20, _asm_inthandler21
		GLOBAL	_asm_inthandler2c, _asm_inthandler0c
		GLOBAL	_asm_inthandler0d, _asm_end_app
		GLOBAL	_memtest_sub
		GLOBAL	_farjmp, _farcall
		GLOBAL	_asm_hrb_api, _start_app
		GLOBAL	_shutdown, _reboot
		EXTERN	_inthandler20, _inthandler21,_inthandler2c
		EXTERN	_inthandler2c, _inthandler0d
		EXTERN	_inthandler0c
		EXTERN	_hrb_api

[SECTION .text]

_io_hlt:	; void io_hlt(void);
		HLT
		RET

_write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX,[ESP+4]		; [ESP+4]��ŵ�ַ ����ecx
		MOV		AL,[ESP+8]		; [ESP+8]������� ����al
		MOV		[ECX],AL
		RET

_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS �Ƃ����Ӗ�
		POP		EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS �Ƃ����Ӗ�
		RET

_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

_load_tr:		; void load_tr(int tr);
		LTR		[ESP+4]			; tr
		RET


_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler0c:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0c
		CMP		EAX,0
		JNE		_asm_end_app
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0c �ł��A���ꂪ�K�v
		IRETD

_asm_inthandler0d:
		STI
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler0d
		CMP		EAX,0			; ���������Ⴄ
		JNE		_asm_end_app	; ���������Ⴄ
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d �ł́A���ꂪ�K�v
		IRETD

_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; �iEBX, ESI, EDI ���g�������̂Łj
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

_farjmp:		; void farjmp(int eip, int cs);
		JMP		FAR	[ESP+4]				; eip, cs
		RET

_farcall:		; void farcall(int eip, int cs);
		CALL	FAR	[ESP+4]				; eip, cs
		RET

_asm_hrb_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD		; �ۑ��̂��߂�PUSH
		PUSHAD		; hrb_api�ɂ킽�����߂�PUSH
		MOV		AX,SS
		MOV		DS,AX		; OS�p�̃Z�O�����g��DS��ES�ɂ������
		MOV		ES,AX
		CALL	_hrb_api
		CMP		EAX,0		; EAX��0�łȂ���΃A�v���I������
		JNE		_asm_end_app
		ADD		ESP,32
		POPAD
		POP		ES
		POP		DS
		IRETD

end_app:
;	EAXΪtss.esp0�ĵ�ַ
		MOV		ESP,[EAX]
		POPAD
		RET					; ����cmd_app		

_asm_end_app:
;	EAX��tss.esp0�̔Ԓn
		MOV		ESP,[EAX]
		MOV		DWORD [EAX+4],0
		POPAD
		RET					; cmd_app�֋A��

_start_app:		; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD		; 32�r�b�g���W�X�^��S���ۑ����Ă���
		MOV		EAX,[ESP+36]	; �A�v���p��EIP
		MOV		ECX,[ESP+40]	; �A�v���p��CS
		MOV		EDX,[ESP+44]	; �A�v���p��ESP
		MOV		EBX,[ESP+48]	; �A�v���p��DS/SS
		MOV		EBP,[ESP+52]	; tss.esp0�̔Ԓn
		MOV		[EBP  ],ESP		; OS�p��ESP��ۑ�
		MOV		[EBP+4],SS		; OS�p��SS��ۑ�
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
;	�ȉ���RETF�ŃA�v���ɍs�����邽�߂̃X�^�b�N����
		OR		ECX,3			; �A�v���p�̃Z�O�����g�ԍ���3��OR����
		OR		EBX,3			; �A�v���p�̃Z�O�����g�ԍ���3��OR����
		PUSH	EBX				; �A�v����SS
		PUSH	EDX				; �A�v����ESP
		PUSH	ECX				; �A�v����CS
		PUSH	EAX				; �A�v����EIP
		RETF
;	�A�v�����I�����Ă������ɂ͗��Ȃ�

[BITS	32]
backtoreal:
	PUSHFD
	PUSHAD
	JMP		start2
	db 0x00, 0x00
protect16:;26�ֽ�
;		mov     ax, 8
;        mov     ds, ax
;        mov     es, ax
;        ;mov     fs, ax
;        ;mov     gs, ax
;        mov     ss, ax
;
;        mov     eax, cr0
;        and     ax, 11111110b
;        mov     cr0, eax
db 0xb8, 0x08, 0x00, 0x8e, 0xd8, 0x8e, 0xc0, 0x8e, 0xd0
db 0x0f, 0x20, 0xc0, 0x66, 0x25, 0xfe, 0xff, 0xff, 0x7f
db 0x0f, 0x22, 0xc0
db 0xea
dw 0x0650,0x0000
ALIGNB 16
protect16_len EQU	$ - protect16

;����Ĵ���Ϊ16λ����ģʽ����ʵģʽ���ܴ���
;����ģʽ���봫�͵��ڴ�0x0630����Ϊ������0x20 B

realmode:;27�ֽ�
;        mov     ax, cs
;		 mov     ds, ax
;		 mov     es, ax
;	     mov     ss, ax
;
;	     mov     sp, 0800h
;
;		in      al, 92h
;		and     al, 11111101b
;		out     92h, al
;		nop
;		nop
;		nop
;		sti
;		nop
;		һ�´������������ʾģʽΪͼ��ģʽ���룬��goback֮ǰ���� BIOS�жϵ���ʹ�ܡ�
;		С�������������ֵ�ʹ��BIOS�жϳ����
;		mov     ax, 0003h
;		int     10h
db 0x8c, 0xc8
db 0x8e, 0xd8
db 0x8e, 0xc0
db 0x8e, 0xd0
db 0xbc, 0x00, 0x08
db 0xe4, 0x92
db 0x24, 0xfd
db 0xe6, 0x92
db 0x90, 0x90, 0x90
db 0xfb, 0x90
db 0xb8, 0x03, 0x00
db 0xcd, 0x10
db 0xea
dw 0x0670, 0x0000
ALIGNB 16
realmode_len	EQU		$ - realmode
; ���ϴ����Ϊʵģʽ�������ַ���ʾģʽ
; ʵģʽ���ܴ��봫�͵�0x0650��,����32�ֽ�
;db 0xf4

GDTIDT:;38�ֽ�
dw 0x0000, 0x0000, 0x0000, 0x0000
dw 0xffff, 0x0000, 0x9200, 0x0000
dw 0xffff, 0x0000, 0x9800, 0x0000
dw 0x0000
dw 0x0017
dw 0x0600, 0x0000
dw 0x03ff
dw 0x0000, 0x0000
ALIGNB 16
GDTIDT_lenth EQU	$ - GDTIDT
;����ΪGDT��ITD��������
;�������ݴ��͵�0x0600��������0x30 B�Ŀռ䡣
start2:
	MOV		EBX, GDTIDT
	MOV		EDX, 0x600
	MOV		CX, GDTIDT_lenth
.loop1:
	MOV		AL, [CS:EBX]
	MOV		[EDX], AL
	INC		EBX
	INC		EDX
	loop	.loop1

	MOV		EBX, protect16
	MOV		EDX, 0x630
	MOV		CX, protect16
.loop2:
	MOV		AL, [CS:EBX]
	MOV		[EDX], AL
	INC		EBX
	INC		EDX
	loop	.loop2	

	MOV		EBX, realmode
	MOV		EDX, 0x650
	MOV		CX, realmode_len
.loop3:
	MOV		AL, [CS:EBX]
	MOV		[EDX], AL
	INC		EBX
	INC		EDX
	loop	.loop3	

	POPAD
	POPFD
	RET
	
_shutdown:
	JMP		shutdown_start
	db 0x00, 0x00
shutdown_con:
;	MOV		AX, 5307H 		;Function 5307h: APM Set system power state
;	MOV 	BX, 01H 		;Device ID: 0001h (=All devices)
;	MOV 	CX, 0003H 		;Power State: 0003h (=Off)
;	INT 	15H		 		;Call interrupt: 15h
db 0xb8, 0x07, 0x53
db 0xbb, 0x01, 0x00
db 0xb9, 0x03, 0x00
db 0xcd, 0x15
ALIGNB 16
shutdown_con_len	EQU		$ - shutdown_con
; ���ϴ���ιػ�����
; ʵģʽ���ܴ��봫�͵�0x0670����

shutdown_start:
	CALL 	backtoreal

	MOV		EBX, shutdown_con
	MOV		EDX, 0x670
	MOV		CX, shutdown_con_len
.loop4:
	MOV		AL, [CS:EBX]
	MOV		[EDX], AL
	INC		EBX
	INC		EDX
	loop	.loop4

	LGDT	[0x061A]
	LIDT	[0x0620]
	JMP		2*8:0x0630

_reboot:
	JMP 	reboot_start
	db 0x00, 0x00
reboot_con:
	;mov ax, 0xffff
	;push ax
	;mov ax, 0x0000
	;push ax
	;retf
;db 0xb8, 0xff, 0xff
;db 0x50
;db 0xb8, 0x00, 0x00
;db 0x50
;db 0xcb

;	MOV 	BX, 0x0040
;	MOV 	DS, BX
;	MOV 	BX, 0x1234
;	MOV 	[0x0072], BX
;	JMP 	0xffff:0x0000
db 0xbb, 0x40, 0x00
db 0x8e, 0xdb
db 0xbb, 0x34, 0x12
db 0x89, 0x1e, 0x72, 0x00
db 0xea, 0x00, 0x00, 0xff, 0xff
ALIGNB 16
reboot_con_len	EQU		$ - reboot_con
;���ϴ���Ϊ��������
;ʵģʽ�´��봫�͵�0x0670��
reboot_start:
	call 	backtoreal

	MOV		EBX, reboot_con
	MOV		EDX, 0x670
	MOV		CX, reboot_con_len
.loop5:
	MOV		AL, [CS:EBX]
	MOV		[EDX], AL
	INC		EBX
	INC		EDX
	loop	.loop5

	LGDT	[0x061A]
	LIDT	[0x0620]
	JMP		2*8:0x0630


