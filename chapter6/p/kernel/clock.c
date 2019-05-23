/* 进程切换模块（进程调度）
 * clock.c
 * chinbor
 * 19.5.18
*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	ticks++;
	p_proc_ready->ticks--;
	// 当发生中断重入的时候该函数会再返回前打印一个！
	if(k_reenter != 0){
		//disp_str("!");
		return;
	}
	schedule();
}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();
		//当get_ticks()=100的时候跳出循环，函数结束
        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}
