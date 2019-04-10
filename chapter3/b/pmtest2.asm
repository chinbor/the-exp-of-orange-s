;====================================================================================================================================
;pmtest2.asm
;编译方法：nasm pmtest2.asm -o pmtest2.com
;注意在挂载的时候利用bximage生成的pm.img文件类型可能不是boot sector，需要先格式化再继续sudo mount -o loop pm.img /mnt/floppy才能成功
;====================================================================================================================================

;原本在pm.inc中的宏定义和一些常量
DA_32 	EQU 	4000h 		;代表32位段,因为二进制是0100 0000 0000 0000，D/B位为1，若为可执行代码段，那么代表的意思就是指令，操作数用32位
DA_C 	EQU 	98h   		;1001 1000,p为1，代表存在于内存中，s位为1代表是数据段/代码段，type中的第3位为1代表是代码段，dpl为0代表特权及为0（最高）
DA_DRW 	EQU 	92h   		;1001 0010,存在的可读写数据段属性
DA_DRWA EQU 	93h  		;1001 0011,存在的以访问可读写数据段属性
;ATCE32 EQU 4098h 

;描述符的结构定义
%macro Descriptor 3
	dw %2 & 0FFFFh   						;段界限低16位
	dw %1 & 0FFFFh    						;段基址低16位
	db (%1 >> 16) & 0FFh  					;段基址中间8位
	dw ((%2 >> 8) & 0F00h) | (%3 & 0F0FFh) 	;段属性1（G，D/B，0，AVL）段界限高4位 段属性2（p，dpl，s，type）
	db (%1 >> 24) & 0FFh   					;段基址高8位
%endmacro

;起始地址，这样做的目的是因为在DOS下，装载程序会事先创建一个称为程序段前缀（PSP）256字节的数据区，用来和被装载的程序通信
org 0100h
	jmp LABEL_BEGIN

;定义gdt表
[SECTION .gdt]

LABEL_GDT: 				Descriptor 		0, 			0, 					0        		;空描述符，用于一个任务没有ldt时候，ldtr清0（ldtr中存放的是ldt表在gdt中的描述符选择子），此时就是指向空描述符
LABEL_DESC_NORMAL: 		Descriptor 		0, 			0ffffh, 			DA_DRW 			;正常描述符，为了从保护模式返回实模式的时候刷新各个段描述符缓冲寄存器
LABEL_DESC_CODE32: 		Descriptor 		0, 			segCode32Len-1, 	DA_C+DA_32 		;非一致代码段，32位，也就是.s32的段选择子
LABEL_DESC_CODE16: 		Descriptor 		0, 			0ffffh, 			DA_C 			;非一致代码段，16位，是.s16code的段选择子
LABEL_DESC_DATA: 		Descriptor 		0, 			DataLen-1, 			DA_DRW 			;数据段
LABEL_DESC_STACK: 		Descriptor 		0, 			TopOfStack, 		DA_DRWA+DA_32 	;堆栈段，注意这里因为后面用了ss寄存器来存放选择子selectorstack，所以b=1代表堆栈访问指令默认用32位esp寄存器
LABEL_DESC_TEST: 		Descriptor 		0500000h, 	0ffffh, 			DA_DRW 			;这是自己建立的一个以5mb为基址的段（也就是体现保护模式下寻址空间更广的段）
LABEL_DESC_VIDEO:  		Descriptor 		0B8000h, 	0ffffh, 			DA_DRW 			;这是显存的首地址地址

Gdtlen 		equ 	$ - LABEL_GDT 	;gdt长度
GdtPtr 		dw 		Gdtlen - 1 		;gdt界限
	   		dd 		0 				;gdt基地址（还未赋值默认初始化为0，后面会进行赋值，在.s16中进行）
