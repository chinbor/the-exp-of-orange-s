; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               sconst.inc
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

P_STACKBASE	equ	0
GSREG		equ	P_STACKBASE				; 0
FSREG		equ	GSREG		+ 4			; 4
ESREG		equ	FSREG		+ 4			; 8
DSREG		equ	ESREG		+ 4			; 12
EDIREG		equ	DSREG		+ 4			; 16
ESIREG		equ	EDIREG		+ 4			; 20
EBPREG		equ	ESIREG		+ 4			; 24
KERNELESPREG	equ	EBPREG		+ 4		; 28
EBXREG		equ	KERNELESPREG	+ 4		; 32
EDXREG		equ	EBXREG		+ 4			; 36
ECXREG		equ	EDXREG		+ 4			; 40
EAXREG		equ	ECXREG		+ 4			; 44
RETADR		equ	EAXREG		+ 4			; 48
EIPREG		equ	RETADR		+ 4			; 52
CSREG		equ	EIPREG		+ 4			; 56
EFLAGSREG	equ	CSREG		+ 4			; 60
ESPREG		equ	EFLAGSREG	+ 4			; 64
SSREG		equ	ESPREG		+ 4			; 68
P_STACKTOP	equ	SSREG		+ 4			; 72
P_LDT_SEL	equ	P_STACKTOP				; 72
P_LDT		equ	P_LDT_SEL	+ 4			; 76

TSS3_S_SP0	equ	4

INT_M_CTL	equ	0x20 ; I/O port for interrupt controller        <Master>
INT_M_CTLMASK	equ	0x21 ; setting bits in this port disables ints  <Master>
INT_S_CTL	equ	0xA0 ; I/O port for second interrupt controller <Slave>
INT_S_CTLMASK	equ	0xA1 ; setting bits in this port disables ints  <Slave>

EOI		equ	0x20

; 以下选择子值必须与 protect.h 中保持一致!!!
SELECTOR_FLAT_C		equ		0x08	; LOADER 里面已经确定了的.
SELECTOR_TSS		equ		0x20	; TSS
SELECTOR_KERNEL_CS	equ		SELECTOR_FLAT_C

