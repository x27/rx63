#ifndef _Z8_HPP
#define _Z8_HPP

#include "../idaidp.hpp"
#include "ins.hpp"
#include <segregs.hpp>

#define PLFM_RX63 0x8001

enum RX63_registers
{
	r_r0, r_r1, r_r2, r_r3, r_r4, r_r5, r_r6, r_r7, r_r8, r_r9, r_r10, r_r11, r_r12, r_r13, r_r14, r_r15,
	r_isp, r_usp, r_intb, r_pc, r_psw, r_bpc, r_bpsw, r_fintv, r_fpsw, 
	r_cs, r_ds
};

#define memex specflag1
#define ld specflag2
#define phrase_type specflag3
#define o_flag o_idpspec0
#define o_creg o_idpspec1

enum rx63_cregs
{
	cr_psw = 0,
	cr_pc = 1,
	cr_usp = 2,
	cr_fpsw = 3,
	cr_bpsw = 8,
	cr_bpc = 9,
	cr_isp = 10,
	cr_fintv = 11,
	cr_intb = 12
};

enum rx63_phrases
{
	rx63_phrasesf_r_plus,		// [r+]
	rx63_phrasesf_r_minus,		// [-r]
	rx63_phrasesf_r_r,			// [r,r]
	rx63_phrasesf_r_2_r			// r-r
};

enum memex_t
{
	memex_tb	= 0,
	memex_tw	= 1,
	memex_tl	= 2,
	memex_tuw	= 3,
	memex_tub	= 4,
	memex_ts	= 5,
	memex_ta	= 6,
	memex_tdont_show = 0x10
};

enum cflag_t
{
	cflag_tflag_c = 0,
	cflag_tflag_z = 1,
	cflag_tflag_s = 2,
	cflag_tflag_o = 3,
	cflag_tflag_i = 8,
	cflag_tflag_u = 9,
};

enum condition_t
{
	condition_teq		= 0,
	condition_tne		= 1,
	condition_tc		= 2,
	condition_tnc		= 3,
	condition_tgtu		= 4,
	condition_tleu		= 5,
	condition_tpz		= 6,
	condition_tn		= 7,
	condition_tge		= 8,
	condition_tlt		= 9,
	condition_tgt		= 10,
	condition_tle		= 11,
	condition_to		= 12,
	condition_tno		= 13,
	condition_tbra		= 14,
	condition_tnone	= 15
};

enum ld_t
{
	ld_tin_reg = 0, // [reg]
	ld_tdsp8 = 1,
	ld_tdsp16 = 2,
	ld_treg = 3	// reg
};

void idaapi rx63_header(outctx_t &ctx);
void idaapi rx63_footer(outctx_t &ctx);

void idaapi rx63_segstart(outctx_t &ctx, ea_t ea);
void idaapi rx63_segend(outctx_t &ctx, ea_t ea);

int  idaapi ana(insn_t *insn);
int  idaapi emu(const insn_t *insn);

#endif
