

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");
	// task_table是一个事先初始化好了拥有进程体，进程名，进程堆栈的结构体数组
	struct task*	 p_task;
	// 指向第一个进程表的首地址
	struct proc*	 p_proc = proc_table;
	// 进程堆栈栈顶指针
	char*	 p_task_stack = task_stack + STACK_SIZE_TOTAL;
	// 第一个ldt在gdt中的选择子
	u16		 selector_ldt = SELECTOR_LDT_FIRST;
	u8	privilege;
	u8	rpl;
	int i;
	int eflags;
	int prio;
	// 利用循环语句初始化NR_TASKS+NR_PROCS个进程表
	for(i=0;i<NR_TASKS+NR_PROCS;i++){
		if(i < NR_TASKS){   /* 任务,运行再ring1 */
			p_task		= task_table + i;
			privilege	= PRIVILEGE_TASK;
			rpl			= RPL_TASK;
			eflags		= 0x1202;     /* iopl=1;in，out等指令，只有当cpl<=iopl的时候才能执行 */
			prio		= 15;
		}else{				/* 用户进程，运行再ring3 */
			p_task		= user_proc_table + (i - NR_TASKS);
			privilege	= PRIVILEGE_USER;
			rpl			= RPL_USER;
			eflags		= 0x202;
			prio		= 5;
		}
		// 初始化进程表中的name字段
		strcpy(p_proc->name,p_task->name);

		// 初始化id字段
		p_proc->pid = i;

		// 等于0x28，也就是gdt中的第六个选择子rpl=0,索引为5
		p_proc->ldt_sel	= selector_ldt;

		// 将gdt中的序号为1选择子的8个字节复制到ldt表的第一个描述符位置,填充第一个局部描述符
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(struct descriptor));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;		// change the DPL

		// 将gdt中的序号为2选择子的8个字节复制到ldt表的第二个描述符位置,填充第二个局部描述符
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(struct descriptor));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;	// change the DPL

		// 初始化cs的值为对应的进程的ldt表中的第一个描述符（代码段）,rpl为1（task）或3（user_proc）,打开了选择子中允许ldt的那个标志，所以这个选择子对应的就是ldt表中的选择子，而不是gdt中的，对应的ldt表第一个描述符
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		// ds,es,fs,ss都是对应进程的ldt表中的第二个描述符(数据段)
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->regs.eip= (u32) p_task->initial_eip;
		// esp指向一个堆栈大小为0x8000+0x8000的顶点，将这个作为进程的堆栈
		p_proc->regs.esp= (u32) p_task_stack;
		p_proc->regs.eflags = eflags;	// IF=1, IOPL=1, bit 2 is always 1.打开if位,这样才能允许中断
		// 初始化每一个进程的nr_tty = 0
		p_proc->nr_tty = 0;

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		p_proc->ticks = p_proc->priority = prio;

		// 因为每一个进程的堆栈大小都是0x8000
		p_task_stack -= p_task->stacksize;

		// 指向下一个进程表
		p_proc++;

		// 指向下一个进程（事先准备好了进程体(也就是一个进程的具体代码)，堆栈大小，进程名）
		p_task++;

		// 指向下一个在gdt中的选择子，也就是另一个进程的ldt表所在的选择子(ldt描述符)
		selector_ldt += 1 << 3;
	}

	proc_table[NR_TASKS + 0].nr_tty = 0;    /* TestA 控制台0*/
	proc_table[NR_TASKS + 1].nr_tty = 1;	 /* TestB 控制台1*/
	proc_table[NR_TASKS + 2].nr_tty = 1;	 /* TestC 控制台1*/


	// 为什么初始化为0，因为进入restart函数会dec减一
	k_reenter = 0;  
	// 初始化时钟中断发生次数初始值=0，然后每一次进入clock_handler值就+1
	ticks = 0;
	// 指向初始化好了的进程表	
	p_proc_ready = proc_table;

	init_clock();
	init_keyboard();

	restart();

	while(1){}
}

/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	while(1){
		printf("<Ticks:%02d>",get_ticks());
		milli_delay(200);     // 在这里触发中断程序
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	while(1){
		printf("B");
		milli_delay(200);     // 在这里触发中断程序
	}
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	// assert(0);
	while(1){
		printf("C");
		milli_delay(200);     // 在这里触发中断程序
	}
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2"); // 内联汇编用法，ud2指令是停止运行
}
