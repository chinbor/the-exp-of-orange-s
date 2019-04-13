; ==========================================
; pmtest5a.asm
; 编译方法：nasm pmtest5a.asm -o pmtest5a.com
; ==========================================

%include	"pm.inc"	; 常量, 宏, 以及一些说明

org	0100h
	jmp	LABEL_BEGIN

[SECTION .gdt]
; GDT
;                           	   段基址,             段界限,                       属性
LABEL_GDT:             Descriptor  		0,          	    0,   0          			; 空描述符
LABEL_DESC_NORMAL:     Descriptor 		0,        	   0ffffh,   DA_DRW    				; Normal 描述符
LABEL_DESC_CODE32:     Descriptor 		0,     SegCode32Len-1,   DA_C+DA_32 			; 非一致代码段,32
LABEL_DESC_CODE16:     Descriptor 		0,             0ffffh,   DA_C      				; 非一致代码段,16
LABEL_DESC_DATA: 	   Descriptor 		0,        DataLen - 1,   DA_DRW					; Data
LABEL_DESC_STACK:  	   Descriptor 		0,         TopOfStack,   DA_DRWA + DA_32 		; Stack, 32 位
LABEL_DESC_STACK3:     Descriptor 		0, 		  TopOfStack3,   DA_DRWA+DA_32+DA_DPL3  ;堆栈段stack3，dpl为3
LABEL_DESC_LDT:        Descriptor       0,   	   LDTLen - 1,   DA_LDT					; LDT
LABEL_DESC_VIDEO:      Descriptor 0B8000h,             0ffffh,   DA_DRW+DA_DPL3			; 显存首地址
LABEL_DESC_CODE_RING3: Descriptor 	    0, 	SegCodeRing3Len-1,   DA_C+DA_32+DA_DPL3     ;特权级3的代码段
LABEL_DESC_CODE_DEST:  Descriptor  		0, 	 SegCodeDestLen-1,   DA_C+DA_32  			;非一致代码段，32  	
;tss段描述符
LABEL_DESC_TSS: 	   Descriptor       0, 	       TSSLen - 1,   DA_386TSS              ;tss段的描述符（tss是任务状态段）
;门                                目标选择子,     偏移,  pCount,                   属性
LABEL_CALL_GATE_TEST:  Gate  SelectorCodeDest,        0,       0,    DA_386CGate+DA_DPL3;门描述符DA_386CGate=8CH 这里的结果为1000 1100
; GDT 结束


GdtLen		equ	$ - LABEL_GDT	; GDT长度
GdtPtr 		dw	GdtLen - 1	; GDT界限
			dd	0		; GDT基地址

; GDT 选择子
SelectorNormal			equ		LABEL_DESC_NORMAL		- LABEL_GDT  			;正常段选择子(目的是为了后面对段高速缓冲寄存器的刷新)
SelectorCode32			equ		LABEL_DESC_CODE32		- LABEL_GDT 			;由.s16这个段跳入（保护模式,） 32位代码段选择子（.s32），这个段具体是显示字符串到屏幕以及一些调用
SelectorCode16			equ		LABEL_DESC_CODE16		- LABEL_GDT 			;(.s16code)
SelectorData			equ		LABEL_DESC_DATA			- LABEL_GDT 			;(.data1)
SelectorStack			equ		LABEL_DESC_STACK		- LABEL_GDT 			;(.gs)
SelectorStack3			equ		LABEL_DESC_STACK3		- LABEL_GDT + SA_RPL3 	;(.s3),该选择子的rpl为3
SelectorCodeRing3 		equ 	LABEL_DESC_CODE_RING3 	- LABEL_GDT + SA_RPL3 	;(.ring3)
SelectorLDT				equ		LABEL_DESC_LDT			- LABEL_GDT 			;(.ldt)
SelectorVideo			equ		LABEL_DESC_VIDEO		- LABEL_GDT 			;显存段的选择子
SelectorCallGateTest	equ		LABEL_CALL_GATE_TEST	- LABEL_GDT + SA_RPL3   ;386调用门描述符在gdt中的选择子
SelectorTSS 			equ 	LABEL_DESC_TSS 			- LABEL_GDT 			;(.tss)
SelectorCodeDest 		equ 	LABEL_DESC_CODE_DEST 	- LABEL_GDT 			;作为测试用的代码段描述符的选择子(.sdest)
; END of [SECTION .gdt]

[SECTION .data1]	 ; 数据段
ALIGN	32
[BITS	32]
LABEL_DATA:
	SPValueInRealMode	dw	0
	; 字符串
	PMMessage:		 	db	"!-- In Protect Mode now. ^-^", 0	 ; 进入保护模式后显示此字符串
	OffsetPMMessage		equ	PMMessage - $$ 				 	 ;这样做是为了在保护模式下得到他的偏移
	DataLen				equ	$ - LABEL_DATA
