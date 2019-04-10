;====================================
;pmtest1.asm
;编译方法:nasm pmtest1.asm -o pmtest1.bin
;===================================
;原本在pm.inc中的常量和宏
DA_32  equ 4000h 
DA_C   equ 98h
DA_DRW equ 92h
;描述符的结构
%macro Descriptor 3
	dw %2 & 0ffffh  ;代表段界限的低16位
	dw %1 & 0ffffh  ;代表段基址的低16位
	db (%1 >> 16) & 0ffh ;代表段基址的中间8位
	dw ((%2 >> 8) & 0f00h) | (%3 & 0f0ffh) ;代表的含义是（属性高4位）+（段界限高4位）+（属性底8位）
	db (%1 >> 24) & 0ffh   ;代表的是段基址的高8位
%endmacro
;==================================
org 0100h ;这样做的目的是因为在DOS下，装载程序会事先创建一个称为程序段前缀（PSP）256字节的数据区，
	jmp LABEL_BEGIN

[SECTION .gdt]                     ;段基址              ;段界限       ;段属性
LABEL_GDT: 			Descriptor              0,                0,            0
LABEL_DESC_CODE32: 	Descriptor              0, SegCode32Len - 1, DA_C + DA_32 ;结果是0100 0000 1001 1000对应段属性
LABEL_DESC_VIDEO: 	Descriptor        0b8000h ,          0ffffh,       DA_DRW ;结果是0000 0000 1001 0010对应段属性,这里是已经初始化好的一个显存地址 
;GDT结束这里建立了一个全局描述符

GdtLen 	equ 	$ - LABEL_GDT;全局描述符的长度
GdtPtr 	dw 		GdtLen -1 ;全局描述符的界限
		dd 		0  ;这里是指的全局描述符的基址但是这里还未赋值

;gdt选择子
SelectorCode32 	equ 	LABEL_DESC_CODE32 	- 	LABEL_GDT ;在gdt表中的序号
SelectorVideo 	equ 	LABEL_DESC_VIDEO 	- 	LABEL_GDT ;同上
;END OF [SECTION .gdt]

[SECTION .s16]
[BITS 16];这里是指接下来都是在16位cpu模式下进行
LABEL_BEGIN:
	mov ax,cs;将代码段内容放在ax，ds，es，ss中,因为这里的这整个程序都是在代码段中的
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,0100h
	;这里的意思是指对描述符LABEL_DESC_CODE32的段基址进行初始化。
	xor eax,eax;eax清0
	mov ax,cs
	shl eax,4;代码段地址左移动4位，然后加上偏移量LABEL_SEG_CODE32因为此时还是遵循实模式的寻址方式
	add eax,LABEL_SEG_CODE32
	mov word [LABEL_DESC_CODE32 + 2],ax;段基址1
	shr eax,16
	mov byte [LABEL_DESC_CODE32 + 4],al;段基址中间8位
	mov byte [LABEL_DESC_CODE32 + 7],ah;段基址高8位
	;这里是对全局符号表gdt进行初始化
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_GDT;代码段地址加上LABEL_GDT（实模式）
	mov dword [GdtPtr + 2],eax;将全局符号表的始址存放在GdtPtr中，为之后加载进全局符号表寄存器作准备
	;加载进GDTR寄存器
	lgdt [GdtPtr]
	;关中断，因为实模式下的中断方式和保护模式不一样
	cli
	;开启A20地址环绕，因为很多程序是采用地址环绕的工作方式，为了兼容
	in al,92h
	or al,00000010b
	out 92h,al
	;这里是置cr0的pe位为1，这样就开启了保护模式
	mov eax,cr0
	or eax,1
	mov cr0,eax
	;这里就是将这个选择子SelectorCode32放入cs寄存器，然后采用的是保护模式下的寻址，根据cs寄存器和gdtr寄存器找到段描述符，然后就知道段的界限，基址，属性，后面的0代表在段中  的偏移地址，就进入了LABEL_SEG_CODE32标签处开始在保护模式32位模式下执行接下来的代码
	jmp dword SelectorCode32:0
;end of [SECRION .s16]
[SECTION .s32]
[BITS 32];工作在32位模式下
LABEL_SEG_CODE32:
	mov ax,SelectorVideo
	mov gs,ax
	mov edi,(80 * 11 +79) * 2 ; 屏幕的11行 79列
	mov ah,0ch ;0000:黑底  1100红字
	mov al,'p' 
	mov [gs:edi],ax ;采用的是保护模式下的寻址方式，选择子+段内偏移地址
	jmp $   ;无限循环

SegCode32Len equ $ - LABEL_SEG_CODE32 ;这里的意思是这个段的长度，再减去一个1就是这个段的界限了
;END OF [SECTION .s32]
