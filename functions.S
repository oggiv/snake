#	functions.S
#	Contains MIPS assembly code
#	The macros PUSH and POP are in the Public Domain

# PUSH and POP macros
.macro  PUSH reg
	addi  $sp,$sp,-4
	sw  \reg,0($sp)
.endm

.macro  POP reg
	lw  \reg,0($sp)
	addi  $sp,$sp,4
.endm

.global enable_interrupt

enable_interrupt:
	ei
	jr  $ra
	nop