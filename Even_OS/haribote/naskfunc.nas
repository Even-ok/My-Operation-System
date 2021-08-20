; naskfunc
; TAB=4

; naskfunc.nas, 将只能用汇编语句实现或更益用汇编语句实现的功能
; 通过汇编子程序提供给C程序调用,所有的参数都通过栈传递。现在常
; 用的编译会适当选择用寄存器传参, 都通过栈传递参数可能是作者根
; 据当时需要而改写gcc 编译器所得到的结果。该编译器在将 C全局标
; 识符转换为汇编标识符时,会自动在C全局标识符前加'_'前缀。

[FORMAT "WCOFF"]				; 	
[INSTRSET "i486p"]				; 
[BITS 32]						; 
[FILE "naskfunc.nas"]			; 

; GLOBAL 告知编译器其后跟随的标号为全局标识符;
; EXTERN 告知编译器其后所声明标识符在其他文件中定义。
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

; _io_hlt,让CPU进入休眠。
;
; HLT 指令让CPU进入休眠;当复位,
; 中断到来时才唤醒CPU继续执行下一条指令(RET)。
_io_hlt:	; void io_hlt(void);
		HLT
		RET

_write_mem8:	; void write_mem8(int addr, int data);
		MOV		ECX,[ESP+4]		; [ESP+4] ecx
		MOV		AL,[ESP+8]		; [ESP+8] al
		MOV		[ECX],AL
		RET

; _io_cli,
; 禁止CPU处理调用者所在任务中断。
_io_cli:	; void io_cli(void);
		CLI
		RET

; _io_sti,
; 使能CPU处理调用者所在任务中断。
_io_sti:	; void io_sti(void);
		STI
		RET

; _io_stihlt,
; 让CPU进入睡眠,显式允许中断唤醒CPU。
_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

; _io_in8,
; 从I/O端口地址port读取1字节数据并返回。
_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

; _io_in16,
; 从I/O端口地址port读取2字节数据并返回。
_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

; _io_in32,
; 从I/O端口地址port读取4字节数据并返回。
_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

; _io_out8,
; 将data最低字节写往I/O端口地址port。
_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

; _io_out16,
; 将data低2字节数据写往I/O端口地址port。
_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

; _io_out32,
; 将4字节的data写往I/O端口地址port。
_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

; _io_load_eflags,
; 获取32位标志寄存器EFLAG的值并返回。
_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS 
		POP		EAX
		RET

; _io_store_eflags,
; 将eflags赋值给标志寄存器EFLAG。
_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS 
		RET

; _load_gdtr,
; 将GDT内存信息加载给GDTR寄存器,addr为GDT基址,limit为GDT限长。
_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

; _load_idtr,
; 加载IDT内存信息给IDTR寄存器,addr为IDT基址,limit为IDT限长。
_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET

; _load_cr0,
; 获取CR0寄存器值并返回。
_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

; _store_cr0,
; 将 cr0 设置给CR0寄存器。
_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

; _load_tr,
; 将tr加载给TR任务寄存器,tr为任务号。
_load_tr:		; void load_tr(int tr);
		LTR		[ESP+4]			; tr
		RET

; _asm_inthandler20,
; 定时器中断入口处理程序。
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

; _asm_inthandler21,
; 键盘中断入口处理承程序。
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

; _asm_inthandler2c,
; 鼠标中断处理入口程序。
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

; _asm_inthandler0c,
; 栈异常(如访问栈时超过了应用程序数据段)中断处理入口程序。
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
		ADD		ESP,4			; INT 0x0c 
		IRETD

; _asm_inthandler0d,
; 保护异常中断处理入口程序。
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
		CMP		EAX,0			;
		JNE		_asm_end_app	; 
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		ADD		ESP,4			; INT 0x0d 
		IRETD

; _memtest_sub,
; 以4Kb为单位测试[esp+12+4, esp+12+8)内存段中始于 esp+16 连续可用的内存段。
; 对于每个4Kb内存块,测试其最后4字节,若该4字节可用则标识整4Kb内存块可用。
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; EBX, ESI, EDI 
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
; 未检查完所有内存块时程序会执行到此处。
; 恢复最后4Kb块的最后4字节内容,
; 恢复相应寄存器并返回
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

; _farjmp,
; 实现段间跳转即CS:EIP=cs:eip。
_farjmp:		; void farjmp(int eip, int cs);
		JMP		FAR	[ESP+4]				; eip, cs
		RET

; _farcall,
; 实现段间调用即CS:EIP=cs:eip。
_farcall:		; void farcall(int eip, int cs);
		CALL	FAR	[ESP+4]				; eip, cs
		RET

; _asm_hrb_api,
; 系统调用(int 40h)入口处理程序。
_asm_hrb_api:
		STI
		PUSH	DS
		PUSH	ES
		PUSHAD		; 
		PUSHAD		; hrb_api PUSH
		MOV		AX,SS
		MOV		DS,AX		; 
		MOV		ES,AX
		CALL	_hrb_api
		CMP		EAX,0		; 
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
		RET					; cmd_app		

; _asm_end_app,
; 结束应用程序即跳转调用启动引用程序的 _start_app 后续语句处。
_asm_end_app:
;	EAX��tss.esp0�̔Ԓn
		MOV		ESP,[EAX]
		MOV		DWORD [EAX+4],0
		POPAD
		RET					; cmd_app

; _start_app,
; 跳转执行cs:eip处(应用程序)指令,内核当前栈内存的寄存器将被备份在 tss_esp0 所指栈内存中。
_start_app:		; void start_app(int eip, int cs, int esp, int ds, int *tss_esp0);
		PUSHAD		; 
		MOV		EAX,[ESP+36]	; 
		MOV		ECX,[ESP+40]	; 
		MOV		EDX,[ESP+44]	; 
		MOV		EBX,[ESP+48]	; 
		MOV		EBP,[ESP+52]	; 
		MOV		[EBP  ],ESP		; 
		MOV		[EBP+4],SS		; 
		MOV		ES,BX
		MOV		DS,BX
		MOV		FS,BX
		MOV		GS,BX
;	
		OR		ECX,3			; 
		OR		EBX,3			; 
		PUSH	EBX				; 
		PUSH	EDX				; 
		PUSH	ECX				; 
		PUSH	EAX				; 
		RETF
;	

[BITS	32]
backtoreal:
	PUSHFD
	PUSHAD
	JMP		start2
	db 0x00, 0x00
protect16:;26
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

;
;

realmode:;27
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
;
;	
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
; 
; 
;db 0xf4

GDTIDT:;38
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
;
;
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
; 
; 

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
;
;
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


