
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               kernel.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

; 导入函数
extern	cstart				; 再start.c中
extern	kernel_main			; 在main.c中
extern	exception_handler	; 再protect.c中
extern	spurious_irq		; 在i8259.c中
extern	disp_str			; 再kliba.asm中 
extern	delay				; 再klib.c中
extern	clock_handler		; 在clock.c中

; 导入全局变量
extern	gdt_ptr			; 再global.h中
extern	idt_ptr			; 在global.h中
extern	disp_pos		; 再global.h中
extern	p_proc_ready	; 再global.h中
extern	tss				; 再global.h中
extern	k_reenter		; 再global.h中声明

bits	32

[SECTION .bss]
StackSpace		resb	2 * 1024
StackTop:		; 栈顶

[SECTION .data]
clock_int_msg	db		"^",0


[section .text]	; 代码在此

global _start	; 导出 _start  地址为:0x30400,特权级为0

global restart

global divide_error
global single_step_exception
global nmi
global breakpoint_exception
global overflow
global bounds_check
global inval_opcode
global copr_not_available
global double_fault
global copr_seg_overrun
global inval_tss
global segment_not_present
global stack_exception
global general_protection
global page_fault
global copr_error
global  hwint00
global  hwint01
global  hwint02
global  hwint03
global  hwint04
global  hwint05
global  hwint06
global  hwint07
global  hwint08
global  hwint09
global  hwint10
global  hwint11
global  hwint12
global  hwint13
global  hwint14
global  hwint15


_start:     ;地址为0x30400

	; 把 esp 从 LOADER 挪到 KERNEL
	mov	esp, StackTop				; 堆栈在 bss 段中

	mov dword [disp_pos],0			; 初始化值为0

	sgdt	[gdt_ptr]				; 将gdtr寄存器中的值给到gdt_ptr地址所指的内存处
	call	cstart					; 在此函数中改变了gdt_ptr地址所指内存的内容（gdt基址和界限），让它指向&gdt，还有就是让idt_ptr地址所指内存处的内容指向了&idt，以及初始化了idt表,
	lgdt	[gdt_ptr]				; 使用新的GDT，加载新的gdt基址和界限

	lidt	[idt_ptr]				; 加载idt
	
	jmp	SELECTOR_KERNEL_CS:csinit	
csinit:							
	;sti								; 设置if位，触发，硬件中断，因为通过ocw打开了键盘中断
	;hlt								; 处理器暂停执行
	
	xor eax,eax
	mov ax,SELECTOR_TSS					; tss描述符的选择子，只有发生外层向内层特权级变换的时候才会使用tss中的堆栈字段，不同特权级使用不同堆栈
	ltr ax								; 加载tss描述符的选择子

	jmp kernel_main						; 由于kernel.bin是加载再0x30000的，但是实际代码是在0x30400,所以这里的后面的代码都是再这个地址以上


; 中断和异常 -- 硬件中断
; ---------------------------------
%macro  hwint_master    1
        push    %1
        call    spurious_irq		
        add     esp, 4
        hlt
%endmacro
; ---------------------------------

ALIGN   16				; 16字节对齐，也就是地址必须是0x10的整数倍
hwint00:                ; Interrupt routine for irq 0 (the clock).由testa函数中的delay函数触发，由于涉及特权级的转换，所以需要tss中ring0的ss和esp，然后保存a进程的状态
		sub esp,4		; ----------------------------------------------------------------------------------------------------------	
		pushad			;
		push ds			;				保存进程A的寄存器，保存到的是进程表中，因为esp指向的就是进程表的p_stacktop位置
		push es			;	
		push fs			;
		push gs			; ----------------------------------------------------------------------------------------------------------

		mov dx,ss		; 这里的ss是tss中ring0的ss所以值为selector_kernel_rw,初始化这几个寄存器
		mov ds,dx
		mov es,dx

		inc byte [gs:0]	
		mov al,EOI
		out INT_M_CTL,al
		
		inc dword [k_reenter]		;这是一个全局变量，初始值为-1，目的是为了防止中断的嵌套，也就是上一个中断还没执行结束，下一个中断就开始执行了
		cmp dword [k_reenter],0		;同0作比较，若不为0（说明一次中断还没处理完又发生了一次中断）就跳转
		jne .re_enter

		mov esp,StackTop  ;使用内核栈

		sti       ;可以不要，因为前面送eoi的时候就置if=1了

		push 0
		call clock_handler
		add esp,4

		cli				; 即便这里关了中断，也就是if位为0，但是之前保存的eflags中已经将if置位1了，所以后面出栈操作将会打开if位

		mov esp,[p_proc_ready]		; 加载新的进程表（这个值由进程调度模块来确定）
		lldt [esp + P_LDT_SEL]		; 新的进程表的ldt表的选择子
		lea eax,[esp+P_STACKTOP]
		mov dword [tss+TSS3_S_SP0],eax
