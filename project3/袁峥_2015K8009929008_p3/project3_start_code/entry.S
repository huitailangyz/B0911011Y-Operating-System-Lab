#include "asm.h"
#include "regs.h"
#include "cp0regdefs.h"

  /* Hardcoded values. Do not change these!! */
  /* Offset of the kernel context in a PCB */
  .equ  KERNEL, 8
  /* Offset of the user context in a PCB */
  .equ  USER, 164
  /* Offset of %esp in a context */
  .equ  STACK, TF_REG29
  /* Offset of field nested_count in a PCB */
  .equ  NESTED_COUNT, 320

.macro PUTCHAR ch label
	  addi  k0,$0,10000
	\label:
	addi  k0,k0,-1
	bne k0,$0,\label

	addi  k0,$0,\ch
	la    k1,0xbfe48000
	sb    k0,0(k1)
.endm



.macro STI
  mfc0    k0, CP0_STATUS
  li      k1, (STATUS_CU0 | 0x1)
  or      k0, k1
  mtc0    k0, CP0_STATUS
.endm

.macro CLI
  mfc0    k0, CP0_STATUS
  li      k1, (STATUS_CU0 | 0x1)
  or      k0, k1
  xori    k0, 0x1
  mtc0    k0, CP0_STATUS
.endm

/* Do not change any of these macros! */
/* Save registers/flags to the specified offset in the current PCB */
.macro SAVE_CONTEXT offset
/* TODO: need add */
	la k0,current_running
	lw k0,(k0)
	sw $0,TF_REG0+\offset(k0)
	sw $1,TF_REG1+\offset(k0)
	sw $2,TF_REG2+\offset(k0)
	sw $3,TF_REG3+\offset(k0)
	sw $4,TF_REG4+\offset(k0)
	sw $5,TF_REG5+\offset(k0)
	sw $6,TF_REG6+\offset(k0)
	sw $7,TF_REG7+\offset(k0)
	sw $8,TF_REG8+\offset(k0)
	sw $9,TF_REG9+\offset(k0)
	sw $10,TF_REG10+\offset(k0)
	sw $11,TF_REG11+\offset(k0)
	sw $12,TF_REG12+\offset(k0)
	sw $13,TF_REG13+\offset(k0)
	sw $14,TF_REG14+\offset(k0)
	sw $15,TF_REG15+\offset(k0)
	sw $16,TF_REG16+\offset(k0)
	sw $17,TF_REG17+\offset(k0)
	sw $18,TF_REG18+\offset(k0)
	sw $19,TF_REG19+\offset(k0)
	sw $20,TF_REG20+\offset(k0)
	sw $21,TF_REG21+\offset(k0)
	sw $22,TF_REG22+\offset(k0)
	sw $23,TF_REG23+\offset(k0)
	sw $24,TF_REG24+\offset(k0)
	sw $25,TF_REG25+\offset(k0)
//	sw $26,TF_REG26+\offset(k0)
//	sw $27,TF_REG27+\offset(k0)
	sw $28,TF_REG28+\offset(k0)
	sw $29,TF_REG29+\offset(k0)
	sw $30,TF_REG30+\offset(k0)
	sw $31,TF_REG31+\offset(k0)
	mfc0 k1,$12
	nop
	sw k1,TF_STATUS+\offset(k0)
	mfhi k1
    nop
	sw k1,TF_HI+\offset(k0)
	mflo k1
	nop
	sw k1,TF_LO+\offset(k0)
	mfc0 k1,$8
    nop
	sw k1,TF_BADVADDR+\offset(k0)
	mfc0 k1,$13
	nop
	sw k1,TF_CAUSE+\offset(k0)
	mfc0 k1,$14
	nop
	sw k1,TF_EPC+\offset(k0)
	
/* TODO: end */
.endm

