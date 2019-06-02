
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);

/*======================================================================*
			   init_screen
1.初始化每个console
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	// 取当前tty对应的console
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;
	// 右移动1位，就变成了16k
	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */
	// 每一个console占用5k大小，因为一个80*25的屏幕占用了4k
	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;

	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}

/*======================================================================*
			   is_current_console
1.判断是否为tty对应的控制台
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	//进行判断是否为当前的控制台
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
1.显示字符
2.光标跟随字符
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	// 在显存中的地址,不乘以2代表的是第几个字符，乘以二之后就代表的是地址，每个字符两字节
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	*p_vmem++ = ch;
	*p_vmem++ = DEFAULT_CHAR_COLOR;
	// 下一个字符
	p_con->cursor++;

	// 光标跟随字符
	set_cursor(p_con->cursor);
}

/*======================================================================*
			    set_cursor
1.设置光标位置
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
传入的参数是第几个字符，而不是再显存中的偏移地址，因为偏移地址需要乘以二,
例如:传入5k，那么偏移地址就是10k，因为5k代表的是第5k个字符
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}

/*======================================================================*
			   select_console
1.切换到对应显存的位置（也就是不同控制台对应的显存位置）
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}
