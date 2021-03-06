
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            start.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "global.h"

PUBLIC void cstart()
{
	/* 这个函数再kliba.asm中,地址为0x309c0 */
	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		 "-----\"cstart\" begins-----\n");

	/* 将 LOADER 中的 GDT 复制到新的 GDT 中 ,这个函数再string.asm中,地址为0x30a60 */
	memcpy(&gdt,				   /* New GDT ,这里的&gdt是数组指针，也就是数组的首地址，值为0x00031820*/
	       (void*)(*((u32*)(&gdt_ptr[2]))),    /* Base  of Old GDT, 值为0x0009013d */
	       *((u16*)(&gdt_ptr[0])) + 1	   /* Limit of Old GDT ，值为0x20*/
		);
	/* gdt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sgdt/lgdt 的参数。*/
	u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);      /* p_gdt_limit值为：0x31c24 */
	u32* p_gdt_base  = (u32*)(&gdt_ptr[2]);      /* p_gdt_base值为:0x31c26 */
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;   /* 0x31c24地址的内容为0x03ff */
	*p_gdt_base  = (u32)&gdt;                       /* 0x31c26地址内容为0x00031820 */
	
	/* idt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sidt/lidt 的参数。*/
	u16* p_idt_limit = (u16*)(&idt_ptr[0]);   /* p_idt_limit值为：0x31800 */
	u32* p_idt_base  = (u32*)(&idt_ptr[2]);   /* p_idt_base值为: 0x31802 */
	*p_idt_limit = IDT_SIZE * sizeof(GATE) - 1;  /* 0x31800地址内容为：0x07ff */
	*p_idt_base  = (u32)&idt;					/* 这里的意思是将这个数组首地址(也就是idt表的首址)转换成32位表示 ,0x31802地址的内容为：0x31c40*/
	
	/* 再protect.c中，地址为：0x305d9 */
	init_prot();
	/* 字符串所在地址为0x30aca */
	disp_str("-----\"cstart\" ends-----\n");
}
