;; lib.inc

;; 显示 AL 中的数字
DispAL:
	push	ecx
	push	edx
	push	edi

	mov	edi, [dwDispPos]

	mov	ah, 0Fh			; 0000b: 黑底    1111b: 白字
	mov	dl, al
	shr	al, 4
	mov	ecx, 2
.begin:
	and	al, 01111b
	cmp	al, 9
	ja	.1        ;大于则转移
	add	al, '0'
	jmp	.2
.1:
	sub	al, 0Ah
	add	al, 'A'
.2:
	mov	[gs:edi], ax
	add	edi, 2

	mov	al, dl
	loop	.begin
	;add	edi, 2

	mov	[dwDispPos], edi

	pop	edi
	pop	edx
	pop	ecx

	ret
;; DispAL 结束


;; 显示一个整型数
DispInt:
	mov	eax, [esp + 4]   
	shr	eax, 24     ;取这个32位数字的最高8位
	call	DispAL

	mov	eax, [esp + 4]  
	shr	eax, 16     ;取这个32位数字的第二高8位
	call	DispAL

	mov	eax, [esp + 4]
	shr	eax, 8      
	call	DispAL

	mov	eax, [esp + 4]
	call	DispAL

	mov	ah, 07h			; 0000b: 黑底    0111b: 灰字
	mov	al, 'h'         ;这里是为了加一个h表示是16进制数
	push	edi
	mov	edi, [dwDispPos]
	mov	[gs:edi], ax
	add	edi, 4          ;每个项值之间留一个空格
	mov	[dwDispPos], edi;接着屏幕位置显示
	pop	edi

	ret
;; DispInt 结束

;; 显示一个字符串
DispStr:
	push	ebp
	mov	ebp, esp
	push	ebx
	push	esi
	push	edi

	mov	esi, [ebp + 8]	 ;这里是为了得到保护模式下的那个字符串在data1段中的偏移
	mov	edi, [dwDispPos] ;这里是为了得到保护模式下这个变量的值也就是在屏幕显示的位置
	mov	ah, 0Fh          ;可能跟0ch一样，为黑底加一种颜色显示
.1:
	lodsb                ;[ds:esi]所指示的内容一个字节一个字节的加载到al中
	test	al, al       ;测试是否为0，为0则结束
	jz	.2    
	cmp	al, 0Ah	         ; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160          ;每一行80字符，一个字符2字节所以每一行160字节
	div	bl               ;得到行数，ax作为被除数，bl作为除数，结果的商存在al中，余数存在ah
	and	eax, 0FFh        ;只用取得最低8位，也就是al中内容，也就是行数
	inc	eax              ;加1得到下一行
	mov	bl, 160 
	mul	bl               ;为了得到起始行的位置
	mov	edi, eax  
	pop	eax
	jmp	.1   
.3:
	mov	[gs:edi], ax     ;在屏幕的这个位置显示
	add	edi, 2
	jmp	.1

.2:
	mov	[dwDispPos], edi ;更新这个地方的值，也就是在屏幕中的位置,方便后面的信息在接着显示

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	ret
;; DispStr 结束

;; 换行
DispReturn:
	push	szReturn        ;szReturn的值0AH，0
	call	DispStr			;换行
	add	esp, 4

	ret
;; DispReturn 结束


