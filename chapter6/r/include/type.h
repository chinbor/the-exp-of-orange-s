
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            type.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef	_ORANGES_TYPE_H_
#define	_ORANGES_TYPE_H_

typedef	unsigned int		u32;    /* 一个 int 4字节32位 */
typedef	unsigned short		u16;    /* 一个 short 2字节16位 */
typedef	unsigned char		u8;     /* 一个 char 1字节8位 */

typedef	void	(*int_handler)	();  /* 函数指针，int_handler是一个指针,他的值为地址 */
typedef void    (*task_f) ();
typedef void    (*irq_handler)	(int irq);
typedef void* system_call;			/* 只要是指针，都可以 */

#endif /* _ORANGES_TYPE_H_ */
