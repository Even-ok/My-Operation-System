; haribote-ipl
; TAB=4

; ipl09.nas, 操作系统引导程序

CYLS	EQU		30				; 从磁盘读取haribote时所读柱面数

		ORG		0x7c00			; 告知编译器, 引导程序段内偏移基址为0x7c00，让后续程序的段内偏移基址为0x7c00。


; 因为haribote支持FAT12文件系统且haribote在该文件系统中,
; 所以引导区中包含了FAT12保留区域内容。
;
; 粗略了解FAT12文件系统在磁盘上的大体格式。
; ---------------------------
; |保留区域|FAT区域|数据区域|
; ---------------------------
; [1] 保留区域总体描述了FAT文件系统的基本组成和相关参数。
; [2] FAT区域管理文件系统簇的使用情况。
; [3] 数据区用于保存文件数据。
;
; 先来看看FAT12保留区域内容吧。
; (对于偶尔不明白其实际含义的注释,暂可略过,file.c 可能助该处内容的理解)
;
; 这段被用作FAT12保留区域的描述内容共占62字节。
; 对加载引导程序的BIOS来说, 这62字节是透明的:
; BIOS加载引导程序到[0x07c00, 0x07e00)内存段后,
; 若引导程序最后两字节为0xaa55则跳转执行0x07c00
; 处内容(对应此处的'jmp entry'指令)。

		JMP entry       ; 跳转指令
    DB  0x90        ; nop指令的机器码(与跳转指令共占3字节)
    DB  "HARIBOTE"  ; 用作操作系统名称
    DW  512         ; 磁盘扇区字节数
    DB  1           ; 磁盘每簇扇区数
    DW  1           ; 保留区扇区数
    DB  2           ; FAT(文件分配表)数量
    DW  224         ; 根目录下的文件最大数量
    DW  2880        ; 文件系统扇区总数, 2880个扇区为1.44Mb,
                    ; 扇区数若超过65535,则此处为0, [32,35]偏移处为文件系统扇区总数。
    DB  0xf0        ; 文件系统所在存储介质类型, 0xf0表示可移动介质
    DW  9           ; 一个FAT所占扇区数
    DW  18          ; 每磁道扇区数
    DW  2           ; 磁头数
    DD  0           ; 无隐藏扇区(若有则在保留区域前)
    DD  2880        ; 文件系统所占扇区数
    DB  0,0,0x29    ; BIOS int 13h磁盘号; 保留未使用; 0x29表示下一个值才表示卷数
    DD  0xffffffff  ; 卷数(卷序列号)
    DB  "HARIBOTEOS " ; 用作卷标, 磁盘名称
    DB  "FAT12   "    ; 用作文件系统类型标签
    ; 62字节偏移处, FAT12保留区域内容结束
    RESB    18        ; 在描述FAT12文件系统概样之后填充18字节0(之后便从80字节处开始)


; 由FAT12保留区域第一条跳转指令跳转到此
entry:
	MOV AX,0 ;设置引导程序数据段和栈段基址(寄存器)为0
    MOV SS,AX
    MOV SP,0x7c00 ;栈顶基址为0x7c00
    MOV DS,AX

; 调用readfast子程序读取haribote到[0x08200, 0x30800)内存段中
    MOV AX,0x0820
    MOV ES,AX
    MOV CH,0           ; 柱面初始值,从柱面0开始读取
    MOV DH,0           ; 磁头初始值,从磁头0开始读取
    MOV CL,2           ; 扇区初始值,从扇区2开始读取,1对应引导区
    MOV BX,18*2*CYLS-1 ; ES:BX指向保存从磁盘读取的hariboe的内存首地址,
                       ; BX同时兼任保存未读扇区总数的任务。
    CALL readfast

; 将haribote柱面数保存在ds:[0x0ff0]处,即0:0x0ff0处,供后续程序使用。
; 读取haribote到[0x08200, 0x30800)内存中后, 跳转到0xc200处执行haribote
		MOV		BYTE [0x0ff0],CYLS	
		JMP		0xc200

; 通过BIOS 10h中断调用打印haribote读取失败的提示信息
error:
    MOV AX,0    ; msg所在内存段 基址
    MOV ES,AX
    MOV SI,msg  ; msg段内偏移地址
putloop:
    MOV AL,[SI] ; 取msg中的当前字符
    ADD SI,1    ; 让si指向下一个字符
    CMP AL,0
    JE  fin     ; 若到AL上的值为字符串末尾(0标识)则跳转fin处
    MOV AH,0x0e ; AH=0x0e: 显示AL上的ASCII字符到屏幕上
    MOV BX,15   ; BH:字符显示模式;BL:前景色
    INT 0x10    ; 调用BIOS 10h显示入参指定的字符
    JMP putloop

