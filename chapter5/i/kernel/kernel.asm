
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               kernel.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

; ----------------------------------------------------------------------
; 编译连接方法:
; $ rm -f kernel.bin
; $ nasm -f elf -o kernel.o kernel.asm
; $ nasm -f elf -o string.o string.asm
; $ nasm -f elf -o klib.o klib.asm
; $ gcc -c -o start.o start.c
; $ ld -s -Ttext 0x30400 -o kernel.bin kernel.o string.o start.o klib.o
; $ rm -f kernel.o string.o start.o
; $ 
; ----------------------------------------------------------------------

SELECTOR_KERNEL_CS	equ	8	;这里用8是为了选中执行代码段也就是描述符LABEL_DESC_FLAT_C(一个描述符占8字节)

; 导入函数
extern	cstart				; 再start.c中
extern	exception_handler	; 再protect.c中
extern	spurious_irq		; 在i8259.c中

; 导入全局变量
extern	gdt_ptr  ; 再global.h中
extern	idt_ptr	 ; 在global.h中
extern	disp_pos ; 再global.h中

SECTION .bss]
StackSpace		resb	2 * 1024
StackTop:		; 栈顶

[section .text]	; 代码在此

global _start	; 导出 _start

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
	; 此时内存看上去是这样的（更详细的内存情况在 LOADER.ASM 中有说明）：
	;              ┃                                    ┃
	;              ┃                 ...                ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■Page  Tables■■■■■■┃
	;              ┃■■■■■(大小由LOADER决定)■■■■┃ PageTblBase
	;    00101000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■Page Directory Table■■■■┃ PageDirBase = 1M
	;    00100000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃□□□□ Hardware  Reserved □□□□┃ B8000h ← gs
	;       9FC00h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■LOADER.BIN■■■■■■┃ somewhere in LOADER ← esp
	;       90000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■KERNEL.BIN■■■■■■┃
	;       80000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃■■■■■■■■KERNEL■■■■■■■┃ 30400h ← KERNEL 入口 (KernelEntryPointPhyAddr)
	;       30000h ┣━━━━━━━━━━━━━━━━━━┫
	;              ┋                 ...                ┋
	;              ┋                                    ┋
	;           0h ┗━━━━━━━━━━━━━━━━━━┛ ← cs, ds, es, fs, ss
	;
	;
	; GDT 以及相应的描述符是这样的：
	;
	;		              Descriptors               Selectors
	;              ┏━━━━━━━━━━━━━━━━━━┓
	;              ┃         Dummy Descriptor           ┃
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_FLAT_C    (0～4G)     ┃   8h = cs
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_FLAT_RW   (0～4G)     ┃  10h = ds, es, fs, ss
	;              ┣━━━━━━━━━━━━━━━━━━┫
	;              ┃         DESC_VIDEO                 ┃  1Bh = gs
	;              ┗━━━━━━━━━━━━━━━━━━┛
	;
	; 注意! 在使用 C 代码的时候一定要保证 ds, es, ss 这几个段寄存器的值是一样的
	; 因为编译器有可能编译出使用它们的代码, 而编译器默认它们是一样的. 比如串拷贝操作会用到 ds 和 es.
	;
	;


	; 把 esp 从 LOADER 挪到 KERNEL
	mov	esp, StackTop				; 堆栈在 bss 段中,这里StackTop的地址为0x32960（利用bochs设置0x30400断点然后执行）

	mov dword [disp_pos],0			; 初始化值为0,地址为ds:0x32d80,ds的值为0x10

	sgdt	[gdt_ptr]				; cstart() 中将会用到 gdt_ptr(地址为0x32d84)，就是将gdtr中的值存放在0x32d84中，此时的gdt基址为0x0009013d，gdt界限为0x001f(32-1)
	call	cstart					; 在此函数中改变了gdt_ptr，让它指向新的GDT，还有就是初始化了idt表,cstart函数地址为0x30598
	lgdt	[gdt_ptr]				; 使用新的GDT，此时地址0x32d84的内容为新的gdt界限（0x03ff 2字节16位gdt界限）地址0x32d86内容为新的gdt基址=0x00032980

	lidt	[idt_ptr]				; idt_ptr的地址为0x32960，内容为idt表界限：0x07ff，地址0x32962内容为idt基址：0x32da0

	jmp	SELECTOR_KERNEL_CS:csinit	;csinit地址为0x30430
csinit:								;注意这里当利用bochs设置0x30430为断点的时候，然后按s执行（因为这样才会进函数），按n不会进函数，执行完毕后会卡顿，这个时候在虚拟机中按键盘，那么就会触发hwint01，就会跳转到函数spurious_irq
	sti								; 设置if位，触发，硬件中断，因为通过ocw打开了键盘中断
	hlt								; 处理器暂停执行



; 中断和异常 -- 硬件中断
; ---------------------------------
%macro  hwint_master    1
        push    %1
        call    spurious_irq		;spurious_irq地址为:0x306e1
        add     esp, 4
        hlt
%endmacro
; ---------------------------------

ALIGN   16				; 16字节对齐，也就是hwint00和hwint01地址之间相差0x10
hwint00:                ; Interrupt routine for irq 0 (the clock).
        hwint_master    0

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

exception:
	call	exception_handler	; 
	add	esp, 4*2				; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt
