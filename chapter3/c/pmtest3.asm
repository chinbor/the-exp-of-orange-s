; ==========================================
; pmtest3.asm
; 编译方法：nasm pmtest3.asm -o pmtest3.com
; ==========================================

DA_32 	EQU 4000H
DA_C    EQU 98H
DA_LDT EQU 82H
DA_DRW EQU 92H
DA_DRWA EQU 93H
;DA_DPL1 EQU 20H
SA_TIL EQU 4

%macro Descriptor 3    
  
    dw  %2 & 0FFFFh             ; 段界限1（参数2的低16位）  
    dw  %1 & 0FFFFh             ; 段基址1（参数1的低16位）  
    db  (%1 >> 16) & 0FFh       ; 段基址2（参数1的16-23位）  
    dw  ((%2 >> 8) & 0F00h) | (%3 & 0F0FFh) ; 属性1（高4位） + 段界限2（高4位） + 属性2（低8位）  
    db  (%1 >> 24) & 0FFh       ; 段基址3（参数1的24-31位）  
%endmacro ; 共 8 字节

org	0100h
	jmp	LABEL_BEGIN

[SECTION .gdt]
; GDT
;                                         段基址,       段界限     , 属性
LABEL_GDT:         Descriptor       0,                 0, 0     	; 空描述符
LABEL_DESC_NORMAL: Descriptor       0,            0ffffh, DA_DRW	; Normal 描述符
LABEL_DESC_CODE32: Descriptor       0,  SegCode32Len - 1, DA_C + DA_32	; 非一致代码段, 32
LABEL_DESC_CODE16: Descriptor       0,            0ffffh, DA_C		; 非一致代码段, 16
LABEL_DESC_DATA:   Descriptor       0,       DataLen - 1, DA_DRW; Data
LABEL_DESC_STACK:  Descriptor       0,        TopOfStack, DA_DRWA + DA_32; Stack, 32 位
LABEL_DESC_LDT:    Descriptor       0,        LDTLen - 1, DA_LDT	; LDT
LABEL_DESC_VIDEO:  Descriptor 0B8000h,            0ffffh, DA_DRW	; 显存首地址
; GDT 结束

GdtLen		equ	$ - LABEL_GDT	; GDT长度
GdtPtr		dw	GdtLen - 1	; GDT界限
		dd	0		; GDT基地址

; GDT 选择子
SelectorNormal		equ	LABEL_DESC_NORMAL	- LABEL_GDT
SelectorCode32		equ	LABEL_DESC_CODE32	- LABEL_GDT
SelectorCode16		equ	LABEL_DESC_CODE16	- LABEL_GDT
SelectorData		equ	LABEL_DESC_DATA		- LABEL_GDT
SelectorStack		equ	LABEL_DESC_STACK	- LABEL_GDT
SelectorLDT		equ	LABEL_DESC_LDT		- LABEL_GDT
SelectorVideo		equ	LABEL_DESC_VIDEO	- LABEL_GDT
; END of [SECTION .gdt]

[SECTION .data1]	 ; 数据段
ALIGN	32
[BITS	32]
LABEL_DATA:
	SPValueInRealMode	dw	0
	; 字符串
	PMMessage:		db	"*-* In Protect Mode now. ^-^", 0	; 进入保护模式后显示此字符串
	OffsetPMMessage		equ	PMMessage - $$
	PMMessage2: 	db 	"the first task"
	PMMessage2Len 		equ $-PMMessage2
	offsetPMMessage2 	equ PMMessage2 - $$
	PMMessage3: 	db 	"the second task"
	PMMessage3Len 		equ $-PMMessage3
	offsetPMMessage3 	equ PMMessage3 - $$
DataLen			equ	$ - LABEL_DATA
; END of [SECTION .data1]


; 全局堆栈段
[SECTION .gs]
ALIGN	32
[BITS	32]
LABEL_STACK:
	times 512 db 0

TopOfStack	equ	$ - LABEL_STACK - 1

; END of [SECTION .gs]


