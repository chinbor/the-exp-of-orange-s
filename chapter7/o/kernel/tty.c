
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
PRIVATE	void put_key(TTY* p_tty,u32 key);

/*======================================================================*
                           task_tty(restart 后执行到了这个进程中)
1.初始化键盘中断程序
2.初始化TTY
3.初始化控制台为第一个
4.循环read和write每一个tty
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
	select_console(0);
	// 循环读字符进tty的缓冲区以及显示缓冲区的字符（可显示的字符,不可显示的字符就不会读入缓冲区以及显示，而是触发相应功能）
	while(1){
		for(p_tty = TTY_FIRST;p_tty < TTY_END;p_tty++){
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
				init_tty
1.初始化tty的相关成员
2.初始化tty对应console的成员
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
1.若是可显示字符，则存入tty输入缓冲区
2.对于不可显示字符，那么设置相应的功能
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty,u32 key)
{
		// 如果是可打印字符进入这里,并放入tty的输入缓冲区
        if (!(key & FLAG_EXT)){
			put_key(p_tty,key);
		// 不可打印字符，例如一些功能键
		}else{
		    int raw_code = key & MASK_RAW;
		    switch(raw_code) {
				case ENTER:
					put_key(p_tty,'\n');
					break;
				case BACKSPACE:
					put_key(p_tty,'\b');
					break;
				case TAB:
					put_key(p_tty,'\t');
					break;
				case UP:
					if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
						scroll_screen(p_tty->p_console,SCR_UP);
					}
					break;
				case DOWN:
					if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
						scroll_screen(p_tty->p_console,SCR_DN);
					}
					break;
				case F1:
				case F2:
				case F3:
				case F4:
				case F5:
				case F6:
				case F7:
				case F8:
				case F9:
				case F10:
				case F11:
				case F12:
					/* ctrl + F1~F12 */
					if ((key & FLAG_CTRL_L) || (key & FLAG_CTRL_R)) {
						select_console(raw_code - F1);
					}
					break;
				default:
					break;
			}
		}			
}

/*======================================================================*
			      put_key
1.将字符存入tty的输入缓冲区				  
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
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
1.判断是否含有可显示字符
2.显示tty输入缓冲区的字符
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	// 只有当tty的输入缓冲区含有字符的时候才打印到屏幕显示,一些功能键就不需要所以不进入
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

/*======================================================================*
                              tty_write
*======================================================================*/
PUBLIC void tty_write(TTY* p_tty, char* buf, int len)
{
        char* p = buf;
        int i = len;

        while (i) {
                out_char(p_tty->p_console, *p++);
                i--;
        }
}

/*======================================================================*
                              sys_write
*======================================================================*/
PUBLIC int sys_write(char* buf, int len, PROCESS* p_proc)
{
        tty_write(&tty_table[p_proc->nr_tty], buf, len);
        return 0;
}

