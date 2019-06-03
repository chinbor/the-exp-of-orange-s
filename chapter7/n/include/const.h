/* 一些常量的定义
 * const.h
 * chinbor
 * 19.5.13
*/

#ifndef	_ORANGES_CONST_H_
#define	_ORANGES_CONST_H_

/* EXTERN is defined as extern except in global.c */
#define EXTERN extern

/* 函数类型 */
#define	PUBLIC		/* PUBLIC is the opposite of PRIVATE,什么都没有相当于公有 */
#define	PRIVATE	static	/* PRIVATE x limits the scope of x ，只能文件自己用 */

/* Boolean */
#define	TRUE	1
#define	FALSE	0

/* Color */
/*
 * e.g. MAKE_COLOR(BLUE, RED)
 *      MAKE_COLOR(BLACK, RED) | BRIGHT
 *      MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH
 */
#define BLACK   0x0     /* 0000 */
#define WHITE   0x7     /* 0111 */
#define RED     0x4     /* 0100 */
#define GREEN   0x2     /* 0010 */
#define BLUE    0x1     /* 0001 */
#define FLASH   0x80    /* 1000 0000 */
#define BRIGHT  0x08    /* 0000 1000 */
#define MAKE_COLOR(x,y) (x | y) /* MAKE_COLOR(Background,Foreground) */

/* GDT 和 IDT 中描述符的个数,由于gdtr中的表界限为16位，所以最大为ffffh，所以长度为65539字节，但是一个描述符8字节，所以最多8192个描述符，我们这里只定义了128个描述符 */
#define	GDT_SIZE	128
/* 这里固定idt有256个描述符 */
#define IDT_SIZE    256

/* 权限 */
#define PRIVILEGE_KRNL	0
#define PRIVILEGE_TASK	1
#define PRIVILEGE_USER	3

/* RPL */
#define	RPL_KRNL	SA_RPL0      
#define	RPL_TASK	SA_RPL1		
#define	RPL_USER	SA_RPL3		

/* tty */
#define NR_CONSOLES		3  /* consoles */

/* 8259a interrupt controller ports */
#define INT_M_CTL		0x20     
#define INT_M_CTLMASK	0x21
#define INT_S_CTL		0xA0
#define INT_S_CTLMASK	0xA1

/* 8253/8254 PIT (Programmable Interval Timer) */
#define TIMER0         0x40		/* I/O port for timer channel 0 */
#define TIMER_MODE     0x43		/* I/O port for timer mode control */
#define RATE_GENERATOR 0x34		/* 00-11-010-0 :* Counter0 - LSB then MSB - rate generator - binary*/
#define TIMER_FREQ     1193182L /* clock frequency for timer in PC and AT，这里再整数后面加L表示浮点数 */
#define HZ             100		/* clock freq (software settable on IBM-PC) */

/* AT keyboard */
/* 8042 ports */
#define KB_DATA		0x60	/* I/O port for keyboard data
							   Read : Read Output Buffer
							   Write: Write Input Buffer(8042 Data&8048 Command) */
#define KB_CMD		0x64	/* I/O port for keyboard command
							   Read : Read Status Register
							   Write: Write Input Buffer(8042 Command) */
#define	LED_CODE	0xED
#define	KB_ACK		0xFA

/* VGA */
#define	CRTC_ADDR_REG	0x3D4	/* CRT Controller Registers - Addr Register */
#define	CRTC_DATA_REG	0x3D5	/* CRT Controller Registers - Data Register */
#define	START_ADDR_H	0xC		/* reg index of video mem start addr (MSB) */
#define	START_ADDR_L	0xD		/* reg index of video mem start addr (LSB) */
#define	CURSOR_H		0xE		/* reg index of cursor position (MSB) */
#define	CURSOR_L		0xF		/* reg index of cursor position (LSB) */
#define	V_MEM_BASE		0xB8000	/* base of color video memory */
#define	V_MEM_SIZE		0x8000	/* 32K: B8000H -> BFFFFH */

/* 硬件中断 */
#define	NR_IRQ			16	/* Number of IRQs */
#define	CLOCK_IRQ		0	/* 时钟中断号 */
#define	KEYBOARD_IRQ	1	/* 键盘中断号 */
#define	CASCADE_IRQ		2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ		3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ		4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ		5	/* xt winchester */
#define	FLOPPY_IRQ		6	/* floppy disk */
#define	PRINTER_IRQ		7
#define	AT_WINI_IRQ		14	/* at winchester */

/* system call */
#define NR_SYS_CALL		1

#endif /* _ORANGES_CONST_H_ */
