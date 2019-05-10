
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                              klib.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                       Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


[SECTION .data]
disp_pos	dd	0

[SECTION .text]

; 导出函数
global	disp_str

; ========================================================================
;                  void disp_str(char * info);
; ========================================================================
disp_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, 0Fh ; 黑底白字
.1:
	lodsb           ; 将ds:esi的内容存入al寄存器
	test	al, al  ; 测试是否到达字符串末尾
	jz	.2
	cmp	al, 0Ah	; 是回车吗? \n代表的就是这个字符
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160 ; 每一行80列，每一个是2字节。   例如（80*1 + 0)*2代表的就是1行0列
	div	bl      ; 算出行数，余数存在ah中，商存在al中
	and	eax, 0FFh ; 只取商（也就是行数）
	inc	eax       ; 加1（每一行是0-159）
	mov	bl, 160
	mul	bl  	  ; 新的位置 
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

