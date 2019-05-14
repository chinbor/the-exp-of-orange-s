/*  
 * protect.c
 * chinbro
 * 19.5.13
*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "global.h"
#include "proto.h"

/* 本文件内函数声明 */
PRIVATE void init_idt_desc(unsigned char vector, u8 desc_type,
			   int_handler handler, unsigned char privilege);

/* 中断处理函数 */
void	divide_error();
void	single_step_exception();
void	nmi();
void	breakpoint_exception();
void	overflow();
void	bounds_check();
void	inval_opcode();
void	copr_not_available();
void	double_fault();
void	copr_seg_overrun();
void	inval_tss();
void	segment_not_present();
void	stack_exception();
void	general_protection();
void	page_fault();
void	copr_error();

/*======================================================================*
                            init_prot
 *======================================================================*/
PUBLIC void init_prot()
{
	/* 再i8259.c中,函数地址为：0x30525 */
	init_8259A();

	// 全部初始化成中断门(没有陷阱门),init_idt_desc函数地址为0x30747，而divide_erro函数地址为0x30433
	init_idt_desc(INT_VECTOR_DIVIDE,	DA_386IGate,
		      divide_error,		PRIVILEGE_KRNL);
	// single_step_exception函数地址为0x30439
	init_idt_desc(INT_VECTOR_DEBUG,		DA_386IGate,
		      single_step_exception,	PRIVILEGE_KRNL);
	// nmi函数地址为:0x3043f
	init_idt_desc(INT_VECTOR_NMI,		DA_386IGate,
		      nmi,			PRIVILEGE_KRNL);
	// breakpoint_exception函数地址为:0x30445
	init_idt_desc(INT_VECTOR_BREAKPOINT,	DA_386IGate,
		      breakpoint_exception,	PRIVILEGE_USER);
	// overflow函数地址:0x3044b
	init_idt_desc(INT_VECTOR_OVERFLOW,	DA_386IGate,
		      overflow,			PRIVILEGE_USER);
	// bounds_check地址：0x30451
	init_idt_desc(INT_VECTOR_BOUNDS,	DA_386IGate,
		      bounds_check,		PRIVILEGE_KRNL);
	// inval_opcode地址：0x30457
	init_idt_desc(INT_VECTOR_INVAL_OP,	DA_386IGate,
		      inval_opcode,		PRIVILEGE_KRNL);
	// copr_not_available地址:0x3045d
	init_idt_desc(INT_VECTOR_COPROC_NOT,	DA_386IGate,
		      copr_not_available,	PRIVILEGE_KRNL);
	// double_fault地址：0x30463
	init_idt_desc(INT_VECTOR_DOUBLE_FAULT,	DA_386IGate,
		      double_fault,		PRIVILEGE_KRNL);
	// copr_seg_overrun地址：0x30467
	init_idt_desc(INT_VECTOR_COPROC_SEG,	DA_386IGate,
		      copr_seg_overrun,		PRIVILEGE_KRNL);
	// inval_tss地址：0x3046d
	init_idt_desc(INT_VECTOR_INVAL_TSS,	DA_386IGate,
		      inval_tss,		PRIVILEGE_KRNL);
	// segment_not_present地址:0x30471
	init_idt_desc(INT_VECTOR_SEG_NOT,	DA_386IGate,
		      segment_not_present,	PRIVILEGE_KRNL);
	// stack_exception地址:0x30475
	init_idt_desc(INT_VECTOR_STACK_FAULT,	DA_386IGate,
		      stack_exception,		PRIVILEGE_KRNL);
	// general_protection地址:0x30479
	init_idt_desc(INT_VECTOR_PROTECTION,	DA_386IGate,
		      general_protection,	PRIVILEGE_KRNL);
	// page_fault地址:0x3047d
	init_idt_desc(INT_VECTOR_PAGE_FAULT,	DA_386IGate,
		      page_fault,		PRIVILEGE_KRNL);
	// copr_error地址:0x30481
	init_idt_desc(INT_VECTOR_COPROC_ERR,	DA_386IGate,
		      copr_error,		PRIVILEGE_KRNL);
}

/*======================================================================*
                             init_idt_desc
 *----------------------------------------------------------------------*
 初始化 386 中断门
 *======================================================================*/
PRIVATE void init_idt_desc(unsigned char vector, u8 desc_type,
			  int_handler handler, unsigned char privilege)
{
	GATE *	p_gate	= &idt[vector];
	u32	base	= (u32)handler;
	p_gate->offset_low	= base & 0xFFFF;
	p_gate->selector	= SELECTOR_KERNEL_CS;
	p_gate->dcount		= 0;
	p_gate->attr		= desc_type | (privilege << 5);
	p_gate->offset_high	= (base >> 16) & 0xFFFF;
}

/*======================================================================*
                            exception_handler
 *----------------------------------------------------------------------*
 异常处理
 *======================================================================*/
PUBLIC void exception_handler(int vec_no,int err_code,int eip,int cs,int eflags)
{
	int i;
	int text_color = 0x74; /* 灰底红字 */

	char * err_msg[] = {"#DE Divide Error",
			    "#DB RESERVED",
			    "--  NMI Interrupt",
			    "#BP Breakpoint",
			    "#OF Overflow",
			    "#BR BOUND Range Exceeded",
			    "#UD Invalid Opcode (Undefined Opcode)",
			    "#NM Device Not Available (No Math Coprocessor)",
			    "#DF Double Fault",
			    "    Coprocessor Segment Overrun (reserved)",
			    "#TS Invalid TSS",
			    "#NP Segment Not Present",
			    "#SS Stack-Segment Fault",
			    "#GP General Protection",
			    "#PF Page Fault",
			    "--  (Intel reserved. Do not use.)",
			    "#MF x87 FPU Floating-Point Error (Math Fault)",
			    "#AC Alignment Check",
			    "#MC Machine Check",
			    "#XF SIMD Floating-Point Exception"
	};

	/* 通过打印空格的方式清空屏幕的前五行，并把 disp_pos 清零 */
	disp_pos = 0;
	for(i=0;i<80*5;i++){
		disp_str(" ");//地址为0x309c0
	}
	disp_pos = 0;
	// 函数disp_color_str地址为:0x309fe
	disp_color_str("Exception! --> ", text_color);
	disp_color_str(err_msg[vec_no], text_color);
	disp_color_str("\n\n", text_color);
	disp_color_str("EFLAGS:", text_color);
	// 函数disp_int地址为：0x30991,eflags的值为0x00010002
	disp_int(eflags);
	disp_color_str("CS:", text_color);
	disp_int(cs);
	disp_color_str("EIP:", text_color);
	disp_int(eip);

	if(err_code != 0xFFFFFFFF){
		disp_color_str("Error code:", text_color);
		disp_int(err_code);
	}
}

