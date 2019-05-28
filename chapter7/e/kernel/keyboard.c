
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            keyboard.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"
#include "keymap.h"

PRIVATE	KB_INPUT		   kb_in;

PRIVATE	int	code_with_E0 = 0;
PRIVATE	int	shift_l;	/* l shift state */
PRIVATE	int	shift_r;	/* r shift state */
PRIVATE	int	alt_l;		/* l alt state	 */
PRIVATE	int	alt_r;		/* r left state	 */
PRIVATE	int	ctrl_l;		/* l ctrl state	 */
PRIVATE	int	ctrl_r;		/* l ctrl state	 */
PRIVATE	int	caps_lock;	/* Caps Lock	 */
PRIVATE	int	num_lock;	/* Num Lock	 */
PRIVATE	int	scroll_lock;	/* Scroll Lock	 */
PRIVATE	int	column;

/*======================================================================*
                            keyboard_handler
 *======================================================================*/
PUBLIC void keyboard_handler(int irq)
{
	// 读取缓冲区内容赋值给scan_code,一个扫描码1个字节
	u8 scan_code = in_byte(KB_DATA);

	if(kb_in.count < KB_IN_BYTES){
		*(kb_in.p_head) = scan_code;
		kb_in.p_head++;
		if(kb_in.p_head == kb_in.buf + KB_IN_BYTES){
			kb_in.p_head = kb_in.buf;
		}
		kb_in.count++;
	}
}


/*======================================================================*
                           init_keyboard
*======================================================================*/
PUBLIC void init_keyboard()
{
	// 初始化kb_in中的成员
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;
	// 初始化这6个按键开始为false
	shift_l	= shift_r = 0;
	alt_l	= alt_r   = 0;
	ctrl_l	= ctrl_r  = 0;

    put_irq_handler(KEYBOARD_IRQ, keyboard_handler);/*设定键盘中断处理程序*/
    enable_irq(KEYBOARD_IRQ);                       /*开键盘中断*/
}


/*======================================================================*
                           keyboard_read
*======================================================================*/
PUBLIC void keyboard_read()
{
	u8	scan_code;
	char	output[2];
	int	make;	/* TRUE: make;  FALSE: break. */
	u32	key = 0;
	u32* keyrow;

	if(kb_in.count > 0){
		disable_int();
		scan_code = *(kb_in.p_tail);
		kb_in.p_tail++;
		if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
			kb_in.p_tail = kb_in.buf;
		}
		kb_in.count--;
		enable_int();

		/* 下面开始解析扫描码 */
		if (scan_code == 0xE1) {
			/* 暂时不做任何操作 */

		}else if (scan_code == 0xE0) {
			code_with_E0 = 1;

		}else {	
			/* 首先判断Make Code 还是 Break Code ,0代表breakcode（松开），1代表makecode（按下）*/
			make = (scan_code & FLAG_BREAK ? 0 : 1);
			/* 定位到行，也就是scan_code对应的在keymap数组中的元素的地址 */
			keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
			/* 初始化为0，也就是行中的第一列元素 */
			column = 0;
			/* 判断是否按了左或右shift,按了的话，那么就取定位的行的第2列 */
			if(shift_l || shift_r){
				column = 1;
			}
			/* 有的扫描码前面多了一个0xE0,这里就是判断是否含有0xE0,有的话那么就取行的第3列 */
			if(code_with_E0){
				column = 2;
				code_with_E0 = 0;
			}
			/* 取行中的第column+1列元素赋予key（4字节,但是一般字符都是1个字节，在后面输出符号的时候隐式转换了）*/
			key = keyrow[column];
			/* 进行判断是否按了alt，shift，ctrl */
			switch(key) {
			case SHIFT_L:
				shift_l = make;
				key = 0;
				break;
			case SHIFT_R:
				shift_r = make;
				key = 0;
				break;
			case CTRL_L:
				ctrl_l = make;
				key = 0;
				break;
			case CTRL_R:
				ctrl_r = make;
				key = 0;
				break;
			case ALT_L:
				alt_l = make;
				key = 0;
				break;
			case ALT_R:
				alt_l = make;
				key = 0;
				break;
			default:
				if (!make) {	/* 如果是 Break Code */
					key = 0;	/* 忽略之 */
				}
				break;
			}
			/* 输出这个符号 */
			if(key){
				output[0] = key;
				disp_str(output);
			}
		}
	}
}
