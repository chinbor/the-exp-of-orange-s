/* 函数的声明 
 * proto.h
 * chinbor
 * 19.5.13
*/

/* kliba.asm中定义 */
PUBLIC void	out_byte(u16 port, u8 value);
PUBLIC u8	in_byte(u16 port);
PUBLIC void	disp_str(char * info);
PUBLIC void	disp_color_str(char * info, int color);
PUBLIC void enable_irq(int irq);
PUBLIC void disable_irq(int irq);

/* protect.c中定义 */
PUBLIC void	init_prot();
PUBLIC u32  seg2phys(u16 seg);

/* i8259.c中定义 */
PUBLIC void	init_8259A();
PUBLIC void put_irq_handler(int irq,irq_handler handler);

/* klib.c中定义 */
PUBLIC void disp_int(int input);
PUBLIC void delay(int time);

/* kernel.asm */
PUBLIC	void restart();
PUBLIC	void sys_call();

/* main.c */
PUBLIC	void TestA();
PUBLIC	void TestB();
PUBLIC	void TestC();

/* clock.c */
PUBLIC	void clock_handler(int irq);
PUBLIC	void milli_delay(int milli_sec);

/* syscall.asm */
PUBLIC	int get_ticks();		//返回值存放在eax寄存器中

/* proc.c */
PUBLIC	int sys_get_ticks();