/* Restore registers/flags from the specified offset in the current PCB */
.macro RESTORE_CONTEXT offset
/* TODO: need add */
	la k0,current_running
	lw k0,(k0)
	lw $0,TF_REG0+\offset(k0)
	lw $1,TF_REG1+\offset(k0)
	lw $2,TF_REG2+\offset(k0)
	lw $3,TF_REG3+\offset(k0)
	lw $4,TF_REG4+\offset(k0)
	lw $5,TF_REG5+\offset(k0)
	lw $6,TF_REG6+\offset(k0)
	lw $7,TF_REG7+\offset(k0)
	lw $8,TF_REG8+\offset(k0)
	lw $9,TF_REG9+\offset(k0)
	lw $10,TF_REG10+\offset(k0)
	lw $11,TF_REG11+\offset(k0)
	lw $12,TF_REG12+\offset(k0)
	lw $13,TF_REG13+\offset(k0)
	lw $14,TF_REG14+\offset(k0)
	lw $15,TF_REG15+\offset(k0)
	lw $16,TF_REG16+\offset(k0)
	lw $17,TF_REG17+\offset(k0)
	lw $18,TF_REG18+\offset(k0)
	lw $19,TF_REG19+\offset(k0)
	lw $20,TF_REG20+\offset(k0)
	lw $21,TF_REG21+\offset(k0)
	lw $22,TF_REG22+\offset(k0)
	lw $23,TF_REG23+\offset(k0)
	lw $24,TF_REG24+\offset(k0)
	lw $25,TF_REG25+\offset(k0)
//	sw $26,TF_REG26+\offset(k0)
//	sw $27,TF_REG27+\offset(k0)
	lw $28,TF_REG28+\offset(k0)
	lw $29,TF_REG29+\offset(k0)
	lw $30,TF_REG30+\offset(k0)
	lw $31,TF_REG31+\offset(k0)
	lw k1,TF_STATUS+\offset(k0)
	mtc0 k1,$12
	nop
	lw k1,TF_HI+\offset(k0)
	mthi k1
	nop
	lw k1,TF_LO+\offset(k0)
	mtlo k1
	nop
	lw k1,TF_BADVADDR+\offset(k0)
	mtc0 k1,$8
	nop
	lw k1,TF_CAUSE+\offset(k0)
	mtc0 k1,$13
	nop
	lw k1,TF_EPC+\offset(k0)
	mtc0 k1,$14
	nop	
/* TODO: end */
.endm

/* Increment disable_count, panicking on a double ENTER
 * Interrupts should be disabled before this macro is used
 */
.macro ENTER_CRITICAL
  la      k0, disable_count
  lw      k1, 0(k0)

  bne     k1, $0, panice
  nop
  addi    k1, k1, 1
  sw      k1, 0(k0)
.endm

/* Decrement disable_count, panicking on a double LEAVE
 * Interrupts should be enabled after this macro is used
 */
.macro LEAVE_CRITICAL
  la      k0, disable_count
  lw      k1, 0(k0)
  addi    k1, k1, -1
  bne     k1, $0, panicl
  nop
  sw      k1, 0(k0)
.endm

/* TODO: Test current_running->nested_count, clobbering only eflags */
.macro TEST_NESTED_COUNT
  la      k0, current_running
  lw      k0, 0(k0)
  lw      k1,NESTED_COUNT(k0)
  
.endm

/** TODO end */

  .data
// Temporary storage for 32-bit registers
scratch:
  .long    0
// Temporary storage for 32-bit registers
scratch2:
  .long    0
// Temporary storage for syscall return value
return_value:
  .long    0
// Signals status of interrupts
    .globl    disable_count
disable_count:
  .long    0

  .text

NESTED(scheduler_entry,0,ra)
	/* TODO: need add */

  	SAVE_CONTEXT(KERNEL)
  	jal  scheduler 
  	nop
  	RESTORE_CONTEXT(KERNEL)
  	jr   ra
  	nop
	
	/* TODO: end */
END(scheduler_entry)

LEAF(asm_start)
  /* Disable interrupts */
  mtc0    zero, CP0_STATUS

  /* Disable watch exception. */
  mtc0    zero, CP0_WATCHLO
  mtc0    zero, CP0_WATCHHI

  /* disable kernel mode cache */
  mfc0    t0, CP0_CONFIG
  and     t0, ~0x7
  ori     t0, 0x2
  mtc0    t0, CP0_CONFIG

  jr  ra
END(asm_start)

LEAF(reset_timer)
  mtc0 $0, CP0_COUNT
  mtc0 a0, CP0_COMPARE
  jr ra
END(reset_timer)

  .global exception_handler_start
  .global exception_handler_end
NESTED(general_exception_handler,0,sp)
exception_handler_start:
  .set  noat
  .set  mips32

  CLI

  mfc0  k0, CP0_CAUSE
  andi  k0, k0, CAUSE_EXCCODE   /* k0 = {exc_code,00} */
  la    k1, interrupt_handlers
  add   k0, k0, k1
  lw    k0, 0(k0)
  j     k0     /* interrupt_handlers[exc_code](); */
exception_handler_end:
END(general_exception_handler)
  .set at
  .set reorder
  .text

LEAF(return_from_exception)
	STI
	eret
END(return_from_exception)

