/* 
 * vsprintf.c
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
                                vsprintf
 *======================================================================*/
int vsprintf(char *buf, const char *fmt, va_list args)
{
	char	*p,*q,r;
	char	tmp[256];
	va_list	p_next_arg = args;

	for (p=buf;*fmt;fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt;
			continue;
		}

		fmt++;

		switch (*fmt) {
		case 'x':
			// 注意必须tmp字符串含有'\0'，因为再strcpy和strlen中用它进行的判断
			itoa(tmp, *((int*)p_next_arg));
			strcpy(p, tmp);
			p_next_arg += 4;
			p += strlen(tmp);
			break;
		case 's':
			q = (char*)(*((int*)p_next_arg));
			memcpy(tmp,q,strlen(q)+1);
			strcpy(p,tmp);
			p_next_arg += 4;
			p += strlen(tmp);
			break;
		case 'c':
			q = p_next_arg;
			// 注意这里因为进入了保护模式，所以进栈的参数都是4字节，但是一个字符只要1字节，所以其余位为0
			memcpy(tmp,q,strlen(q)+1);
			strcpy(p,tmp);
			p_next_arg += 4;
			p += strlen(tmp);
			break;
		default:
			break;
		}
	}

	return (p - buf);
}
