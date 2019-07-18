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
PUBLIC void	port_read(u16 port, void* buf, int n);
PUBLIC void	port_write(u16 port, void* buf, int n);
PUBLIC void	glitter(int row, int col);

/* protect.c中定义 */
PUBLIC void	init_prot();
PUBLIC u32  seg2phys(u16 seg);

/* i8259.c中定义 */
PUBLIC void	init_8259A();
PUBLIC void put_irq_handler(int irq,irq_handler handler);
PUBLIC void spurious_irq(int irq);

/* klib.c中定义 */
PUBLIC void disp_int(int input);
PUBLIC void delay(int time);
PUBLIC char* itoa(char* str,int num);

/* kernel.asm */
PUBLIC	void restart();
PUBLIC	void sys_call();

/* main.c */
PUBLIC	void TestA();
PUBLIC	void TestB();
PUBLIC	void TestC();
PUBLIC int get_ticks();
PUBLIC void panic(const char *fmt, ...);

/* clock.c */
PUBLIC	void clock_handler(int irq);
PUBLIC	void milli_delay(int milli_sec);
PUBLIC	void init_clock();

/* syscall.asm */
/* 系统调用 - 用户级 */
PUBLIC	int	sendrec(int function, int src_dest, MESSAGE* p_msg);
PUBLIC	int	printx(char* str);

/* proc.c */
PUBLIC	void schedule();
PUBLIC int sys_sendrec(int function, int src_dest, MESSAGE* m, struct proc* p);
PUBLIC int send_recv(int function, int src_dest, MESSAGE* msg);
PUBLIC int ldt_seg_linear(struct proc* p, int idx);
PUBLIC void* va2la(int pid, void* va);
PUBLIC void reset_msg(MESSAGE* p);
PUBLIC void dump_proc(struct proc* p);
PUBLIC void dump_msg(const char * title, MESSAGE* m);
PUBLIC void inform_int(int task_nr);



/* keyboard.c */
PUBLIC	void keyboard_handler(int irq);
PUBLIC	void init_keyboard();
PUBLIC	void keyboard_read(TTY* p_tty);

/* tty.c */
PUBLIC	void task_tty();
PUBLIC	void in_process(TTY* p_tty,u32 key);
PUBLIC	void tty_write(TTY*	p_tty,char* buf,int len);
PUBLIC  int sys_printx(int _unused1, int _unused2, char* s, struct proc* p_proc);

/* systask.c */
PUBLIC void task_sys();

/* console.c */
PUBLIC	int is_current_console(CONSOLE* p_con);
PUBLIC	void out_char(CONSOLE* p_con,char ch);
PUBLIC	void init_screen(TTY* p_tty);
PUBLIC	void select_console(int nr_console);
PUBLIC	void scroll_screen(CONSOLE*	p_con, int direction);

/* printf.c */
PUBLIC int printf(const char *fmt, ...);
#define printl printf

/* vsprintf.c */
PUBLIC int vsprintf(char *buf, const char *fmt, va_list args);
PUBLIC int sprintf(char *buf, const char *fmt, ...);


/* lib/misc.c */
PUBLIC void spin(char * func_name);
PUBLIC void assertion_failure(char *exp, char *file, char *base_file, int line);

/* kernel/hd.c */
PUBLIC void	task_hd();
PUBLIC void	hd_handler(int irq);

/* fs/main.c */
PUBLIC void task_fs();