NESTED(simple_handler,0,sp)
  SAVE_CONTEXT(USER)

  mfc0  k0, CP0_CAUSE           /* Read Cause register for IP bits */
  //mfc0  k1, CP0_STATUS        /* and Status register for IM bits */
  andi  k0, k0, CAUSE_IPL       /* Keep only IP bits from Cause */
  //and   k0, k0, k1            /* and mask with IM bits */
  clz   k0, k0                  /* Find first bit set, IP7..IP0; k0 = 16..23 */
  xori  k0, k0, 0x17            /* 16..23 => 7..0 */
  add   a0, k0, $0

  mfc0  k0, CP0_CAUSE
  andi  k0, k0, CAUSE_EXCCODE   /* k0 = {exc_code,00} */
  srl   k0, k0, 2
  add   a1, k0, $0

  jal   c_simple_handler

  mfc0  k0, CP0_CAUSE           /* Read Cause register for IP bits */
  andi  k1, k0, CAUSE_IPL       /* Keep only IP bits from Cause */
  xor   k0, k0, k1
  mtc0  k0, CP0_CAUSE

  LEAVE_CRITICAL
  RESTORE_CONTEXT(USER)
  j return_from_exception
END(simple_handler)

NESTED(handle_syscall,0,sp)
  SAVE_CONTEXT(USER)
  //RESTORE_CONTEXT(KERNEL)
  add   a3,a2,$0
  add   a2,a1,$0
  add   a1,a0,$0
  add   a0,v0,$0

  jal system_call_helper

  LEAVE_CRITICAL
  RESTORE_CONTEXT(USER)
  j return_from_exception
END(handle_syscall)

NESTED(handle_int,0,sp)
  /* TODO: timer_irq */
  /* read int IP and handle clock interrupt or just call do_nothing */
  SAVE_CONTEXT(USER)


  mfc0  k0, CP0_CAUSE 		  /* Read Cause register for IP bits */
  nop
  andi	k0, k0, CAUSE_IPL		/* Keep only IP bits from Cause */
  nop
  clz	k0, k0					/* Find first bit set, IP7..IP0; k0 = 16..23 */
  xori	k0, k0, 0x17			/* 16..23 => 7..0 */
  li    k1, 7
  beq   k0, k1, timer_irq
  nop
  jal   clr_int

finish:
  RESTORE_CONTEXT(USER)
  j return_from_exception
  nop

timer_irq:
  li    a0, 150000000
  jal   reset_timer
  nop
  jal   clr_int
  la    k0, time_elapsed
  lw    k1, (k0)
  addiu k1, k1, 1000
  sw    k1, (k0)

  TEST_NESTED_COUNT
  bne   k1, zero, finish
  nop

  ENTER_CRITICAL 
  la    k0, current_running
  lw    k0, (k0)
  li    k1, 1
  sw    k1, NESTED_COUNT(k0)

  jal   put_current_running
  nop
  jal   scheduler_entry
  nop

  la    k0, current_running
  lw    k0, (k0)
  sw    zero, NESTED_COUNT(k0)

  LEAVE_CRITICAL
  j     finish
  nop


clr_int:  
  mfc0  k0, CP0_CAUSE 		  /* Read Cause register for IP bits */
  nop
  andi	k1, k0, CAUSE_IPL		/* Keep only IP bits from Cause */
  xor   k0, k0, k1			/* and mask with IM bits */
  mtc0  k0, CP0_CAUSE
  nop
  jr    ra
  
  /* TODO:end */
END(handle_int)


LEAF(get_cp0_status)
  mfc0  v0, CP0_STATUS
  jr ra
END(get_cp0_status)

LEAF(set_cp0_status)
  mtc0  a0, CP0_STATUS
  jr ra
END(set_cp0_status)


LEAF(enter_critical)
  CLI
  ENTER_CRITICAL
  jr ra
END(enter_critical)

LEAF(leave_critical)
  LEAVE_CRITICAL
  STI
  jr ra
END(leave_critical)

LEAF(first_entry)
  LEAVE_CRITICAL 
  j  finish
END(first_entry)

/* Error Message: Write "PANIC" to the screen and enter an infinite loop */
LEAF(panic)
  CLI
  PUTCHAR 0x50 P1// 'P'
  j panic
END(panic)

LEAF(panice)
  CLI
  PUTCHAR 69 P2// 'E'
  j panic
END(panice)

LEAF(panicl)
    CLI
	PUTCHAR 76 P3// 'L'
	j panic
END(panicl)

LEAF(infinite_loop)
//        hlt
  j    infinite_loop
END(infinite_loop)