; 读取haribote失败, 则最后休眠死机
fin:
    HLT
    JMP fin

; 读取haribote失败后所欲显示的提示信息
msg:
    DB  0x0a, 0x0a  ;回车的ASCII
    DB  "load error"
    DB  0x0a
    DB  0 ;提示信息结束标志

readfast:
    MOV AX,ES
    SHL AX,3    ; 将段地址的bit[9..11]移到AH低3位中
    AND AH,0x7f ; 取段地址bit[9..15]的值
    MOV AL,128  ; 128 * 512 = 64Kb(x86实模式下 一个段的大小)
    SUB AL,AH   ; AL=AL-AH,即计算读满当前段还需的扇区数

    MOV AH,BL   ; 当未读扇区数小于256时,BL值就是未读扇区数,尤其当未读扇区所剩无几时,该子程序段有效
    CMP BH,0    ; if (BH != 0) { AH = 18; }, 该条件略微粗糙, if (BX>=18){AH=18}更好一点
    JE  .skip1
    MOV AH,18
.skip1:
		CMP		AL,AH			; if (AL > AH) { AL = AH; }
		JBE		.skip2
		MOV		AL,AH
.skip2:

		MOV		AH,19			;
		SUB		AH,CL			; AH = 19 - CL; 计算当前磁道还未读取的扇区数与AH中
		CMP		AL,AH			; if (AL > AH) { AL = AH; }
		JBE		.skip3
		MOV		AL,AH
; 到此处, AL包含了当前所能读取扇区的最大值

.skip3:

		PUSH	BX
		MOV		SI,0			;  用si记录每次读磁盘的错误次数
retry:
    MOV AH,0x02 ; AH=0x02: 读磁盘
    MOV BX,0    ; 所读内容保存在ES:BX指向的内存段
    MOV DL,0x00 ; 读A盘驱动器
    PUSH ES     ; 在栈中备份以下几个寄存器
    PUSH DX
    PUSH CX
    PUSH AX
    INT 0x13    ; 调用BIOS 13h读磁盘
    JNC next    ; eflag.CF=0即若读取成功则跳转next处
    ADD SI,1    ; 读取失败次数增1
    CMP SI,5
    JAE error   ; if (si >=5) 则跳转error处
    MOV AH,0x00 ; if (si < 5) 则重置A驱重读, AH=0表重置磁盘功能号
    MOV DL,0x00 ; A驱动器号
    INT 0x13
    POP AX
    POP CX
    POP DX
    POP ES
    JMP retry

next:
    POP AX
    POP CX
    POP DX
    POP BX      ; ES寄存器的值出栈给BX
    SHR BX,5    ; BX右移5位,对于段地址来说相当于右移了9位
    MOV AH,0
    ADD BX,AX   ; BX += AL, 即更新已读扇区数
    SHL BX,5    ; 再将BX左移5位,对于段地址来说相当于左移了9位,即让bit[9..]记录已读扇区数
    MOV ES,BX   ; 将BX计算结果赋给ES, 将ES出栈给BX到此处的整个过程相当于ES += AL * 0x20;
    POP BX      ; 再从栈中取出真正的BX
    SUB BX,AX   ; 将未读扇区总数减去已读扇区数
    JZ  .ret    ; 若已读完所有扇区则跳转至.ret处返回
    ADD CL,AL   ; 否则更新下一次读磁盘时的起始扇区
    CMP CL,18   ;
    JBE readfast; 若起始扇区小于磁道扇区数即还未读完当前磁道则返回到readfast处继续阅读
    MOV CL,1    ; 若已读完当前磁道,则将起读扇区为1,准备读取下一个磁道
    ADD DH,1
    CMP DH,2
    JB  readfast ; DH<2表示当前磁头为1,则将磁头设置为2即读取当前柱面另一面
    MOV DH,0     ; 若当前磁头为2则又回到磁头1(读取下一柱面)
    ADD CH,1     ; 若当前磁头为2则表明当前柱面正反面都已读取完毕, 则应读取下一柱面了
    JMP readfast
.ret:            ; 读取haribote完毕,子程序返回
    RET

; 本段程序从偏移0x7c00开始, $表示当前位置基于段内偏移基址的值,
; 即将[引导程序末尾处即.ret处, 0x7dfe]置为0
    RESB    0x7dfe-$
    DB  0x55, 0xaa      ; 设置引导区有效标志0xaa55
