
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
extern	irq_table		; 在global.h中声明
extern	sys_call_table	; 在global.h中声明

bits	32

[SECTION .bss]
StackSpace		resb	2 * 1024
StackTop:		; 栈顶

[SECTION .data]
clock_int_msg	db		"^",0

[section .text]	; 代码在此

global _start	; 导出 _start  地址为:0x30400,特权级为0

global restart

global sys_call

;导出中断例程函数

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
	call	cstart					; 在此函数中改变了gdt_ptr地址所指内存的内容（gdt基址和界限），让它指向&gdt，还有就是让idt_ptr地址所指内存处的内容指向了&idt，以及初始化了idt表,初始化了tss，每个进程的ldt表在gdt中的描述符
	lgdt	[gdt_ptr]				; 使用新的GDT，加载新的gdt基址和界限

	lidt	[idt_ptr]				; 加载idt
	
	jmp	SELECTOR_KERNEL_CS:csinit	
csinit:							
	
	xor eax,eax
	mov ax,SELECTOR_TSS					; tss描述符的选择子，只有发生外层向内层特权级变换的时候才会使用tss中的堆栈字段，不同特权级使用不同堆栈
	ltr ax								; 加载tss描述符的选择子

	jmp kernel_main						; 由于kernel.bin是加载再0x30000的，但是实际代码是在0x30400,所以这里的后面的代码都是再这个地址以上,注意这里是跳转不是call，跳转到这个函数执行，但是不会返回，所以一直向后执行


; 中断和异常 -- 外部中断
; -------------------------------------------------------------主8259a------------------------------------------------------------------------------------
%macro  hwint_master    1

		call save				; 使用save目的是为了简化中断例程，优化代码

		in al,INT_M_CTLMASK		; ---------------------------------------------------------------------------------------
		or al,(1 << %1)			;		关闭当前中断
		out INT_M_CTLMASK,al	; ---------------------------------------------------------------------------------------

		mov al,EOI				; 置EOI位,目的是将ISR寄存器中的对应中断的位复位，这样，才能允许同级和低优先级的中断响应，在这里也就是时钟中断的发生的同时，可以响应键盘中断，因为时钟中断是ir0，键盘中断是ir1，ir0>ir1
		out INT_M_CTL,al
		
		sti							; cpu在响应中断的过程中会自动关中断，这句之后就可以响应新的中断
		push %1
		call [irq_table + 4 * %1]	; 中断处理程序,加4×%1是因为每一个中断处理函数的地址为32位，也就是4字节
		pop ecx						; 这里的cx的值就是前面push %1的值
		cli						; 即便这里关了中断，也就是if位为0，但是之前保存的eflags中已经将if置位1了，所以后面出栈操作将会打开if位

		in al,INT_M_CTLMASK		; ------------------------------------
		and al,~(1 << %1)       ;		恢复当前中断
		out INT_M_CTLMASK,al    ; ------------------------------------

		ret						; 注意这一条ret指令，他表示返回的是restart（没有发生中断重入）或则restart_reenter（发生了中断重入）

%endmacro
; --------------------------------------------------------------------------------------------------------------------------------------------------------

ALIGN   16				; 16字节对齐，也就是地址必须是0x10的整数倍,注意这里的中断号再实际的idt表中都是加了0x20的，也就是hwint00对应0x20
hwint00:				; Interrupt routine for irq 0 (the clock).由testa函数中的delay函数触发，由于涉及特权级的转换，所以需要tss中ring0的ss和esp，然后保存a进程的状态
		hwint_master	0

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

; -------------------------------------------------------------从8259a------------------------------------------------------------------------------------
%macro  hwint_slave     1
        push    %1
        call    spurious_irq
        add     esp, 4
        hlt
%endmacro
; --------------------------------------------------------------------------------------------------------------------------------------------------------

