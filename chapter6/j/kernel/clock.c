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
	disp_str("#");
	// 当发生中断重入的时候该函数会再返回前打印一个！
	if(k_reenter != 0){
		disp_str("!");
		return;
	}
	// 指向下一个进程表
	p_proc_ready++;
	// 进行判断是否超过了进程表数组的界限，若超过就指向a进程的进程表（依次往复，就达到了a，b进程切换的目的）
	if(p_proc_ready >= proc_table + NR_TASKS){
		p_proc_ready = proc_table;
	}
}
