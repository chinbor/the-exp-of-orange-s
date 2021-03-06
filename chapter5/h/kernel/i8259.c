/* 初始化8259a可编程中断寄存器
 * i8259.c
 * chinbor
 * 19.5.13
*/

#include "const.h"
#include "type.h"
#include "protect.h"
#include "proto.h"

PUBLIC void init_8259A()
{
	/* 主8259a可编程中断控制器，icw1，INT_M_CTL=0x20,函数再kliba.asm中,地址为0x30a3d */
	out_byte(INT_M_CTL,0x11);

	/* 从8259a，icw1，INT_S_CTL=0xA0 */
	out_byte(INT_S_CTL,0x11);

	/* 主8259a，icw2，INT_M_CTLMASK=0x21,INT_VECTOR_IRQ0=0x20 */
	out_byte(INT_M_CTLMASK,INT_VECTOR_IRQ0);

	/* 从8259a，icw2，INT_S_CTLMASK=0xA1,INT_VECTOR_IRQ8=0x28 */
	out_byte(INT_S_CTLMASK,INT_VECTOR_IRQ8);

	/* 主 8259a, ICW3. IR2 对应 '从8259a'. */
	out_byte(INT_M_CTLMASK,	0x4);

	/* 从8259a, ICW3. 对应 '主8259' 的 IR2. */
	out_byte(INT_S_CTLMASK,	0x2);

	/* 主8259a, ICW4. */
	out_byte(INT_M_CTLMASK,	0x1);

	/* 从8259a, ICW4. */
	out_byte(INT_S_CTLMASK,	0x1);

	/* 主8259a, OCW1.屏蔽主8259的所有中断  */
	out_byte(INT_M_CTLMASK,	0xFF);

	/* 从8259a, OCW1.屏蔽从8259的所有中断  */
	out_byte(INT_S_CTLMASK,	0xFF);
}