ALIGN   16
hwint08:                ; Interrupt routine for irq 8 (realtime clock).注意这里的中断号，再idt表中都是加了0x28的也就是hwint08对应的0x28
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
;                                   save
; ====================================================================================
save:
        pushad          ; `.
        push    ds      ;  |
        push    es      ;  | 保存原寄存器值
        push    fs      ;  |
        push    gs      ; /

		mov		esi,edx

        mov     dx, ss						; 就是我们前面初始化TSS的时候设置的ring0的ss为SELECTOR_KERNEL_DS=0x10;
        mov     ds, dx						
        mov     es, dx
		mov		fs, dx

		mov		edx, esi					; 系统调用的传入的参数，可能是p_msg,也可能是s
        mov     esi, esp                    ; esi = 进程表起始地址,因为前面一直再进栈，所以这时候esp到了进程表起始地址，也就是gs的位置

        inc     dword [k_reenter]           ; 解决中断重入
        cmp     dword [k_reenter], 0        ; 发生中断重入就跳转到.1,此时已经再内核栈了,因为因为第一次中断的时候是顺序执行，不会跳转到.1所以已经切换了内核栈，所以当中断重入的时候仍然是再内核栈中
        jne     .1                          ;
        mov     esp, StackTop               ; 切换到内核栈 
        push    restart                     ; 内核栈中push的
        jmp     [esi + RETADR - P_STACKBASE]; 这里就指向了retadr字段，所以就是call save的时候保存的下一条指令地址 
.1:                                         ;
        push    restart_reenter             ; 是在内核栈中push的 
        jmp     [esi + RETADR - P_STACKBASE]; 同上 

; ====================================================================================
;                                 sys_call
; ====================================================================================
sys_call:
		call	save                              ; 当发生系统调用的时候由于由ring3转到ring0所以需要保存ring3用户进程的当前上下文（执行状态，以便返回的时候返回到系统调用的下一条指令地址）
		sti
		push	esi								  ; 是进程表的gs那个位置的地址，也就是进程表顶

		push	dword [p_proc_ready]			  ; 代表的意思就是对应进程的进程表，因为后面的系统调用的处理函数会用到这个参数比如write就用到了p_proc_ready->nr_tty也就是进程对应的终端tty来显示
		push	edx
		push	ecx
		push	ebx
		call	[sys_call_table + eax * 4]
		add esp,4 * 4

		pop		esi
		mov		[esi + EAXREG - P_STACKBASE],eax  ;存放函数返回值,因为调用的函数有返回值的时候返回值存放在eax寄存器

		cli
		ret										  ; 注意这里是返回到的restart(若无中断嵌套)若有中断嵌套就会到restart_reenter

; ============================================================================================================================================================
;                                   restart,具体作用就是实现特权级0（内核）向特权级1的进程(tty_task)和特权级3的进程(TestA，TestB，TestC)转换
; ============================================================================================================================================================
restart:
	mov	esp, [p_proc_ready]					; p_proc_ready是一个结构体指针，表示的是这个结构体第一个元素的地址，然而这个结构体第一个元素是一个结构体，所以第一个元素的值也是一个地址，是所指向结构体第 一个元素的地址,所以这里就指向了gs字段，也就是进程表的首地址
	lldt	[esp + P_LDT_SEL]				; 加载ldt再gdt中的选择子到ldtr寄存器中
	lea	eax, [esp + P_STACKTOP]				; esp+72的值给eax,此时指向的就是p_stacktop字段
	mov	dword [tss + TSS3_S_SP0], eax		; tss中的ring0堆栈指针esp等于这个地址，目的是为了当中断发生，也就是当特权级1到特权级0转移的时候就会到tss中取这个ring0的esp域，然后将当前进程的ss，esp，efl ags，cs，eip，错误码（如果有的话）进栈，从而达到保存当前进程状态的作用
restart_reenter:
	dec dword [k_reenter]
	pop	gs
	pop	fs
	pop	es
	pop	ds
	popad
	add	esp, 4
	iretd									; 第一次的时候是先到的tty_task进程执行(ring1)这个进程的具体作用就是tty切换还有键盘的输入还有显示器的显示
