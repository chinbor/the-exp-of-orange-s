
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
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
#include "keyboard.h"

#define	TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE	void init_tty(TTY* p_tty);
PRIVATE	void tty_do_read(TTY* p_tty);
PRIVATE	void tty_do_write(TTY* p_tty);

/*======================================================================*
                           task_tty(restart 后执行到了这个进程中)
1.初始化键盘中断程序
2.初始化TTY
3.循环read和write每一个tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;
	// 设置键盘中断以及键盘中断处理程序
	init_keyboard();
	// 初始化每一个TTY结构体
	for (p_tty = TTY_FIRST; p_tty < TTY_END ; p_tty++) {
		init_tty(p_tty);
	}
	// 初始化控制台为第一个控制台 
	nr_current_console = 0;
	// 循环读字符进tty的缓冲区以及显示缓冲区的字符（可显示的字符）
	while(1){
		for(p_tty = TTY_FIRST;p_tty < TTY_END;p_tty++){
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
				init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty,u32 key)
{
       // char output[2] = {'\0', '\0'};
		// 如果是可打印字符进入这里,并放入tty的输入缓冲区
        if (!(key & FLAG_EXT)){
			if(p_tty->inbuf_count < TTY_IN_BYTES){
				*(p_tty->p_inbuf_head) = key;
				p_tty->p_inbuf_head++;
				if(p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES){
					p_tty->p_inbuf_head = p_tty->in_buf;
				}
				p_tty->inbuf_count++;
			}
		// 不可打印字符，例如一些功能键
		}else{
		    int raw_code = key & MASK_RAW;
		    switch(raw_code) {
				case UP:
					if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
							disable_int();
							out_byte(CRTC_ADDR_REG, START_ADDR_H);
							out_byte(CRTC_DATA_REG, ((80*15) >> 8) & 0xFF);
							out_byte(CRTC_ADDR_REG, START_ADDR_L);
							out_byte(CRTC_DATA_REG, (80*15) & 0xFF);
							enable_int();
					}
					break;
				case DOWN:
					if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				/* Shift+Down, do nothing */
					}
					break;
				default:
					break;
			}
		}			
}

/*======================================================================*
			      tty_do_read
1.判断是否是当前tty对应的console
2.读取键盘缓冲
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		// 这里有一个隐式转换，4字节转向1字节，因为可打印字符都是1个字节的
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}
