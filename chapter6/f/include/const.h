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

/* 8259a interrupt controller ports */
#define INT_M_CTL		0x20     
#define INT_M_CTLMASK	0x21
#define INT_S_CTL		0xA0
#define INT_S_CTLMASK	0xA1


#endif /* _ORANGES_CONST_H_ */
