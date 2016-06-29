#ifndef _Z8_HPP
#define _Z8_HPP

#include "../idaidp.hpp"
#include "ins.hpp"
#include <srarea.hpp>

#define PLFM_RX63 0x8001

enum RX63_registers
{
	r_r0, r_r1, r_r2, r_r3, r_r4, r_r5, r_r6, r_r7, r_r8, r_r9, r_r10, r_r11, r_r12, r_r13, r_r14, r_r15,
	r_isp, r_usp, r_intb, r_pc, r_psw, r_bpc, r_bpsw, r_fintv, r_fpsw, 
	r_cs, r_ds
};

#define memex specflag1
#define ld specflag2

enum memex_t
{
	b = 0,
	w = 1,
	l = 2,
	uw = 3,
	ub = 4
};

enum ld_t
{
	in_reg = 0, // @reg
	dsp8 = 1,
	dsp16 = 2,
	reg = 3	// reg
};

void idaapi header( void );
void idaapi footer( void );

void idaapi segstart( ea_t ea );
void idaapi segend( ea_t ea );

int  idaapi ana( void );
int  idaapi emu( void );
void idaapi out( void );
bool idaapi outop( op_t &op );

#endif
