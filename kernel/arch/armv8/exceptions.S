/*
 * Copyright (c) 2015,2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstrasse 6, CH-8092 Zurich. Attn: Systems Group.
 */

#include <asmoffsets.h>
#include <offsets.h>
#include <exceptions.h>

.global do_resume
.global vectors

.macro invalid_exception vector
    /* Just stick the trap frame on the kernel stack - we're about to panic
     * anyway.  */

    add sp, sp, #-(34 * 8)

    /* Spill the GPRs */
    stp  x0,  x1, [sp]
    stp  x2,  x3, [sp, #( 2 * 8)]
    stp  x4,  x5, [sp, #( 4 * 8)]
    stp  x6,  x7, [sp, #( 6 * 8)]
    stp  x8,  x9, [sp, #( 8 * 8)]
    stp x10, x11, [sp, #(10 * 8)]
    stp x12, x13, [sp, #(12 * 8)]
    stp x14, x15, [sp, #(14 * 8)]
    stp x16, x17, [sp, #(16 * 8)]
    stp x18, x19, [sp, #(18 * 8)]
    stp x20, x21, [sp, #(20 * 8)]
    stp x22, x23, [sp, #(22 * 8)]
    stp x24, x25, [sp, #(24 * 8)]
    stp x26, x27, [sp, #(26 * 8)]
    stp x28, x29, [sp, #(28 * 8)]

    /* Stack pointer */
    mrs x0, sp_el0
    stp x30, x0, [sp, #(30 * 8)]

    mrs x0, elr_el1
    mrs x1, spsr_el1
    stp x0, x1, [sp, #(32 * 8)]

    /* Exception Syndrome Register */
    mrs x2, esr_el1

    /* Exception vector */
    mov x3, \vector

    /* Base of the register save area. */
    mov x4, sp

    /* Arguments: x0 = EPC, x1 = SPSR, x2 = ESR, x3 = vector, x4 = save area. */
    b fatal_kernel_fault
.endm

/**********************************/
/*** Start of exception vectors ***/
/**********************************/

/* The AArch64 exception vectors occupy 2kiB */
.align 11
vectors:

/* Offset 0x000 */
/* Exceptions from the current EL, on the EL0 stack.  We never do this. */
/* Each table entry occupies 128B, which lets us put up to 32 instructions
 * here before we branch. */
.align 7 /* 0x000 */
el1_sp_el0_sync:
    invalid_exception AARCH64_EVECTOR_EL0_SYNC
.align 7 /* 0x080 */
el1_sp_el0_irq:
    invalid_exception AARCH64_EVECTOR_EL0_IRQ 
.align 7 /* 0x100 */
el1_sp_el0_fiq:
    invalid_exception AARCH64_EVECTOR_EL0_FIQ
.align 7 /* 0x180 */
el1_sp_el0_serror:
    invalid_exception AARCH64_EVECTOR_EL0_SERROR

/* Offset 0x200 */
/* Exceptions from the kernel itself, at EL1. */
.align 7 /* 0x200 */
el1_sync:
    invalid_exception AARCH64_EVECTOR_EL1_SYNC
.align 7 /* 0x280 */
el1_irq:
    invalid_exception AARCH64_EVECTOR_EL1_IRQ
.align 7 /* 0x300 */
el1_fiq:
    invalid_exception AARCH64_EVECTOR_EL1_FIQ
.align 7 /* 0x380 */
el1_serror:
    invalid_exception AARCH64_EVECTOR_EL1_SERROR

/* Offset 0x400 */
/* Exceptions from user level, EL0, executing AArch64.  For any of these four
 * exceptions, the stack pointer is SP_EL1, which is left at the top of
 * 'kernel_stack'. */
 .align 7 /* 0x400 */
/*
 * Synchronous exceptions from a lower execution level using AArch64: SVC
 * (syscall), data abort, prefetch abort and undefined instruction.
 */
el0_aarch64_sync:
    invalid_exception AARCH32_EVECTOR_EL0_SYNC
.align 7 /* 0x480 */
/* An interrupt at user level */
el0_aarch64_irq:
    invalid_exception AARCH32_EVECTOR_EL0_IRQ

.align 7 /* 0x500 */
/* We don't implement fast IRQs */
el0_aarch64_fiq:
    invalid_exception AARCH64_EVECTOR_EL0_FIQ

.align 7 /* 0x580 */
/* A delayed abort.  We don't handle this. */
el0_aarch64_serror:
    invalid_exception AARCH64_EVECTOR_EL0_SERROR

/* Offset 0x600 */
/* Exceptions from user level, EL0, executing AArch32.  This is currently
 * unimplemented. */
.align 7 /* 0x600 */
el0_aarch32_sync:
    invalid_exception AARCH32_EVECTOR_EL0_SYNC
.align 7 /* 0x680 */
el0_aarch32_irq:
    invalid_exception AARCH32_EVECTOR_EL0_IRQ
.align 7 /* 0x700 */
el0_aarch32_fiq:
    invalid_exception AARCH32_EVECTOR_EL0_FIQ
.align 7 /* 0x780 */
el0_aarch32_serror:
    invalid_exception AARCH32_EVECTOR_EL0_SERROR

.align 11
