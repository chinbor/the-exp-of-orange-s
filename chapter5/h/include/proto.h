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

/* protect.c中定义 */
PUBLIC void	init_prot();

/* i8259.c中定义 */
PUBLIC void	init_8259A();

/* klib.c中定义 */
PUBLIC void disp_int(int input);
