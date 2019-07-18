/* 一些常量的定义
 * const.h
 * chinbor
 * 19.5.13
*/

#ifndef	_ORANGES_CONST_H_
#define	_ORANGES_CONST_H_

/* the assert macro */
#define ASSERT
#ifdef ASSERT
/* 先声明再使用 */
void assertion_failure(char *exp, char *file, char *base_file, int line);
/* 对于这里的#exp，他的意思是将exp转化为字符串,后面的__FILE__,__BASE_FILE__,__LINE__都是gcc内置宏 */
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
#else
#define assert(exp)
#endif

/* EXTERN is defined as extern except in global.c */
#define EXTERN extern

/* 函数类型 */
#define	PUBLIC		/* PUBLIC is the opposite of PRIVATE,什么都没有相当于公有 */
#define	PRIVATE	static	/* PRIVATE x limits the scope of x ，只能文件自己用 */

#define	STR_DEFAULT_LEN	1024

/* max() & min() */
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))

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
#define MAKE_COLOR(x,y) ((x<<4) | y) /* MAKE_COLOR(Background,Foreground) */

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

/* Process */
#define SENDING   0x02	/* set when proc trying to send */
#define RECEIVING 0x04	/* set when proc trying to recv */

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

/* tasks */
/* 注意 TASK_XXX 的定义要与 global.c 中对应 */
#define INVALID_DRIVER	-20
#define INTERRUPT	-10
#define TASK_TTY	0
#define TASK_SYS	1
#define TASK_HD		2
/* #define TASK_FS	3 */
/* #define TASK_MM	4 */
#define ANY		(NR_TASKS + NR_PROCS + 10)
#define NO_TASK		(NR_TASKS + NR_PROCS + 20)

/* system call */
#define NR_SYS_CALL		3

/* ipc */
#define SEND		1
#define RECEIVE		2
#define BOTH		3	/* BOTH = (SEND | RECEIVE) */

/* magic chars used by `printx' */
#define MAG_CH_PANIC	'\002'
#define MAG_CH_ASSERT	'\003'

/**
 * @enum msgtype
 * @brief MESSAGE types
 * 枚举类型，GET_TICKS=2,DEV_CLOSE=1002,后面的依次加1
 */
enum msgtype {
	/* 
	 * when hard interrupt occurs, a msg (with type==HARD_INT) will
	 * be sent to some tasks
	 */
	HARD_INT = 1,

	/* SYS task */
	GET_TICKS,

	/* message type for drivers */
	DEV_OPEN = 1001,
	DEV_CLOSE,
	DEV_READ,
	DEV_WRITE,
	DEV_IOCTL

};

#define	CNT		u.m3.m3i2
#define	REQUEST		u.m3.m3i2
#define	PROC_NR		u.m3.m3i3
#define	DEVICE		u.m3.m3i4
#define	POSITION	u.m3.m3l1
#define	BUF		u.m3.m3p2

#define	RETVAL		u.m3.m3i1

#define	DIOCTL_GET_GEO	1

/* Hard Drive */
#define SECTOR_SIZE		512
#define SECTOR_BITS		(SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT	9

/* major device numbers (corresponding to kernel/global.c::dd_map[]) */
#define	NO_DEV			0
#define	DEV_FLOPPY		1
#define	DEV_CDROM		2
#define	DEV_HD			3
#define	DEV_CHAR_TTY	4
#define	DEV_SCSI		5
/* make device number from major and minor numbers */
#define	MAJOR_SHIFT		8
#define	MAKE_DEV(a,b)		((a << MAJOR_SHIFT) | b)
/* separate major and minor numbers from device number */
#define	MAJOR(x)		((x >> MAJOR_SHIFT) & 0xFF)
#define	MINOR(x)		(x & 0xFF)

/* device numbers of hard disk */
#define	MINOR_hd1a		0x10
#define	MINOR_hd2a		0x20
#define	MINOR_hd2b		0x21
#define	MINOR_hd3a		0x30
#define	MINOR_hd4a		0x40

#define	ROOT_DEV		MAKE_DEV(DEV_HD, MINOR_BOOT)	/* 3, 0x21 */

#define	INVALID_INODE		0
#define	ROOT_INODE			1

#define	MAX_DRIVES			2
#define	NR_PART_PER_DRIVE	4
#define	NR_SUB_PER_PART		16
#define	NR_SUB_PER_DRIVE	(NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define	NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)

/**
 * @def MAX_PRIM_DEV
 * Defines the max minor number of the primary partitions.
 * If there are 2 disks, prim_dev ranges in hd[0-9], this macro will
 * equals 9.
 */
#define	MAX_PRIM		(MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)

#define	MAX_SUBPARTITIONS	(NR_SUB_PER_DRIVE * MAX_DRIVES)

#define	P_PRIMARY	0
#define	P_EXTENDED	1

#define ORANGES_PART	0x99	/* Orange'S partition */
#define NO_PART		0x00	/* unused entry */
#define EXT_PART	0x05	/* extended partition */

#define	NR_FILES	64
#define	NR_FILE_DESC	64	/* FIXME */
#define	NR_INODE	64	/* FIXME */
#define	NR_SUPER_BLOCK	8


/* INODE::i_mode (octal, lower 32 bits reserved) */
#define I_TYPE_MASK     0170000
#define I_REGULAR       0100000
#define I_BLOCK_SPECIAL 0060000
#define I_DIRECTORY     0040000
#define I_CHAR_SPECIAL  0020000
#define I_NAMED_PIPE	0010000

#define	is_special(m)	((((m) & I_TYPE_MASK) == I_BLOCK_SPECIAL) ||	\
			 (((m) & I_TYPE_MASK) == I_CHAR_SPECIAL))


#define	NR_DEFAULT_FILE_SECTS	2048 /* 2048 * 512 = 1MB */

#endif /* _ORANGES_CONST_H_ */
