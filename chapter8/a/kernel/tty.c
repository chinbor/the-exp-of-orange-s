
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

	/* assert对于ring0-1则会叫停，而对于ring3不会叫停,会进入spin函数然后无限循环 */
	//assert(0);
	/* panic只会用于ring0-ring1，因为他们会叫停程序 */
	// panic("in TTY");

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
					}else{
						if (raw_code == F12) {
							disable_int();
							dump_proc(proc_table + 4);
							for(;;);
						}
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
                              sys_printx
*======================================================================*/
PUBLIC int sys_printx(int _unused1, int _unused2, char* s, struct proc* p_proc)
{
	const char * p;
	char ch;

	char reenter_err[] = "? k_reenter is incorrect for unknown reason";
	reenter_err[0] = MAG_CH_PANIC;

	/**
	 * @note Code in both Ring 0 and Ring 1~3 may invoke printx().
	 * If this happens in Ring 0, no linear-physical address mapping
	 * is needed.
	 *
	 * @attention The value of `k_reenter' is tricky here. When
	 *   -# printx() is called in Ring 0
	 *      - k_reenter > 0. When code in Ring 0 calls printx(),
	 *        an `interrupt re-enter' will occur (printx() generates
	 *        a software interrupt). Thus `k_reenter' will be increased
	 *        by `kernel.asm::save' and be greater than 0.
	 *   -# printx() is called in Ring 1~3
	 *      - k_reenter == 0.
	 */
	if (k_reenter == 0)  /* printx() called in Ring<1~3> */
		p = va2la(proc2pid(p_proc), s);
	else if (k_reenter > 0) /* printx() called in Ring<0> */
		p = s;
	else	/* this should NOT happen */
		p = reenter_err;

	/**
	 * @note if assertion fails in any TASK, the system will be halted;
	 * if it fails in a USER PROC, it'll return like any normal syscall
	 * does.
	 */
	if ((*p == MAG_CH_PANIC) ||
	    (*p == MAG_CH_ASSERT && p_proc_ready < &proc_table[NR_TASKS])) {
		disable_int();
		char * v = (char*)V_MEM_BASE;
		const char * q = p + 1; /* +1: skip the magic char */

		while (v < (char*)(V_MEM_BASE + V_MEM_SIZE)) {
			*v++ = *q++;
			*v++ = RED_CHAR;
			if (!*q) {
				/* 这里的意思就是遇见最后字符串结尾为0的就换8行 */
				while (((int)v - V_MEM_BASE) % (SCR_WIDTH * 16)) {
					/* *v++ = ' '; */
					v++;
					*v++ = GRAY_CHAR;
				}
				/* 重新再新的换了8行后开始再打印这个字符串 */
				q = p + 1;
			}
		}
		/* 就直接暂停执行了 */
		__asm__ __volatile__("hlt");
	}
	/* user_proc就打印一句话就行了 */
	while ((ch = *p++) != 0) {
		if (ch == MAG_CH_PANIC || ch == MAG_CH_ASSERT)
			continue; /* skip the magic char */

		out_char(tty_table[p_proc->nr_tty].p_console, ch);
	}

	return 0;
}


