/* itoa disp_int函数的定义 
 * klib.c
 * chinbor
 * 19.5.13
*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "string.h"

/*======================================================================*
                               itoa
 *======================================================================*/
/* 数字前面的 0 不被显示出来, 比如 0000B800 被显示成 B800 */
PUBLIC char * itoa(char * str, int num)
{
	char *	p = str;
	char	ch;
	int	i;
	int	flag = 0;

	*p++ = '0';
	*p++ = 'x';

	if(num == 0){
		*p++ = '0';
	}
	else{	
		for(i=28;i>=0;i-=4){
			ch = (num >> i) & 0xF;
			if(flag || (ch > 0)){
				flag = 1;
				ch += '0';
				// 这一句话是为了转换成对应的16进制ABCDEF
				if(ch > '9'){
					ch += 7;
				}
				*p++ = ch;
			}
		}
	}

	*p = 0;

	return str;
}

/*======================================================================*
                               disp_int
 *======================================================================*/
PUBLIC void disp_int(int input)
{
	char output[16];
	// input的值为0x00000001，output的值为0x32910,函数itoa的地址为0x30b85,这个函数的作用就是将input的值去掉前面的0，然后将结果字符放在output地址上
	itoa(output, input);
	// disp_str地址为0x30c60
	disp_str(output);
}


/*======================================================================*
                               delay
 *======================================================================*/
PUBLIC void delay(int time)
{
	int i, j, k;
	for (k = 0; k < time; k++) {
		for (i = 0; i < 10; i++) {
			for (j = 0; j < 10000; j++) {}
		}
	}
}