[SECTION .s16]
[BITS	16]
LABEL_BEGIN:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, 0100h

	mov	[LABEL_GO_BACK_TO_REAL+3], ax
	mov	[SPValueInRealMode], sp

	; 初始化 16 位代码段描述符
	mov	ax, cs
	movzx	eax, ax
	shl	eax, 4
	add	eax, LABEL_SEG_CODE16
	mov	word [LABEL_DESC_CODE16 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_CODE16 + 4], al
	mov	byte [LABEL_DESC_CODE16 + 7], ah

	; 初始化 32 位代码段描述符
	xor	eax, eax
	mov	ax, cs
	shl	eax, 4
	add	eax, LABEL_SEG_CODE32
	mov	word [LABEL_DESC_CODE32 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_CODE32 + 4], al
	mov	byte [LABEL_DESC_CODE32 + 7], ah

	; 初始化数据段描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_DATA
	mov	word [LABEL_DESC_DATA + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_DATA + 4], al
	mov	byte [LABEL_DESC_DATA + 7], ah

	; 初始化堆栈段描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_STACK
	mov	word [LABEL_DESC_STACK + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_STACK + 4], al
	mov	byte [LABEL_DESC_STACK + 7], ah

	; 初始化 LDT 在 GDT 中的描述符
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_LDT
	mov	word [LABEL_DESC_LDT + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_LDT + 4], al
	mov	byte [LABEL_DESC_LDT + 7], ah

	; 初始化 LDT 中的描述符A
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_CODE_A
	mov	word [LABEL_LDT_DESC_CODEA + 2], ax
	shr	eax, 16
	mov	byte [LABEL_LDT_DESC_CODEA + 4], al
	mov	byte [LABEL_LDT_DESC_CODEA + 7], ah

	; 初始化 LDT 中的描述符B
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_CODE_B
	mov	word [LABEL_LDT_DESC_CODEB + 2], ax
	shr	eax, 16
	mov	byte [LABEL_LDT_DESC_CODEB + 4], al
	mov	byte [LABEL_LDT_DESC_CODEB + 7], ah

	; 为加载 GDTR 作准备
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_GDT		; eax <- gdt 基地址
	mov	dword [GdtPtr + 2], eax	; [GdtPtr + 2] <- gdt 基地址

	; 加载 GDTR
	lgdt	[GdtPtr]

	; 关中断
	cli

	; 打开地址线A20
	in	al, 92h
	or	al, 00000010b
	out	92h, al

	; 准备切换到保护模式
	mov	eax, cr0
	or	eax, 1
	mov	cr0, eax

	; 真正进入保护模式
	jmp	dword SelectorCode32:0	; 执行这一句会把 SelectorCode32 装入 cs, 并跳转到 Code32Selector:0  处

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

LABEL_REAL_ENTRY:		; 从保护模式跳回到实模式就到了这里
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax

	mov	sp, [SPValueInRealMode]

	in	al, 92h		; ┓
	and	al, 11111101b	; ┣ 关闭 A20 地址线
	out	92h, al		; ┛

	sti			; 开中断

	mov	ax, 4c00h	; ┓
	int	21h		; ┛回到 DOS
; END of [SECTION .s16]


[SECTION .s32]; 32 位代码段. 由实模式跳入.
[BITS	32]

LABEL_SEG_CODE32:
	mov	ax, SelectorData
	mov	ds, ax			; 数据段选择子
	mov	ax, SelectorVideo
	mov	gs, ax			; 视频段选择子

	mov	ax, SelectorStack
	mov	ss, ax			; 堆栈段选择子

	mov	esp, TopOfStack


	; 下面显示一个字符串
	mov	ah, 0Ch			; 0000: 黑底    1100: 红字
	xor	esi, esi
	xor	edi, edi
	mov	esi, OffsetPMMessage	; 源数据偏移
	mov	edi, (80 * 10 + 0) * 2	; 目的数据偏移。屏幕第 10 行, 第 0 列。
	cld
