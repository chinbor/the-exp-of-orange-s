/* 一些变量的声明,global.c文件引入这个头文件就是对这些变量的定义,其余的文件引入这个头文件都是声明 
 * global.h
 * chinbor
 * 19.5.13
*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		          disp_pos;
EXTERN	u8		        gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	DESCRIPTOR	 gdt[GDT_SIZE];
EXTERN	u8			    idt_ptr[6];	/* 0~15:Limit  16~47:Base */
EXTERN	GATE	     idt[IDT_SIZE];

EXTERN	u32				 k_reenter;

EXTERN	TSS					   tss;
EXTERN	PROCESS*	  p_proc_ready;

EXTERN	int					 ticks;    /* 注意这个ticks，只要时钟中断发生，那么他就加1，时钟中断10ms发生一次，那么这个变量值超范围需要6年时间，所以再程序执行过程中，永远不会超范围(因为你不可能让计算机执行6年多时间 ^.^) */

EXTERN	int				nr_current_console;

extern	PROCESS		  proc_table[];
extern	char		  task_stack[];
extern	TASK		  task_table[];
extern	TASK		  user_proc_table[];
extern	irq_handler	   irq_table[];
extern	system_call	  sys_call_table[];
extern	TTY				tty_table[];
extern	CONSOLE			console_table[];
