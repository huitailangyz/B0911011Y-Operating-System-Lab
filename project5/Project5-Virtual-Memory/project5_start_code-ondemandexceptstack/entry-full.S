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
  .equ  PID, 328
  .equ  PAGETABLE, 332

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
  .set noat
  add     k1, sp, $0
  la      k0, current_running
  lw      sp, 0(k0)
  addi    sp, sp, \offset
  mfc0    k0, CP0_STATUS
  sll     k0, 3
  bltz    k0, 1f
  nop
  1:
  sw      k1, TF_REG29(sp)
  sw      $2, TF_REG2(sp)
  mfc0    k0, CP0_STATUS
  sw      k0, TF_STATUS(sp)
  mfc0    k0, CP0_CAUSE
  sw      k0, TF_CAUSE(sp)
  mfc0    k0, CP0_EPC
  sw      k0, TF_EPC(sp)
  mfc0    k0, CP0_BADVADDR
  sw      k0, TF_BADVADDR(sp)
  mfhi    k0
  sw      k0, TF_HI(sp)
  mflo    k0
  sw      k0, TF_LO(sp)
  sw      $0, TF_REG0(sp)
  sw      $1, TF_REG1(sp)
  sw      $3, TF_REG3(sp)
  sw      $4, TF_REG4(sp)
  sw      $5, TF_REG5(sp)
  sw      $6, TF_REG6(sp)
  sw      $7, TF_REG7(sp)
  sw      $8, TF_REG8(sp)
  sw      $9, TF_REG9(sp)
  sw      $10, TF_REG10(sp)
  sw      $11, TF_REG11(sp)
  sw      $12, TF_REG12(sp)
  sw      $13, TF_REG13(sp)
  sw      $14, TF_REG14(sp)
  sw      $15, TF_REG15(sp)
  sw      $16, TF_REG16(sp)
  sw      $17, TF_REG17(sp)
  sw      $18, TF_REG18(sp)
  sw      $19, TF_REG19(sp)
  sw      $20, TF_REG20(sp)
  sw      $21, TF_REG21(sp)
  sw      $22, TF_REG22(sp)
  sw      $23, TF_REG23(sp)
  sw      $24, TF_REG24(sp)
  sw      $25, TF_REG25(sp)
  sw      $26, TF_REG26(sp)
  sw      $27, TF_REG27(sp)
  sw      $28, TF_REG28(sp)
  sw      $30, TF_REG30(sp)
  sw      $31, TF_REG31(sp)
  add     sp, k1, $0
  .set at
.endm

/* As above, but save only %esp */
.macro SAVE_STACK offset
  la      k0, current_running
  lw      k0, 0(k0)
  sw      sp, ((\offset)+STACK)(k0)
.endm

/* Restore registers/flags from the specified offset in the current PCB */
.macro RESTORE_CONTEXT offset
  .set    noat
  la      sp,current_running
  lw      sp,0(sp)
  addi    sp,sp,\offset
  mfc0    t0, CP0_STATUS
  ori     t0, 0x3
  xori    t0, 0x3
  mtc0    t0, CP0_STATUS
  lw      v0, TF_STATUS(sp)
  li      v1, 0xff00
  and     t0, v1
  nor     v1, $0, v1
  and     v0, v1
  or      v0, t0
  mtc0    v0, CP0_STATUS
  lw      v1, TF_LO(sp)
  mtlo    v1
  lw      v0, TF_HI(sp)
  lw      v1, TF_EPC(sp)
  mthi    v0
  mtc0    v1, CP0_EPC
  lw      $31, TF_REG31(sp)
  lw      $30, TF_REG30(sp)
  lw      $28, TF_REG28(sp)
  lw      $25, TF_REG25(sp)
  lw      $24, TF_REG24(sp)
  lw      $23, TF_REG23(sp)
  lw      $22, TF_REG22(sp)
  lw      $21, TF_REG21(sp)
  lw      $20, TF_REG20(sp)
  lw      $19, TF_REG19(sp)
  lw      $18, TF_REG18(sp)
  lw      $17, TF_REG17(sp)
  lw      $16, TF_REG16(sp)
  lw      $15, TF_REG15(sp)
  lw      $14, TF_REG14(sp)
  lw      $13, TF_REG13(sp)
  lw      $12, TF_REG12(sp)
  lw      $11, TF_REG11(sp)
  lw      $10, TF_REG10(sp)
  lw      $9, TF_REG9(sp)
  lw      $8, TF_REG8(sp)
  lw      $7, TF_REG7(sp)
  lw      $6, TF_REG6(sp)
  lw      $5, TF_REG5(sp)
  lw      $4, TF_REG4(sp)
  lw      $3, TF_REG3(sp)
  lw      $2, TF_REG2(sp)
  lw      $1, TF_REG1(sp)
  lw      sp, TF_REG29(sp)
  .set    at