.1:
	lodsb
	test	al, al
	jz	.2
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1
.2:	; 显示完毕

	;call	DispReturn

	; Load LDT
	mov	ax, SelectorLDT
	lldt	ax

	call SelectorLDTCodeA:0 ;跳入局部任务a
	call SelectorLDTCodeB:0 ;跳入局部任务b
	jmp SelectorCode16:0

; ------------------------------------------------------------------------
DispReturn:
	push	eax
	push	ebx
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	ebx
	pop	eax

	ret
; DispReturn 结束---------------------------------------------------------

SegCode32Len	equ	$ - LABEL_SEG_CODE32
; END of [SECTION .s32]


; 16 位代码段. 由 32 位代码段跳入, 跳出后到实模式
[SECTION .s16code]
ALIGN	32
[BITS	16]
LABEL_SEG_CODE16:
	; 跳回实模式:
	mov	ax, SelectorNormal
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	mov	eax, cr0
	and	al, 11111110b
	mov	cr0, eax

LABEL_GO_BACK_TO_REAL:
	jmp	0:LABEL_REAL_ENTRY	; 段地址会在程序开始处被设置成正确的值

Code16Len	equ	$ - LABEL_SEG_CODE16

; END of [SECTION .s16code]


; LDT
[SECTION .ldt]
ALIGN	32 				;注意这里的意思是32字节对齐，也就是这一个节section的地址必须能整除32，这个节里面的地址都是按照顺序的
LABEL_LDT:
;                            段基址       段界限      属性
LABEL_LDT_DESC_CODEA: Descriptor 0, CodeALen - 1, DA_C + DA_32 ; Code, 32 位
LABEL_LDT_DESC_CODEB: Descriptor 0, CodeBLen - 1, DA_C + DA_32 ; Code, 32 位
LDTLen		equ	$ - LABEL_LDT

; LDT 选择子
SelectorLDTCodeA	equ	LABEL_LDT_DESC_CODEA	- LABEL_LDT + SA_TIL ;这里是为了置ti为1，因为，无论如何选择子的底3位都为0，一个描述符8字节，无论怎么选择子最低4位都是1000开始
SelectorLDTCodeB   	equ LABEL_LDT_DESC_CODEB 	- LABEL_LDT + SA_TIL
; END of [SECTION .ldt]


; CodeA (LDT, 32 位代码段)
[SECTION .la]
ALIGN	32
[BITS	32]
LABEL_CODE_A:
	mov		ax, SelectorVideo
	mov		gs, ax			; 视频段选择子(目的)
	mov 	ax,SelectorData;
	mov 	ds,ax
	mov     esi,offsetPMMessage2;mov     esi,PMMessage2注意不能这样写
	mov     ecx,PMMessage2Len
	mov	edi, (80 * 11 + 0) * 2	; 屏幕第 11 行, 第 0 列。
	mov	ah, 0Ch			; 0000: 黑底    1100: 红字
 .loop  mov     al,[ds:esi]
	mov	[gs:edi], ax
	add     edi,2
	inc esi
	loop .loop
	retf   
CodeALen	equ	$ - LABEL_CODE_A


; END of [SECTION .la]

; CodeB (LDT, 32 位代码段)
[SECTION .lB]
ALIGN	32
[BITS	32]
LABEL_CODE_B:
	mov		ax, SelectorVideo
	mov		gs, ax			; 视频段选择子(目的)
	mov 	ax,SelectorData;
	mov 	ds,ax
	mov     esi,offsetPMMessage3    
	mov     ecx,PMMessage3Len
	mov		edi, (80 * 12 + 0) * 2	; 屏幕第 11 行, 第 0 列。
	mov		ah, 0Ch			; 0000: 黑底    1100: 红字
 .loop: 
	mov     al,[ds:esi]
	mov	 	[gs:edi], ax
	add     edi,2
	inc esi
	loop .loop
	retf   
CodeBLen	equ	$ - LABEL_CODE_B

; END of [SECTION .lb]
