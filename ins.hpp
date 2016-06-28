#ifndef __INSTRS_HPP
#define __INSTRS_HPP

extern instruc_t Instructions[];

enum nameNum ENUM_SIZE(uint16)
{
	RX63_null=0,	// Unknown Operation

	RX63_abs,		// Absolute value
	RX63_abs_,		// Absolute value
	RX63_adc,		// Addition with carry
	RX63_add,		// Addition without carry
	RX63_add_,		// Addition without carry
	RX63_and,		// Logical AND
	RX63_and_,		// Logical AND
	RX63_bclr,		// Clearing a bit 
	RX63_b,			// Relative conditional branch
	RX63_bm,		// Conditional bit transfer
	RX63_bnot,		// Inverting a bit
	RX63_bra,		// Unconditional relative branch
	RX63_brk,		// Unconditional trap
	RX63_bset,		// Setting a bit
	RX63_bsr,		// Relative subroutine branch
	RX63_btst,		// Testing a bit
	RX63_clrpsw,	// Clear a flag or bit in the PSW
	RX63_cmp,		// Comparison
	RX63_div,		// Signed division
	RX63_divu,		// Unsigned division
	RX63_emul,		// Signed multiplication
	RX63_emulu,		// Unsigned multiplication 
	RX63_fadd,		// Floating-point addition
	RX63_fcmp,		// Floating-point comparison
	RX63_fdiv,		// Floating-point division
	RX63_fmul,		// Floating-point multiplication
	RX63_fsub,		// Floating-point subtraction
	RX63_ftoi,		// Floating point to integer conversion 
	RX63_int,		// Software interrupt
	RX63_itof,		// Integer to floating-point conversion
	RX63_jmp,		// Unconditional jump
	RX63_jsr,		// Jump to a subroutine
	RX63_machi,		// Multiply-Accumulate the high-order word
	RX63_maclo,		// Multiply-Accumulate the low-order word
	RX63_max,		// Selecting the highest value
	RX63_min,		// Selecting the lowest value
	RX63_mov,		// Transferring data
	RX63_movu,		// Transfer unsigned data
	RX63_mul,		// Multiplication
	RX63_mul_,		// Multiplication
	RX63_mulhi,		// Multiply the high-order word
	RX63_mullo,		// Multiply the low-order word
	RX63_mvfachi,	// Move the high-order longword from accumulator
	RX63_mvfacmi,	// Move the middle-order longword from accumulator
	RX63_mvfc,		// Transfer from a control register
	RX63_mvtachi,	// Move the high-order longword to accumulator
	RX63_mvtaclo,	// Move the low-order longword to accumulator
	RX63_mvtc,		// Transfer to a control register
	RX63_mvtipl,	// Interrupt priority level setting
	RX63_neg,		// Two’s complementation
	RX63_neg_,		// Two’s complementation
	RX63_nop,		// No operation
	RX63_not,		// Logical complementation
	RX63_not_,		// Logical complementation
	RX63_or,		// Logical OR
	RX63_or_,		// Logical OR
	RX63_pop,		// Restoring data from stack to register 
	RX63_popc,		// Restoring a control register
	RX63_popm,		// Restoring multiple registers from the stack
	RX63_push,		// Saving data on the stack
	RX63_pushc,		// Saving a control register
	RX63_pushm,		// Saving multiple registers
	RX63_racw,		// Round the accumulator word
	RX63_revl,		// Endian conversion
	RX63_revw,		// Endian conversion
	RX63_rmpa,		// Multiply-and-accumulate operation
	RX63_rolc,		// Rotation with carry to left
	RX63_rorc,		// Rotation with carry to right
	RX63_rotl,		// Rotation to left
	RX63_rotr,		// Rotation to right
	RX63_round,		// Conversion from floating-point to integer
	RX63_rte,		// Return from the exception
	RX63_rtfi,		// Return from the fast interrupt
	RX63_rts,		// Returning from a subroutine
	RX63_rtsd,		// Releasing stack frame and returning from subroutine
	RX63_rtsd_,		// Releasing stack frame and returning from subroutine
	RX63_sat,		// Saturation of signed 32-bit data
	RX63_satr,		// Saturation of signed 64-bit data for RMPA
	RX63_sbb,		// Subtraction with borrow
	RX63_sc,		// Condition setting
	RX63_scmpu,		// String comparison
	RX63_setpsw,	// Setting a flag or bit in the PSW
	RX63_shar,		// Arithmetic shift to the right
	RX63_shar_,		// Arithmetic shift to the right
	RX63_shll,		// Logical and arithmetic shift to the left
	RX63_shll_,		// Logical and arithmetic shift to the left
	RX63_shlr,		// Logical shift to the right
	RX63_shlr_,		// Logical shift to the right
	RX63_smovb,		// Transferring a string backward
	RX63_smovf,		// Transferring a string forward
	RX63_smovu,		// Transferring a string
	RX63_sstr,		// Storing a string
	RX63_stnz,		// Transfer with condition
	RX63_stz,		// Transfer with condition
	RX63_sub,		//  Subtraction without borrow
	RX63_sub_,		//  Subtraction without borrow
	RX63_suntil,	// Searching for a string
	RX63_swhile,	// Searching for a string
	RX63_tst,		// Logical test
	RX63_wait,		// Waiting
	RX63_xchg,		// Exchanging values
	RX63_xor,		// Logical exclusive or

	RX63_last
};

#endif
