; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


%include "sconst.inc"

INT_VECTOR_SYS_CALL	equ	0x90
_NR_printx			equ	0
_NR_sendrec			equ 1

; 导出符号
global	printx
global	sendrec

bits 32
[section .text]

; ====================================================================
;           sendrec(int function,int src_dest,MESSAGE *msg);
; ====================================================================
; Never call sendrec() directly, call send_recv() instead.
sendrec:
	mov eax,_NR_sendrec
	mov ebx,[esp + 4]		;function
	mov ecx,[esp + 8]		;src_dest
	mov edx,[esp + 12]		;p_msg
	int INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                       void printx(char* s);
; ====================================================================
printx:
	mov eax,_NR_printx	   ; 功能号
	mov edx,[esp + 4]	   ; 参数s
	int INT_VECTOR_SYS_CALL	;发生软中断（指令中断）的时候出现特权级向内的切换，所以会保存ss，esp，efl ags，cs，eip，错误码（如果有的话）进栈，这个栈的ss和esp是在TSS中定义的也就是ring0的ss和esp（这个栈顶指针很特殊，他是指向每一个进程对应进程表中的P_STACKTOP处），这样刚好就可以再发生系统调用的这个进程的进程表中存储这个进程的上下文
 	ret
