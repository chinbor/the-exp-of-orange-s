
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            start.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC void cstart()
{
	/* 这个函数再kliba.asm中*/
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n-----\"cstart\" begins-----\n");

	/* 将 LOADER 中的 GDT 复制到新的 GDT 中 ,这个函数再string.asm中 */
	memcpy(&gdt,								/* New GDT ,这里的&gdt是数组指针，也就是数组的首地址*/
	       (void*)(*((u32*)(&gdt_ptr[2]))),     /* Base  of Old GDT,void * 表示可以使任意类型的指针 */
	       *((u16*)(&gdt_ptr[0])) + 1			/* Limit of Old GDT*/
		);
	/* gdt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sgdt/lgdt 的参数。*/
	u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);				/* p_gdt_limit */
	u32* p_gdt_base  = (u32*)(&gdt_ptr[2]);				/* p_gdt_base */
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;   /* 新的gdt界限 */ 
	*p_gdt_base  = (u32)&gdt;							/* 新的gdt表的基址 */
	
	/* idt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sidt/lidt 的参数。*/
	u16* p_idt_limit = (u16*)(&idt_ptr[0]);			/* p_idt_limit */
	u32* p_idt_base  = (u32*)(&idt_ptr[2]);			/* p_idt_base */
	*p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;		/* idt界限 */
	*p_idt_base  = (u32)&idt;						/* idt表的基址 */
	
	/* 再protect.c中*/
	init_prot();

	disp_str("-----\"cstart\" ends-----\n");
}
