
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            const.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef	_ORANGES_CONST_H_
#define	_ORANGES_CONST_H_


/* 函数类型 */
#define	PUBLIC		/* PUBLIC is the opposite of PRIVATE,什么都没有相当于公有 */
#define	PRIVATE	static	/* PRIVATE x limits the scope of x ，只能文件自己用*/

/* GDT 和 IDT 中描述符的个数,由于gdtr中的表界限为16位，所以最大为ffffh，所以长度为65539字节，但是一个描述符8字节，所以最多8192个描述符，我们这里只定义了128个描述符 */
#define	GDT_SIZE	128 


#endif /* _ORANGES_CONST_H_ */
