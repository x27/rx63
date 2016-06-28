#include <ida.hpp>
#include <idp.hpp>
#include "ins.hpp"

instruc_t Instructions[] = {

  { "", 0 },										// Unknown Operation
  { "abs",		CF_USE1|CF_CHG1 },					// Absolute value
  { "abs",		CF_USE1|CF_CHG2 },					// Absolute value
  { "adc",		CF_USE1|CF_USE2|CF_CHG2 },			// Addition with carry
  { "add",		CF_USE1|CF_USE2|CF_CHG2 },			// Addition without carry
  { "add",		CF_USE1|CF_USE2|CF_CHG3 },			// Addition without carry
  { "and",		CF_USE1|CF_USE2|CF_CHG2 },			// Logical AND
  { "and",		CF_USE1|CF_USE2|CF_CHG3 },			// Logical AND
  { "bclr",		CF_USE1|CF_USE2|CF_CHG2 },			// Clearing a bit 
  { "b",		CF_USE1||CF_JUMP },					// Relative conditional branch
  { "bm",		CF_USE1|CF_USE2|CF_CHG2 },			// Conditional bit transfer
  { "bnot",		CF_USE1|CF_USE2|CF_CHG2 },			// Inverting a bit
  { "bra",		CF_USE1|CF_STOP },					// Unconditional relative branch
  { "brk",		0 },								// Unconditional trap
  { "bset",		CF_USE1|CF_USE2|CF_CHG2 },			// Setting a bit
  { "bsr",		CF_USE1|CF_CALL },					// Relative subroutine branch
  { "btst",		CF_USE1|CF_USE2 },					// Testing a bit
  { "clrpsw",	CF_CHG1 },							// Clear a flag or bit in the PSW
  { "cmp",		CF_USE1|CF_USE1 },					// Comparison
  { "div",		CF_USE1|CF_USE2|CF_CHG2 },			// Signed division
  { "divu",		CF_USE1|CF_USE2|CF_CHG2 },			// Unsigned division
  { "emul",		CF_USE1|CF_USE2|CF_CHG2 },			// Signed multiplication
  { "emulu",    CF_USE1|CF_USE2|CF_CHG2 },			// Unsigned multiplication 
  { "fadd",		CF_USE1|CF_USE2|CF_CHG2 },			// Floating-point addition
  { "fcmp",		CF_USE1|CF_USE2 },					// Floating-point comparison
  { "fdiv",		CF_USE1|CF_USE2|CF_CHG2 },			// Floating-point division
  { "fmul",		CF_USE1|CF_USE2|CF_CHG2 },			// Floating-point multiplication
  { "fsub",		CF_USE1|CF_USE2|CF_CHG2 },			// Floating-point subtraction
  { "ftoi",		CF_USE1|CF_CHG2 },					// Floating point to integer conversion 
  { "int",		0 },								// Software interrupt
  { "itof",		CF_USE1|CF_CHG2 },					// Integer to floating-point conversion
  { "jmp",		CF_USE1|CF_JUMP|CF_STOP },			// Unconditional jump
  { "jsr",		CF_USE1|CF_CALL },					// Jump to a subroutine
  { "machi",    CF_USE1|CF_USE2 },					// Multiply-Accumulate the high-order word
  { "maclo",    CF_USE1|CF_USE2 },					// Multiply-Accumulate the low-order word
  { "max",		CF_USE1|CF_USE2|CF_CHG2 },			// Selecting the highest value
  { "min",		CF_USE1|CF_USE2|CF_CHG2 },			// Selecting the lowest value
  { "mov",		CF_USE1|CF_CHG2 },					// Transferring data
  { "movu",		CF_USE1|CF_CHG2 },					// Transfer unsigned data
  { "mul",		CF_USE1|CF_USE2|CF_CHG2 },			// Multiplication
  { "mul",		CF_USE1|CF_USE2|CF_CHG3 },			// Multiplication
  { "mulhi",    CF_USE1|CF_USE2 },					// Multiply the high-order word
  { "mullo",    CF_USE1|CF_USE2 },					// Multiply the low-order word
  { "mvfachi",	CF_CHG1 },							// Move the high-order longword from accumulator
  { "mvfacmi",	CF_CHG1 },							// Move the middle-order longword from accumulator
  { "mvfc",		CF_USE1|CF_CHG2 },					// Transfer from a control register
  { "mvtachi",  CF_USE1 },							// Move the high-order longword to accumulator
  { "mvtaclo",  CF_USE1 },							// Move the low-order longword to accumulator
  { "mvtc",		CF_USE1|CF_CHG2 },					// Transfer to a control register
  { "mvtipl",   CF_USE1 },							// Interrupt priority level setting
  { "neg",		CF_USE1|CF_CHG1 },					// Two’s complementation
  { "neg",		CF_USE1|CF_CHG2 },					// Two’s complementation
  { "nop",		0 },								// No operation
  { "not",		CF_USE1|CF_CHG1 },					// Logical complementation
  { "not",		CF_USE1|CF_CHG2 },					// Logical complementation
  { "or",		CF_USE1|CF_USE2|CF_CHG2 },			// Logical OR
  { "or",		CF_USE1|CF_USE2|CF_CHG3 },			// Logical OR
  { "pop",		CF_CHG1 },							// Restoring data from stack to register 
  { "popc",		CF_CHG1 },							// Restoring a control register
  { "popm",		CF_CHG1|CF_CHG2 },					// Restoring multiple registers from the stack
  { "push",		CF_USE1 },							// Saving data on the stack
  { "pushc",    CF_USE1 },							// Saving a control register
  { "pushm",    CF_USE1|CF_USE2 },					// Saving multiple registers
  { "racw",		CF_USE1 },							// Round the accumulator word
  { "revl",		CF_USE1|CF_CHG2 },					// Endian conversion
  { "revw",		CF_USE1|CF_CHG2 },					// Endian conversion
  { "rmpa",		0 },								// Multiply-and-accumulate operation
  { "rolc",		CF_CHG1|CF_SHFT },					// Rotation with carry to left
  { "rorc",		CF_CHG1|CF_SHFT },					// Rotation with carry to right
  { "rotl",		CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },	// Rotation to left
  { "rotr",		CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },	// Rotation to right
  { "round",    CF_USE1|CF_CHG2 },					// Conversion from floating-point to integer
  { "rte",		CF_STOP },							// Return from the exception
  { "rtfi",		CF_STOP },							// Return from the fast interrupt
  { "rts",		CF_STOP },							// Returning from a subroutine
  { "rtsd",		CF_USE1 },							// Releasing stack frame and returning from subroutine
  { "rtsd",		CF_USE1|CF_CHG2|CF_CHG3 },			// Releasing stack frame and returning from subroutine
  { "sat",		CF_CHG1 },							// Saturation of signed 32-bit data
  { "satr",		0 },								// Saturation of signed 64-bit data for RMPA
  { "sbb",		CF_USE1|CF_USE2|CF_CHG2 },			// Subtraction with borrow
  { "sc",		CF_USE1 },							// Condition setting
  { "scmpu",    0 },								// String comparison
  { "setpsw",   CF_CHG1 },							// Setting a flag or bit in the PSW
  { "shar",		CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },	// Arithmetic shift to the right
  { "shar",		CF_USE1|CF_USE2|CF_CHG3|CF_SHFT },	// Arithmetic shift to the right
  { "shll",		CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },	// Logical and arithmetic shift to the left
  { "shll",		CF_USE1|CF_USE2|CF_CHG3|CF_SHFT },	// Logical and arithmetic shift to the left
  { "shlr",		CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },	// Logical shift to the right
  { "shlr",		CF_USE1|CF_USE2|CF_CHG3|CF_SHFT },	// Logical shift to the right
  { "smovb",    0 },								// Transferring a string backward
  { "smovf",    0 },								// Transferring a string forward
  { "smovu",    0 },								// Transferring a string
  { "sstr",		0 },								// Storing a string
  { "stnz",		CF_USE1|CF_CHG2 },					// Transfer with condition
  { "stz",		CF_USE1|CF_CHG2 },					// Transfer with condition
  { "sub",		CF_USE1|CF_USE2|CF_CHG2 },			//  Subtraction without borrow
  { "sub",		CF_USE1|CF_USE2|CF_CHG3 },			//  Subtraction without borrow
  { "suntil",   0 },								// Searching for a string
  { "swhile",   0 },								// Searching for a string
  { "tst",		CF_USE1|CF_USE2 },					// Logical test
  { "wait",		0 },								// Waiting
  { "xchg",		CF_USE1|CF_USE2|CF_CHG1|CF_CHG2 },	// Exchanging values
  { "xor",		CF_USE1|CF_USE2|CF_CHG2 },			// Logical exclusive or

};

CASSERT(sizeof(Instructions)/sizeof(Instructions[0]) == RX63_last);
