/* 输出函数
 * printf.c
 * chinbor
 * 19.6.7
 */


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

/*======================================================================*
                                 printf
 *======================================================================*/
// 注意这里的3个点，表示的意思是参数个数和类型可变
int printf(const char *fmt, ...)
{
	int i;
	char buf[256];

	va_list arg = (va_list)((char*)(&fmt) + 4); /*4是参数fmt所占堆栈中的大小*/
	i = vsprintf(buf, fmt, arg);
	write(buf, i);

	return i;
}

