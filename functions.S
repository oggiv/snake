#	functions.S (formerly labwork.S)
#	Written 2015-2017 by F Lundevall
#	The macros PUSH and POP are in the Public Domain
#	Modified by Viggo Hermansson 2022-02-30

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