; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


%include "sconst.inc"

_NR_get_ticks		equ	0
INT_VECTOR_SYS_CALL	equ	0x90
_NR_write			equ 1

; 导出符号
global	get_ticks
global	write

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov eax,_NR_get_ticks
	int INT_VECTOR_SYS_CALL
	ret

; ====================================================================
;                       void write(char* buf,int len);
; ====================================================================
write:
	mov eax,_NR_write	   ; 功能号
	mov ebx,[esp + 4]	   ; 参数buf
	mov ecx,[esp + 8]      ; 参数len
	int INT_VECTOR_SYS_CALL
	ret