.endm

/* As above, but restore only %esp */
.macro RESTORE_STACK offset
  la      k0, current_running
  lw      k0, 0(k0)
  lw      sp, ((\offset)+STACK)(k0)
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
  lw      k0, NESTED_COUNT(k0)
  bne     k0, $0, ret
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
pgd_current:
  .word 0
tlbra:
  .word 0
rastore:
  .word 0

  .text

NESTED(scheduler_entry,0,ra)
  SAVE_CONTEXT(KERNEL)
  jal    scheduler
  LEAVE_CRITICAL
  RESTORE_CONTEXT(KERNEL)
  STI
  jr  ra
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
  jr  k0     /* interrupt_handlers[exc_code](); */
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
  SAVE_CONTEXT(KERNEL)

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
  RESTORE_CONTEXT(KERNEL)
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
  SAVE_CONTEXT(USER)
  mfc0  k0, CP0_CAUSE           /* Read Cause register for IP bits */
  //mfc0  k1, CP0_STATUS        /* and Status register for IM bits */
  andi  k0, k0, CAUSE_IPL       /* Keep only IP bits from Cause */
  //and   k0, k0, k1            /* and mask with IM bits */
  clz   k0, k0                  /* Find first bit set, IP7..IP0; k0 = 16..23 */
  xori  k0, k0, 0x17            /* 16..23 => 7..0 */

  li    k1, 7

//  ENTER_CRITICAL
  
  bne   k0,k1,do_nothing
  jal   timer_irq

do_nothing:
  mfc0  k0, CP0_CAUSE           /* Read Cause register for IP bits */
	andi  k0, k0, CAUSE_IPL       /* Keep only IP bits from Cause */
  add   k1, k0, $0
  clz   k0, k0                  /* Find first bit set, IP7..IP0; k0 = 16..23 */
	xori  k0, k0, 0x17            /* 16..23 => 7..0 */
  addi  k0, k0, 8               /* 7..0 => 15..8 */
  li    t0, 1
  sllv  k0, t0, k0
	xor   k0, k0, k1
	mtc0  k0, CP0_CAUSE
  LEAVE_CRITICAL
  RESTORE_CONTEXT(USER)
	j return_from_exception
END(handle_int)

// --------------------------------- P5 start -----------------------------------
LEAF(tlb_flush)
    mfc0    k1,CP0_ENTRYHI
    mtc0    a0,CP0_ENTRYHI
    nop
    tlbp
    nop
    nop
    nop
    nop
    mfc0    k0,CP0_INDEX
    bltz    k0,NOFOUND
    nop
    mtc0    zero,CP0_PAGEMASK
    mtc0    zero,CP0_ENTRYHI
    mtc0    zero,CP0_ENTRYLO0
    mtc0    zero,CP0_ENTRYLO1
    nop
    tlbwi
NOFOUND:
    mtc0    k1,CP0_ENTRYHI
	nop
    j   ra
    nop
END(tlb_flush)

	
//TO DO
LEAF(set_pt)
	lw   k0, current_running
	lw 	 k1, PID(k0)

	mtc0 k1, CP0_ENTRYHI
	
	lw 	 k1, PAGETABLE(k0)
	sw 	 k1, current_page_table
	jr   ra
	nop
END(set_pt)

//TO DO
	.global handle_tlb_miss_start
	.global handle_tlb_miss_end

NESTED(handle_tlb_miss,0,sp)
handle_tlb_miss_start:


	CLI
//PTE: 20 bit pfn /8 bit pid /4 bit flag: nop D V G

	li 	 t0, 0xffffe000
	li   t1, 0x00000fff
	li   t2, 0xfffff000
	mfc0 k0, CP0_BADVADDR
		
	and  t3, k0, t0
	mfc0 k1, CP0_ENTRYHI
	and  t4, k1, t1
	or   k1, t3, t4
	mtc0 k1, CP0_ENTRYHI