.re_enter:
		dec dword [k_reenter]
		pop gs			; ---------------------------------------------------------------------------------------------------------- 
		pop fs          ;
		pop es          ;				恢复进程A的寄存器，这样才能恢复进程a状态，从而继续执行
		pop ds          ;	
		popad           ;
		add esp,4       ; ----------------------------------------------------------------------------------------------------------
        iretd			; 向进程a返回，0->1特权级的返回

ALIGN   16
hwint01:                ; Interrupt routine for irq 1 (keyboard)
        hwint_master    1

ALIGN   16
hwint02:                ; Interrupt routine for irq 2 (cascade!)
        hwint_master    2

ALIGN   16
hwint03:                ; Interrupt routine for irq 3 (second serial)
        hwint_master    3

ALIGN   16
hwint04:                ; Interrupt routine for irq 4 (first serial)
        hwint_master    4

ALIGN   16
hwint05:                ; Interrupt routine for irq 5 (XT winchester)
        hwint_master    5

ALIGN   16
hwint06:                ; Interrupt routine for irq 6 (floppy)
        hwint_master    6

ALIGN   16
hwint07:                ; Interrupt routine for irq 7 (printer)
        hwint_master    7

; ---------------------------------
%macro  hwint_slave     1
        push    %1
        call    spurious_irq
        add     esp, 4
        hlt
%endmacro
; ---------------------------------

ALIGN   16
hwint08:                ; Interrupt routine for irq 8 (realtime clock).
        hwint_slave     8

ALIGN   16
hwint09:                ; Interrupt routine for irq 9 (irq 2 redirected)
        hwint_slave     9

ALIGN   16
hwint10:                ; Interrupt routine for irq 10
        hwint_slave     10

ALIGN   16
hwint11:                ; Interrupt routine for irq 11
        hwint_slave     11

ALIGN   16
hwint12:                ; Interrupt routine for irq 12
        hwint_slave     12

ALIGN   16
hwint13:                ; Interrupt routine for irq 13 (FPU exception)
        hwint_slave     13

ALIGN   16
hwint14:                ; Interrupt routine for irq 14 (AT winchester)
        hwint_slave     14

ALIGN   16
hwint15:                ; Interrupt routine for irq 15
        hwint_slave     15



; 中断和异常 -- 异常
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0			; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1			; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2			; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3			; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4			; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5			; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6			; vector_no	= 6
	jmp	exception		; 没有用到
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7			; vector_no	= 7
	jmp	exception
double_fault:
	push	8			; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9			; vector_no	= 9
	jmp	exception
inval_tss:
	push	10			; vector_no	= A
	jmp	exception
segment_not_present:
	push	11			; vector_no	= B
	jmp	exception
stack_exception:
	push	12			; vector_no	= C
	jmp	exception
general_protection:
	push	13			; vector_no	= D
	jmp	exception
page_fault:
	push	14			; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16			; vector_no	= 10h
	jmp	exception

; 异常处理程序
exception:
	call	exception_handler	; 
	add	esp, 4*2				; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt


; ====================================================================================
;                                   restart,具体作用就是实现特权级0（内核）向特权级1的进程转换（让第一个进程执行）
; ====================================================================================
restart:
	mov	esp, [p_proc_ready]					; esp指向pcb的gs字段
	lldt	[esp + P_LDT_SEL]				; 加载ldt再gdt中的选择子到ldtr寄存器中
	lea	eax, [esp + P_STACKTOP]				; esp+72的值给eax,此时指向的就是p_stacktop字段
	mov	dword [tss + TSS3_S_SP0], eax		; tss中的ring0堆栈指针esp等于这个地址，目的是为了当中断发生，也就是当特权级1到特权级0转移的时候就会到tss中取这个ring0的esp域，然后将当前进程的ss，esp，eflags，cs，eip，错误码（如果有的话）进栈，从而达到保存当前进程状态的作用，当然还有一些选项没有保存，后面应该会陆续介绍

	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad

	add	esp, 4

	iretd									; 执行这一句后就会跳转到testa（）函数执行
