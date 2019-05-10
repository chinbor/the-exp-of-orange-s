
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            start.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"

PUBLIC	void*	memcpy(void* pDst, void* pSrc, int iSize);

PUBLIC	void	disp_str(char * pszInfo);

PUBLIC	u8		gdt_ptr[6];	/* 0~15:Limit  16~47:Base */
PUBLIC	DESCRIPTOR	gdt[GDT_SIZE];  /* 128个DESCRIPTOR */

PUBLIC void cstart()
{

	disp_str("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		 "-----\"cstart\" begins-----\n");

	/* 将 LOADER 中的 GDT 复制到新的 GDT 中 */
	memcpy(&gdt,				   /* New GDT ,这里的&gdt是数组指针，也就是数组的首地址，值为0x00031820*/
	       (void*)(*((u32*)(&gdt_ptr[2]))),    /* Base  of Old GDT, 值为0x0009013d */
	       *((u16*)(&gdt_ptr[0])) + 1	   /* Limit of Old GDT ，值为0x20*/
		);
	/* gdt_ptr[6] 共 6 个字节：0~15:Limit  16~47:Base。用作 sgdt/lgdt 的参数。*/
	u16* p_gdt_limit = (u16*)(&gdt_ptr[0]);      /* p_gdt_limit值为：0x31c20 */
	u32* p_gdt_base  = (u32*)(&gdt_ptr[2]);      /* p_gdt_base值为:0x31c22 */
	*p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;   /* 0x31c20地址的内容为0x03ff */
	*p_gdt_base  = (u32)&gdt;                       /* 0x31c22地址内容为0x00031820 */
	disp_str("-----\"cstart\" ends-----\n");
}