//		add  a0,k1,0
//		li   a1,10
//		jal  temp_print

	lw 	 t4, current_page_table
	srl  t3, t3, 13
	sll  t3, t3, 3
	add  t4, t4, t3
	lw   t5, 0(t4)
	andi t6, t5, 0xf
	and  t7, t5, t2
	srl  t7, t7, 6
	or   t7, t7, t6
	mtc0 t7, CP0_ENTRYLO0


	lw   t5, 4(t4)
	andi t6, t5, 0xf
	and  t7, t5, t2
	srl  t7, t7, 6
	or   t7, t7, t6
	mtc0 t7, CP0_ENTRYLO1
	mtc0    zero,CP0_PAGEMASK
	tlbwr
	PUTCHAR 84 T3// 'T'
	jal do_tlb_miss
	nop
handle_tlb_miss_end:
	j return_from_exception
   
END(handle_tlb_miss)

NESTED(handle_page_fault,0,sp)
//print all TLB 
	mfc0 k0, CP0_ENTRYHI
		add  a0,k0,0
		li	 a1,9
		jal  temp_print
		nop
		mfc0 k0, CP0_ENTRYHI

	li   k1, 31

continueprint:
	mtc0 k1, CP0_INDEX
	tlbr
	add  a0, k1, zero
	mfc0 a1, CP0_ENTRYHI
	mfc0 a2, CP0_ENTRYLO0
	mfc0 a3, CP0_ENTRYLO1
	jal print_tlb
	nop
	addi k1, k1, -1
	bgez k1, continueprint
	nop


	mtc0 k0, CP0_ENTRYHI

	li	 t0, 0xffffe000
		li	 t1, 0x00000fff
		li	 t2, 0xfffff000
		mfc0 k0, CP0_BADVADDR
			
		and  t3, k0, t0
		mfc0 k1, CP0_ENTRYHI
		and  t4, k1, t1
		or	 k1, t3, t4
		mtc0 k1, CP0_ENTRYHI
	//		add  a0,k1,0
	//		li	 a1,10
	//		jal  temp_print
	
		lw	 t4, current_page_table
		srl  t3, t3, 13
		sll  t3, t3, 3
		add  t4, t4, t3
		lw	 t5, 0(t4)
		andi t6, t5, 0xf
		and  t7, t5, t2
		srl  t7, t7, 6
		or	 t7, t7, t6
		mtc0 t7, CP0_ENTRYLO0
	
	
		lw	 t5, 4(t4)
		andi t6, t5, 0xf
		and  t7, t5, t2
		srl  t7, t7, 6
		or	 t7, t7, t6
		mtc0 t7, CP0_ENTRYLO1
		mtc0	zero,CP0_PAGEMASK
		tlbwr
		PUTCHAR 84 T4// 'T'
		jal do_tlb_miss
		nop



//---		
mfc0 k0, CP0_ENTRYHI
			add  a0,k0,0
			li	 a1,10
			jal  temp_print
			nop
			mfc0 k0, CP0_ENTRYHI
	
		li	 k1, 31
	
	continueprint2:
		mtc0 k1, CP0_INDEX
		tlbr
		add  a0, k1, zero
		mfc0 a1, CP0_ENTRYHI
		mfc0 a2, CP0_ENTRYLO0
		mfc0 a3, CP0_ENTRYLO1
		jal print_tlb
		nop
		addi k1, k1, -1
		bgez k1, continueprint2
		nop
	
	
		mtc0 k0, CP0_ENTRYHI

//-------


	j return_from_exception
	nop
END(handle_page_fault)


// --------------------------------- P5 end --------------------------------


ret:
  LEAVE_CRITICAL   // del disable_count
  eret


LEAF(get_cp0_status)
  mfc0  v0, CP0_STATUS
  jr ra
END(get_cp0_status)

LEAF(set_cp0_status)
  mtc0  a0, CP0_STATUS
  jr ra
END(set_cp0_status)

LEAF(get_asid)
  mfc0  v0, CP0_ENTRYHI
  and	v0, 0x7
  jr ra
END(get_asid)

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

/* Error Message: Write "PANIC" to the screen and enter an infinite loop */
LEAF(panic)
  CLI
  PUTCHAR 0x50 P1// 'P'
  j panic
END(panic)
LEAF(panice)
  CLI
  PUTCHAR 69 P2// 'P'
  j panic
END(panice)
	LEAF(panicl)
	  CLI
	  PUTCHAR 76 P3// 'P'
	  j panic
	END(panicl)
//  cli
//  movl    $0xb800a, %esp
//  pushw    $0x4f43
//  pushw    $0x4f49
//  pushw    $0x4f4e
//  pushw    $0x4f41
//  pushw    $0x4f50
LEAF(infinite_loop)
//        hlt
  j    infinite_loop
END(infinite_loop)
