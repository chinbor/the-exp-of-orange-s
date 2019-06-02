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
PUBLIC void disable_int();
PUBLIC void enable_int(); 

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
PUBLIC	void init_clock();

/* syscall.asm */
PUBLIC	int get_ticks();		//返回值存放在eax寄存器中

/* proc.c */
PUBLIC	int sys_get_ticks();
PUBLIC	void schedule();

/* keyboard.c */
PUBLIC	void keyboard_handler(int irq);
PUBLIC	void init_keyboard();
PUBLIC	void keyboard_read(TTY* p_tty);

/* tty.c */
PUBLIC	void task_tty();
PUBLIC	void in_process(TTY* p_tty,u32 key);

/* console.c */
PUBLIC	int is_current_console(CONSOLE* p_con);
PUBLIC	void out_char(CONSOLE* p_con,char ch);
PUBLIC	void init_screen(TTY* p_tty);
PUBLIC	void select_console(int nr_console);
PUBLIC	void scroll_screen(CONSOLE*	p_con, int direction);