; END of [SECTION .data1]


; 全局堆栈段
[SECTION .gs]
ALIGN	32
[BITS	32]
LABEL_STACK:
	times 512 db 0
TopOfStack	equ	$ - LABEL_STACK - 1   ;栈顶指针

; END of [SECTION .gs]

; 堆栈段ring3
[SECTION .s3]   
ALIGN	32
[BITS	32]
LABEL_STACK3:
	times 512 db 0
TopOfStack3	equ	$ - LABEL_STACK3 - 1
; END of [SECTION .s3]

;tss
[SECTION .tss]
ALIGN 32
[BITS 32]
LABEL_TSS:
	DD 0 			;BACK
	DD TopOfStack 	;0级堆栈
	DD SelectorStack;	
	DD 0 			;1级堆栈
	DD 0 			;
	DD 0 			;2级堆栈
	DD 0 			;
	DD 0 			;cr3
	DD 0 			;eip
	DD 0 			;eflags
	DD 0 			;eax
	DD 0 			;ecx
	DD 0 			;edx
	DD 0 			;ebx
	DD 0 			;esp
	DD 0 			;ebp
	DD 0 			;esi
	DD 0 			;edi
	DD 0 			;es
	DD 0 			;cs
	DD 0 			;ss
	DD 0 			;ds
	DD 0 			;fs
	DD 0 			;gs
	DD 0 			;ldt
	DW 0 			;调试陷阱标志
	DW $ - LABEL_TSS + 2 	;i/o位图基址
	DB 0ffh 				;i/o位图结束标志
TSSLen 	equ $ - LABEL_TSS
;这个段主要工作就是初始化全局描述符以及ldt中的描述符还有
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

	; 初始化 16 位代码段描述符(.s16code)
	mov	ax, cs
	movzx	eax, ax
	shl	eax, 4
	add	eax, LABEL_SEG_CODE16
	mov	word [LABEL_DESC_CODE16 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_CODE16 + 4], al
	mov	byte [LABEL_DESC_CODE16 + 7], ah

	; 初始化 32 位代码段描述符(.s32)
	xor	eax, eax
	mov	ax, cs
	shl	eax, 4
	add	eax, LABEL_SEG_CODE32
	mov	word [LABEL_DESC_CODE32 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_CODE32 + 4], al
	mov	byte [LABEL_DESC_CODE32 + 7], ah

	; 初始化数据段描述符(.data1)
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_DATA
	mov	word [LABEL_DESC_DATA + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_DATA + 4], al
	mov	byte [LABEL_DESC_DATA + 7], ah

	; 初始化堆栈段描述符(.gs)
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_STACK
	mov	word [LABEL_DESC_STACK + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_STACK + 4], al
	mov	byte [LABEL_DESC_STACK + 7], ah
	
	;初始化堆栈段描述符（.s3）
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_STACK3
	mov	word [LABEL_DESC_STACK3 + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_STACK3 + 4], al
	mov	byte [LABEL_DESC_STACK3 + 7], ah

	; 初始化 LDT 在 GDT 中的描述符(.ldt)
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_LDT
	mov	word [LABEL_DESC_LDT + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_LDT + 4], al
	mov	byte [LABEL_DESC_LDT + 7], ah

	; 初始化 LDT 中的描述符(.la)
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_CODE_A
	mov	word [LABEL_LDT_DESC_CODEA + 2], ax
	shr	eax, 16
	mov	byte [LABEL_LDT_DESC_CODEA + 4], al
	mov	byte [LABEL_LDT_DESC_CODEA + 7], ah
	
	;初始化ring3描述符(.ring3)
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_CODE_RING3
	mov word [LABEL_DESC_CODE_RING3 + 2],ax
	shr eax,16
	mov byte [LABEL_DESC_CODE_RING3 + 4],al
	mov byte [LABEL_DESC_CODE_RING3 + 7],ah
	
	; 初始化 TSS 描述符(.tss)
	xor	eax, eax
	mov	ax, ds
	shl	eax, 4
	add	eax, LABEL_TSS
	mov	word [LABEL_DESC_TSS + 2], ax
	shr	eax, 16
	mov	byte [LABEL_DESC_TSS + 4], al
	mov	byte [LABEL_DESC_TSS + 7], ah

	;初始化测试调用门的代码段描述符(.sdest)
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,LABEL_SEG_CODE_DEST
	mov word [LABEL_DESC_CODE_DEST+2],ax
	shr eax,16
	mov byte [LABEL_DESC_CODE_DEST+4],al
	mov byte [LABEL_DESC_CODE_DEST+7],ah
	
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
	jmp	dword SelectorCode32:0	; 执行这一句会把 SelectorCode32 装入 cs, 跳转到.s32段（为什么加dword32位），因为这个段是16位段，所以0是16位的，然而在保护模式中偏移地址可以大于16位

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


