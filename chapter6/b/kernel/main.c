

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");
	// 指向第一个进程控制块的首地址
	PROCESS* p_proc	= proc_table;
	// 等于0x28，也就是gdt中的第六个选择子rpl=0,索引为5
	p_proc->ldt_sel	= SELECTOR_LDT_FIRST;
	// 将gdt中的序号为1选择子的8个字节复制到ldt表的第一个描述符位置,填充第一个局部描述符
	memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(DESCRIPTOR));
	p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;		// change the DPL=1
	// 将gdt中的序号为2选择子的8个字节复制到ldt表的第二个描述符位置,填充第二个局部描述符
	memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(DESCRIPTOR));
	p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;	// change the DPL=1
	// 初始化cs的值为 0x0005,rpl为1,打开了允许ldt的那个标志，所以这个选择子对应的就是ldt表中的选择子，而不是gdt中的
	p_proc->regs.cs	= (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	// 初始化ds,es,fs,ss的值为 0x000d,rpl为1
	p_proc->regs.ds	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	p_proc->regs.es	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	p_proc->regs.fs	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	p_proc->regs.ss	= (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
	// 初始化为显存首址
	p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | RPL_TASK;
	p_proc->regs.eip= (u32)TestA;
	// esp指向一个堆栈大小为0x8000的顶点，将这个作为进程的堆栈
	p_proc->regs.esp= (u32) task_stack + STACK_SIZE_TOTAL;
	p_proc->regs.eflags = 0x1202;	// IF=1, IOPL=1, bit 2 is always 1.打开if位,这样才能允许中断

	p_proc_ready	= proc_table;
	restart();

	while(1){}
}

/*======================================================================*
                               TestA
 *======================================================================*/
// TestA函数地址为0x30724
void TestA()
{
	int i = 0;
	while(1){
		disp_str("A");
		disp_int(i++);
		disp_str(".");
		delay(1);     // 在这里触发中断程序
	}
}
