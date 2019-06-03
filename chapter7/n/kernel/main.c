

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
	// 指向事先准备好的进程数组的首地址
	TASK*	 p_task = task_table;
	// 指向第一个进程表的首地址
	PROCESS* p_proc	= proc_table;
	// 进程堆栈栈顶指针
	char*	 p_task_stack = task_stack + STACK_SIZE_TOTAL;
	// 第一个ldt在gdt中的选择子
	u16		 selector_ldt = SELECTOR_LDT_FIRST;
	int i;
	u8	privilege;
	u8	rpl;
	int eflags;
	// 利用循环语句初始化NR_TASKS个进程表
	for(i=0;i<NR_TASKS+NR_PROCS;i++){
		if(i < NR_TASKS){   /* 任务 */
			p_task		= task_table + i;
			privilege	= PRIVILEGE_TASK;
			rpl			= RPL_TASK;
			eflags		= 0x1202;     /* iopl=1;in，out等指令，只有当cpl<=iopl的时候才能执行 */
		}else{				/* 用户进程 */
			p_task		= user_proc_table + (i - NR_TASKS);
			privilege	= PRIVILEGE_USER;
			rpl			= RPL_USER;
			eflags		= 0x202;
		}
		// 初始化进程表中的name字段
		strcpy(p_proc->p_name,p_task->name);

		// 初始化id字段
		p_proc->pid = i;

		// 等于0x28，也就是gdt中的第六个选择子rpl=0,索引为5
		p_proc->ldt_sel	= selector_ldt;

		// 将gdt中的序号为1选择子的8个字节复制到ldt表的第一个描述符位置,填充第一个局部描述符
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;		// change the DPL=1

		// 将gdt中的序号为2选择子的8个字节复制到ldt表的第二个描述符位置,填充第二个局部描述符
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;	// change the DPL=1

		// 初始化cs的值为 0x0005,rpl为1,打开了允许ldt的那个标志，所以这个选择子对应的就是ldt表中的选择子，而不是gdt中的，对应的ldt表第一个描述符
		p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->regs.eip= (u32) p_task->initial_eip;
		// esp指向一个堆栈大小为0x8000+0x8000的顶点，将这个作为进程的堆栈
		p_proc->regs.esp= (u32) p_task_stack;
		p_proc->regs.eflags = eflags;	// IF=1, IOPL=1, bit 2 is always 1.打开if位,这样才能允许中断
		// 因为每一个进程的堆栈大小都是0x8000
		p_task_stack -= p_task->stacksize;
		// 指向下一个进程表
		p_proc++;
		// 指向下一个进程（事先准备好了进程体(也就是一个进程的具体代码)，堆栈大小，进程名）
		p_task++;
		// 指向下一个在gdt中的选择子，也就是另一个进程的ldt表所在的选择子(ldt描述符)
		selector_ldt += 1 << 3;
	}
	// 初始化每个进程表的ticks和priority,加入tty_task这个任务
	proc_table[0].ticks = proc_table[0].priority = 15;
	proc_table[1].ticks = proc_table[1].priority = 10;
	proc_table[2].ticks = proc_table[2].priority =  5;
	proc_table[3].ticks = proc_table[3].priority =  3;

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

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	while(1){
	//	disp_str("B.");
		milli_delay(10);     // 在这里触发中断程序
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	while(1){
	//	disp_str("C.");
		milli_delay(10);     // 在这里触发中断程序
	}
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	while(1){
	//	disp_str("D.");
		milli_delay(10);     // 在这里触发中断程序
	}
}