[SECTION .s32]; 32 位代码段. 由实模式跳入.特权级0
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
	mov	 ah,  0Ch			; 0000: 黑底    1100: 红字
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

	;call	DispReturn；这个函数目的就是跳到下一行开始处
	;加载tss（为了后面的由低特权级向高特权级转移时候不同特权级堆栈的改变）
	mov ax,SelectorTSS    
	ltr ax
	;这里对应的是ss esp cs eip，之后会进入(.ring3)，因为返回地址的选择子的rpl为3，相对于cpl=0更加小，所以向外层返回
	push SelectorStack3
	push TopOfStack3
	push SelectorCodeRing3
	push 0
	retf

	ud2  ;在这里代码不会到达这里，生成无效的操作码。提供此指令是为了在软件测试时显式生成无效的操作码。此指令的操作码保留用于此目的

	;测试调用门，将打印字母‘C’，SelectorCallGateTest选择子的rpl为3，调用门的dpl为3，.s32的cpl为0，对于门调用符规则和数据段一样，cpl&rpl<=dpl_g,对于调用门中的选择子对应的非一致代码段他的dpl为0，rpl为0，若为call指令，cpl>=门描述符中选择子对应非一致代码段的dpl,同时将会把检测堆栈的变化
	call SelectorCallGateTest:0
	
	;call SelectorCodeDest:0这一个是不通过调用门而是直接调用代码在内存地址所对应描述符

	; Load LDT
	mov	ax, SelectorLDT
	lldt	ax
	
	jmp	SelectorLDTCodeA:0	; 跳入局部任务a，也就是.la

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
	mov	ax, SelectorNormal   ;这里做的就是更新对应段的高速缓冲寄存器
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax
	;切换到实模式
	mov	eax, cr0
	and	al, 11111110b
	mov	cr0, eax

LABEL_GO_BACK_TO_REAL:
	jmp	0:LABEL_REAL_ENTRY	; 段地址会在程序开始处被设置成正确的值这句要理解的话就需要对应机器玛了，进入的是.s16

Code16Len	equ	$ - LABEL_SEG_CODE16

; END of [SECTION .s16code]
;调用门目标段
[SECTION .sdest]
[BITS 32]
LABEL_SEG_CODE_DEST:
	;jmp $   ;为了调试
	mov ax,SelectorVideo
	mov gs,ax 	 	 	 	;视频段选择子
	mov edi,(80 * 12 + 0) * 2
	mov ah,0ch
	mov al,'C'
	mov [gs:edi],ax
	;load ldt
	mov ax,SelectorLDT
	lldt ax
	jmp SelectorLDTCodeA:0    ;跳入局部任务a,(.la)
	;retf
SegCodeDestLen equ  $ - LABEL_SEG_CODE_DEST
;END  OF [SECTION .SDEST]


; LDT
[SECTION .ldt]
ALIGN	32
LABEL_LDT:
;                            段基址       段界限           属性
LABEL_LDT_DESC_CODEA: Descriptor 0, CodeALen - 1, DA_C + DA_32 ; Code, 32 位

LDTLen		equ	$ - LABEL_LDT

; LDT 选择子
SelectorLDTCodeA	equ	LABEL_LDT_DESC_CODEA	- LABEL_LDT + SA_TIL   ;(.la)
; END of [SECTION .ldt]


; CodeA (LDT, 32 位代码段)
[SECTION .la]
ALIGN	32
[BITS	32]
LABEL_CODE_A:
	mov	ax, SelectorVideo
	mov	gs, ax			; 视频段选择子(目的)

	mov	edi, (80 * 13 + 0) * 2	; 屏幕第 10 行, 第 0 列。
	mov	ah, 0Ch			; 0000: 黑底    1100: 红字
	mov	al, 'L'
	mov	[gs:edi], ax

	; 准备经由16位代码段跳回实模式
	jmp	SelectorCode16:0   ;进入(.s16code)
CodeALen	equ	$ - LABEL_CODE_A
; END of [SECTION .la]

;codering3
[SECTION .ring3]
ALIGN 32
[BITS 32]
LABEL_CODE_RING3:
	mov ax,SelectorVideo
	mov gs,ax
	mov edi,(80 * 14 +0) * 2
	mov ah,0ch
	mov al,'3'
	mov [gs:edi],ax
	call SelectorCallGateTest:0 ;进入(.sdest)
	jmp $
SegCodeRing3Len equ $- LABEL_CODE_RING3
;END of [section .ring3]

