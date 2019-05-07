; 编译链接方法
; $ nasm -f elf kernel.asm -o kernel.o
; $ ld -s kernel.o -o kernel.bin    #‘-s’选项意为“strip all”
; 在这个试验中这段程序没有任何作用，仅仅是为了证明kernel.bin加载进了内存，因为gs等于显存地址的段选择子，实在gdt表中的一个描述符，然而我们这里还未引入
[section .text]	; 代码在此

global _start	; 导出 _start

_start:	; 跳到这里来的时候，我们假设 gs 指向显存
	mov	ah, 0Fh				; 0000: 黑底    1111: 白字
	mov	al, 'K'
	mov	[gs:((80 * 1 + 39) * 2)], ax	; 屏幕第 1 行, 第 39 列
	jmp	$
