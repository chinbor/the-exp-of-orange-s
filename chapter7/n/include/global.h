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

EXTERN	int					 ticks;

EXTERN	int				nr_current_console;

extern	PROCESS		  proc_table[];
extern	char		  task_stack[];
extern	TASK		  task_table[];
extern	TASK		  user_proc_table[];
extern	irq_handler	   irq_table[];
extern	system_call	  sys_call_table[];
extern	TTY				tty_table[];
extern	CONSOLE			console_table[];