;段选择子
SelectorNormal 		equ 		LABEL_DESC_NORMAL - LABEL_GDT
SelectorCode32 		equ 		LABEL_DESC_CODE32 - LABEL_GDT
SelectorCode16 		equ 		LABEL_DESC_CODE16 - LABEL_GDT
SelectorData 		equ 		LABEL_DESC_DATA   - LABEL_GDT
SelectorStack 		equ 		LABEL_DESC_STACK  - LABEL_GDT
SelectorTest 		equ 		LABEL_DESC_TEST   - LABEL_GDT
SelectorVideo 		equ 		LABEL_DESC_VIDEO  - LABEL_GDT
;数据段
[SECTION .data1]
ALIGN 32 		;32字节对齐（具体原因为啥这样做，我也暂且不懂）
[BITS 32]
LABEL_DATA:
SPValueInRealMode 	dw 		0 									;用来存放后面的sp寄存器值
PMMessage: 			db 		"qaq In Protect Mode NOW. -.-",0 	;将要在显示器输出的字符串（保护模式下输出）
offsetPMMessage 	equ 	PMMessage - $$ 						;偏移地址（保护模式下寻址用）
StrTest: 			db 		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",0 		;将要写入label_desc_test段描述符所指段的内容
offsetStrTest 		equ 	StrTest - $$ 						;偏移地址（保护模式下寻址用）
DataLen 			equ 	$ - LABEL_DATA    					;data1段界限
;堆栈段
[SECTION .gs]
ALIGN 32
[BITS 32]
LABEL_STACK:
	times 512 db 0
TopOfStack 			equ 	$ - LABEL_STACK -1;代表的是栈底地址

;16位代码段，目的实模式进入保护模式，以及对段基址进行初始化
[SECTION .S16]
[BITS 16]
LABEL_BEGIN:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,0100h
	mov [LABEL_GO_BACK_TO_REAL+3],ax 	;这里的作用是将这里实模式下的cs段寄存器的值存入.s16code段中的标签地址+3的位置处（作为以后返回实模式用于给cs赋值）
	mov [SPValueInRealMode],sp 			;这里的意思是保存sp寄存器内容，方便后面的返回恢复
	;初始化16位代码段描述符（段基址）
	mov ax,cs
	movzx eax,ax ;这里代表的意思是进行扩展
	shl eax,4
	add eax,LABEL_SEG_CODE16
	mov word [LABEL_DESC_CODE16+2],ax
	shr eax,16
	mov byte [LABEL_DESC_CODE16+4],al
	mov byte [LABEL_DESC_CODE16+7],ah
	;初始化32位代码段描述符（段基址）
	xor eax,eax
	mov ax,cs
	shl eax,4
	add eax,LABEL_SEG_CODE32
	mov word [LABEL_DESC_CODE32+2],ax
	shr eax,16
	mov byte [LABEL_DESC_CODE32+4],al
	mov byte [LABEL_DESC_CODE32+7],ah
	;初始化数据段描述符（段基址）
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_DATA
	mov word [LABEL_DESC_DATA+2],ax
	shr eax,16
	mov byte [LABEL_DESC_DATA+4],al
	mov byte [LABEL_DESC_DATA+7],ah
	;初始化堆栈段描述符（段基址）
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_STACK
	mov word [LABEL_DESC_STACK+2],ax
	shr eax,16
	mov byte [LABEL_DESC_STACK+4],al
	mov byte [LABEL_DESC_STACK+7],ah
	;为加载gdtr作准备
	xor eax,eax
	mov ax,ds
	shl eax,4
	add eax,LABEL_GDT
	mov dword [GdtPtr+2],eax
	;gdtr由段界限和段基址组成
	lgdt [GdtPtr]
	;关中断
	cli
	;开启a20地址环绕
	in al,92h
	or al,00000010b
	out 92h,al
	;置pe位为1，打开保护模式
	mov eax,cr0
	or eax,1
	mov cr0,eax
	;跳转到label_seg_code32标签处
	jmp dword SelectorCode32:0
;跳回实模式之后到这里重新设置一些东西
LABEL_REAL_ENTRY:
	mov ax,cs
	mov ds,ax
	mov es,ax
	mov ss,ax
	mov sp,[SPValueInRealMode]
	;关闭a20地址线
	in al,92h
	and al,11111101b
	out 92h,al
	;开中断
	sti
	;回到 DOS（调用dos中断号，结束程序，返回到DOS
	mov ax,4c00h
	int 21h
;运行在保护模式下，也就是完成后续的输出字符串，以及5mb基址测试工作
[SECTION .s32]
[BITS 32]

