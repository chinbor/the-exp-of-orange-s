
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	Backspace: 删除一个字符
	Enter:	把光标前进到下一行开头
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

/*======================================================================*
			   init_screen
1.初始化每个console
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	// 取当前tty对应的console
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	// 每一个控制台占用4000个字符，也就是8000个字节,因为4000/80=50，所以下一个控制台刚好再51行的开头第一个字符,自己的一些计算得出的，书中的没对齐，所以自己就改变了这个数值^.^
	int con_v_mem_size                   = 4000;

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
3.进行换行和退格的判断
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	// 在显存中的地址,不乘以2代表的是第几个字符，乘以二之后就代表的是地址，每个字符两字节,注意这里的这个显存地址，他应该是在这个进程的地址空间中的，应该是再这个进程（task_tty）数据段中进行的操作，因为需要向显存表示的地址写内容，数据段才能读写
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
	case '\n':
		// 判断光标是否在console的最后一行，因为最后一行回车是没用的
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCR_WIDTH) {
			// 光标指到下一行的第一个字符
			p_con->cursor = p_con->original_addr + SCR_WIDTH * 
				((p_con->cursor - p_con->original_addr) /
				 SCR_WIDTH + 1);
		}
		break;
	case '\b':
		// 判断光标是否大于console的起始位置（用的是字符个数为单位，不要乘以二），因为要有字符才可以删除
		if (p_con->cursor > p_con->original_addr) {
			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = 0x07;
		}
		break;
	case '\t':
		if((SCR_WIDTH * ((p_con->cursor - 0) / SCR_WIDTH)) <= p_con->cursor &&
			p_con->cursor <= ((SCR_WIDTH * ((p_con->cursor - 0) / SCR_WIDTH + 1)) - 4)) {
				p_con->cursor+=3;
		}	
		break;
	default:
		// 注意这里为什么用的是<而不是<=，因为若果你为<=那么若你在控制台最后一行的最后一个字符敲了后，那么光标就会跑到下一个控制台的第一行的第一个字符，这样后面的while循环满足判断条件所以会进入，然而在scroll_screen()函数中并没有改变while循环中的任何变量值，所以会无限循环（死循环，所以程序必然会卡死）
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
		}
		break;
	}
	// 当满足条件时，注意这里的current_start_addr变量，由于向上卷屏，所以每次他都会加80也就是不断更新的
	while (p_con->cursor >= p_con->current_start_addr + SCR_SIZE) {
		scroll_screen(p_con, SCR_UP);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
1.设置光标跟随字符
2.设置更新后的显存位置
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
	if(is_current_console(p_con)){
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
	}
}

/*======================================================================*
			    set_cursor
1.设置光标位置,参数为第几个字符
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

	flush(&console_table[nr_console]);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	// 向下
	if (direction == SCR_DN) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCR_WIDTH;
		}
	}
	// 向上
	else if (direction == SCR_UP) {
		// 注意这个判断语句，意思就是每shift+up一次，那么现在的开始地址就加一行（80个字符）,当按了26次，那么不成立，此时刚好current_start_addr=25*80，所以再第26行，所以末尾刚好再第50行，所以不会显示下一个console的标志1#(因为他在51行).
		if (p_con->current_start_addr + SCR_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCR_WIDTH;
		}
	}
	else{
	}

	flush(p_con);
}
