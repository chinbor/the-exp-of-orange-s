##################################################
# Makefile
##################################################

BOOT:=boot.asm
LDR:=loader.asm
KERNEL:=kernel.asm
BOOT_BIN:=$(subst .asm,.bin,$(BOOT))
LDR_BIN:=$(subst .asm,.bin,$(LDR))
KERNEL_BIN:=$(subst .asm,.bin,$(KERNEL))

IMG:=a.img
FLOPPY:=/mnt/floppy/

.PHONY : everything

everything : $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN)
	ld -s -Ttext 0x30400 -o kernel.bin kernel.o string.o start.o kliba.o
	dd if=$(BOOT_BIN) of=$(IMG) bs=512 count=1 conv=notrunc
	sudo mount -o loop $(IMG) $(FLOPPY)
	sudo cp $(LDR_BIN) $(FLOPPY) -v
	sudo cp $(KERNEL_BIN) $(FLOPPY) -v
	sudo umount $(FLOPPY)

clean :
	rm -f $(BOOT_BIN) $(LDR_BIN) $(KERNEL_BIN) *.o

$(BOOT_BIN) : $(BOOT)
	nasm $< -o $@

$(LDR_BIN) : $(LDR)
	nasm $< -o $@

$(KERNEL_BIN) : $(KERNEL) start.c string.asm
	nasm -f elf -o $(subst .asm,.o,$(KERNEL)) $<
	nasm -f elf -o string.o string.asm
	nasm -f elf -o kliba.o kliba.asm
#-fno-builtin意思就是不使用c语言的內建函数，防止我们自己的函数名和c语言內建函数名相同而冲突，从而报错
	gcc -c -fno-builtin -o start.o start.c    