LABEL_SEG_CODE32:
	mov ax,SelectorData   	;加载数据段选择子
	mov ds,ax
	mov ax,SelectorTest 	;加载测试段选择子
	mov es,ax
	mov ax,SelectorVideo 	;加载视频段选择子
	mov gs,ax
	mov ax,SelectorStack 	;加载堆栈段选择子
	mov ss,ax
	mov esp,TopOfStack 		;栈底指针
	;下面显示一个字符串
	mov ah,0ch  			;黑底红字
	xor esi,esi  	
	xor edi,edi
	mov esi,offsetPMMessage ;为后面加载这个地址的字符串作准备	
	mov edi,(80*10+0)*2 	;屏幕的第10行第0列
	cld 					;这个指令使df=0，地址自动增量，配合lodsb使用
.1:
	lodsb 					;这里代表的意思是将[ds:esi]地址的内容给al寄存器
	test al,al 				;这里是做测试，看结果是否为0
	jz .2 					;为0跳转到.2
	mov [gs:edi],ax 		;ax的内容给这个显存首地址（黑底红字）
	add edi,2 				;指向下一个显存地址
	jmp .1 					;做一个循环
.2:
	call DispReturn 		;屏幕换行（也就是edi寄存器的内容应该是下一行的起始位置地址）
	call TestRead 			;读取初始状态label_desc_test描述符对应段的内容
	call TestWrite 			;将data段的字符串内容写进label_desc_test描述符对应段
	call TestRead 			;显示写进去之后的内容
	jmp SelectorCode16:0  	;这里将selectorcode16选择子给了cs寄存器，所以刷新了cs高速缓冲寄存器，使得其有一个合适的段界限和属性

TestRead:
	xor esi,esi
	mov ecx,8 ;只循环8次
.loop:
	mov al,[es:esi]
	call DispAL2   ;调用这个函数在显示器显示字符
	inc esi
	loop .loop
	call DispReturn
	ret

TestWrite:
	push esi
	push edi
	xor esi,esi
	xor edi,edi
	mov esi,offsetStrTest  ;载入字符串“ABCDEFG.......”
	cld
.1:
	lodsb
	test al,al
	jz .2
	mov [es:edi],al   ;这里就向LABEL_DESC_TEST所描述段写入字符
	inc edi
	jmp .1
.2:
	pop edi   ;这里是为了为下面的显示testread作准备，使得显存在这一行的开头
	pop esi
	ret

DispAL:
	push ecx
	push edx
	mov ah,0ch
	mov dl,al
	shr al,4
	mov ecx,2
.begin:
	and al,01111b
	cmp al,9
	ja .1
	add al,'0'
	jmp .2
.1:
	sub al,0ah
	add al,'A'
.2:
	mov byte[gs:edi],al
	add edi,2
	mov al,dl
	loop .begin
	add edi,2
	pop edx
	pop ecx
	ret

DispAL2:
	mov [gs:edi],ax		;这里是csdn博客做的一个小改动而不是调用dispal函数来输出
	add edi,2
	ret

DispReturn:
	push eax   		;保存进入函数之前的值，方便后面用
	push ebx
	mov eax,edi 	;获取当前光标的显存偏移
	mov bl,160  	;一行80字符，一个字符2字节
	div bl 			;这里是为了得到行数（商也就是行数）再将其存放在al寄存器中
	and eax,0ffh 	;这里是为了只用al寄存器内容
	inc eax 		;这里是为了取得下一行
	mov bl,160  	;这里是为了为下面的乘法作铺垫
	mul bl 			;这里是为了得到下一行起始显存偏移地址
	mov edi,eax 	;更新显存偏移
	pop ebx
	pop eax
	ret

segCode32Len equ $ - LABEL_SEG_CODE32

[SECTION .s16code]
ALIGN 32
[BITS 16]
LABEL_SEG_CODE16:
	;下面都是在跳到实模式之前将mormal描述符赋予给这些寄存器，以便刷新段描述符高速缓冲寄存器使得他又合适的属性和界限，不然切会实模式会出现错误，cs不用这种方式是因为cs寄存器不能用于mov指令，但是jmp和call可以所以cs在前面.s32段跳转到这个段的时候就已经做了
	mov ax,SelectorNormal
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax
	;这里是将pe位置0切换到实模式
	mov eax,cr0
	and al,11111110b
	mov cr0,eax
;LABEL_GO_BACK_TO_REAL+3已经是cs段寄存器的值了，所以会直接跳转到.s16段中去
LABEL_GO_BACK_TO_REAL:
	jmp 0:LABEL_REAL_ENTRY  ;注意这里的0并不是cs寄存器的值，而是之前做的那个骚操作才是他的值

Code16len equ $-LABEL_SEG_CODE16


