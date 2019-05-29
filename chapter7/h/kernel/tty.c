
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "keyboard.h"

PRIVATE	int countUp = 1;
PRIVATE	int PrityControl = 0;

//PRIVATE	int countDown = -1;
/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	while (1) {
	//	disp_str("A.");
		keyboard_read();
	//	milli_delay(10);
	}
}


/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(u32 key)
{
        char output[2] = {'\0', '\0'};
        if (!(key & FLAG_EXT)) {
                output[0] = key & 0xFF;
				if(PrityControl % 2 == 0){
					// 字符输出属性为 0000 1100，也就是背景色为蓝色，前景色为亮白
					disp_color_str(output,MAKE_COLOR(BRIGHT | 0x10,WHITE));
					disable_int();
					// 设置光标跟随字符
					out_byte(CRTC_ADDR_REG, CURSOR_H);
					out_byte(CRTC_DATA_REG, ((disp_pos/2)>>8)&0xFF);
					out_byte(CRTC_ADDR_REG, CURSOR_L);
					out_byte(CRTC_DATA_REG, (disp_pos/2)&0xFF);
					PrityControl++;
					enable_int();
				}else{
					// 字符输出属性为 0001 1010，也就是背景色为蓝色，前景色为亮白
					disp_color_str(output,MAKE_COLOR(BRIGHT | 0x10,WHITE));
					disable_int();
					// 设置光标跟随字符
					out_byte(CRTC_ADDR_REG, CURSOR_H);
					out_byte(CRTC_DATA_REG, ((disp_pos/2)>>8)&0xFF);
					out_byte(CRTC_ADDR_REG, CURSOR_L);
					out_byte(CRTC_DATA_REG, (disp_pos/2)&0xFF);
					PrityControl++;
					enable_int();
				}
        }else{
                int raw_code = key & MASK_RAW;
                switch(raw_code) {
                case UP:
                        if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                                disable_int();
                                out_byte(CRTC_ADDR_REG, START_ADDR_H);
                                out_byte(CRTC_DATA_REG, ((80*countUp) >> 8) & 0xFF);
                                out_byte(CRTC_ADDR_REG, START_ADDR_L);
                                out_byte(CRTC_DATA_REG, (80*countUp) & 0xFF);
								countUp++;
                                enable_int();
                        }
                        break;
                case DOWN:
                        if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
                            //    disable_int();
                            //    out_byte(CRTC_ADDR_REG, START_ADDR_H);
                            //    out_byte(CRTC_DATA_REG, ((80*countDown) >> 8) & 0xFF);
                            //    out_byte(CRTC_ADDR_REG, START_ADDR_L);
                            //    out_byte(CRTC_DATA_REG, (80*countDown) & 0xFF);
							//	  countDown--;
                            //    enable_int();
                        }
                        break;
                default:
                        break;
                }
		}
}
