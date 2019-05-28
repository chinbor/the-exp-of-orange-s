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
		return;
	}
	// 加入这个判断语句是为了让一个进程的ticks还没有到0的时候一直调度它,这样能够更加直观的看到执行次数比
	if(p_proc_ready->ticks > 0){
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


/*======================================================================*
                           init_clock
 *======================================================================*/
PUBLIC void init_clock()
{
	// 初始化8253 PIT
	out_byte(TIMER_MODE,RATE_GENERATOR);				/* 首先需要写8253模式控制寄存器 */
	out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );			/* 结果是一个浮点数，先写向低字节 */
	out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));		/* 再写向高字节 */

    put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
    enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */
}
