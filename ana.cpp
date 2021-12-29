#include "rx63.hpp"

enum simm_t 
{
	imm32	= 0,
	simm8	= 1,
	simm16	= 2,
	simm24	= 3,
};

void set_reg(op_t &op, uint16 reg) 
{
	op.type = o_reg;
	op.reg = reg;
	op.dtype = dt_word;
}

void set_displ_reg(op_t &op, uint16 reg, uval_t value, memex_t memex) 
{
	op.type = o_displ;
	op.value = value;
	op.reg = reg;
	op.dtype = dt_word;
	op.memex = (char)memex;
}

inline ea_t real_address( ea_t ea )
{
	if (inf_is_be())
		return (( ea >> 2 << 2 ) + 3 - ( ea & 3 ));
	return ea;
}

uchar get_hl8( ea_t ea)
{
	return (uchar) get_wide_byte(real_address( ea ) );
}

uint get_hl32( ea_t ea )
{
	uint res = 0;
	for( int i=3; i>=0; i-- )
	{
		res = res << 8;
		res = res | get_hl8( ea + i );
	}
	return res;
}

uint get_hl24( ea_t ea )
{
	uint res = 0;
	for( int i=2; i>=0; i-- )
	{
		res = res << 8;
		res = res | get_hl8( ea + i );
	}
	return res;
}

int get_signed_hl24( ea_t ea )
{
	uint res = get_hl24( ea );
	if ((res & 0x800000) != 0)
	{
		return (int)(res | 0xff000000);
	}
	else
	{
		return res;
	}
}


ushort get_hl16( ea_t ea )
{
	return (get_hl8( ea + 1) << 8 ) | get_hl8 ( ea );
}

int set_imm(op_t &op, simm_t imm, ea_t ea) 
{
	int size = 0;
	op.type = o_imm;
	switch(imm)
	{
		case imm32:
			size = 4;
			op.value = get_hl32( ea );
			op.dtype = dt_dword;
			break;
		case simm8:
			size = 1;
			op.value = get_hl8( ea );
			op.dtype = dt_byte;
			break;
		case simm16:
			size = 2;
			op.value = get_hl16( ea );
			op.dtype = dt_word;
			break;
		case simm24:
			size = 3;
			op.value = get_signed_hl24( ea );
			op.dtype = dt_dword;
			break;
	}
	return size;
}

int get_scale(memex_t mem)
{
	memex_t m = (memex_t)((int)mem & 0xf);
	int scale = 1;
	if ( memex_tw == m || memex_tuw == m )
		scale = 2;
	else if ( memex_tl == m )
		scale = 4;
	return scale;
}

int set_displ(op_t &op, ld_t ld, memex_t memex, uval_t reg, ea_t ea)
{
	int res = 0;
	op.ld = (char)ld;

	if (ld == ld_tdsp8)
	{
		uval_t value = get_scale(memex) * get_hl8( ea );
		set_displ_reg(op, reg, value, memex);
		res = 1;
	}
	else if (ld == ld_tdsp16)
	{
		uval_t value = get_scale(memex) * get_hl16( ea );
		//msg("value %a %a\n", ea, value);
		set_displ_reg(op, reg, value, memex);
		res = 2;
	}
	else
	{
		set_displ_reg(op, reg, 0, memex);
	}
	return res;
}

/* -------------------------------------------------------------*/
/* ---------	instruction providers -------------------------- */
/* -------------------------------------------------------------*/

void b1_cd_dsp_s(insn_t *insn)
{
	uchar data = get_hl8( insn->ea );
	insn->auxpref = (data & 0x08) == 0 ? condition_teq : condition_tne;
	insn->auxpref |= (memex_ts + 1) << 4;
	insn->Op1.type = o_near;
	insn->Op1.dtype = dt_code;

	uint disp = data & 7;
	if (disp < 3)
		disp += 8;
	insn->Op1.addr = insn->ea + disp;
	insn->size = 1;
}

void b1_cd_dsp_b(insn_t *insn)
{
	uchar data = get_hl8( insn->ea );

	uchar cd = data & 0xf;

	if (cd == 0xf || cd == 0xe)
		return;

	insn->auxpref = cd;
	insn->auxpref |= (memex_tb + 1) << 4;
	insn->Op1.type = o_near;
	insn->Op1.dtype = dt_code;

	insn->Op1.addr = insn->ea + (char)get_hl8( insn->ea + 1);
	insn->size = 2;
}

void b1_cd_dsp_w(insn_t *insn)
{
	uchar data = get_hl8( insn->ea );
	insn->auxpref = data & 1 ? condition_tne : condition_teq;
	insn->auxpref |= (memex_tw + 1) << 4;
	insn->Op1.type = o_near;
	insn->Op1.dtype = dt_code;

	insn->Op1.addr = insn->ea + (short)get_hl16( insn->ea + 1);
	insn->size = 3;
}

void b1_dsp(insn_t *insn)
{
	uint disp = get_hl8( insn->ea ) & 7;
	if (disp < 3)
		disp += 8;

	insn->Op1.type = o_near;
	insn->Op1.dtype = dt_code;
	insn->auxpref |= (memex_ts + 1) << 4;
	insn->Op1.addr = insn->ea + disp;
	insn->size = 1;
}

void b1_pcdsp8(insn_t *insn)
{
	insn->Op1.type = o_near;
	insn->Op1.dtype = dt_code;
	insn->auxpref |= (memex_tb + 1) << 4;
	insn->Op1.addr = insn->ea + (char)get_hl8( insn->ea + 1 );
	insn->size = 2;
}

void b1_pcdsp16(insn_t *insn)
{
	insn->Op1.type = o_near;
	insn->Op1.dtype = dt_code;
	insn->auxpref |= (memex_tw + 1) << 4;
	insn->Op1.addr = insn->ea + (short)get_hl16( insn->ea + 1 );
	insn->size = 3;
}

void b1_pcdsp24(insn_t *insn)
{
	insn->Op1.type = o_near;
	insn->Op1.dtype = dt_code;
	insn->auxpref |= (memex_ta + 1) << 4;
	insn->Op1.addr = insn->ea + get_signed_hl24( insn->ea + 1 );
	insn->size = 4;
}

void b1_imm4_rd(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 1 );

	insn->size = 2;
	insn->Op1.type = o_imm;
	insn->Op1.value = data >> 4;
	set_reg(insn->Op2, data & 0xf);
}

void b1_imm4_rd_(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 1 );

	insn->auxpref |= (memex_tl + 1) << 4;

	insn->size = 2;
	insn->Op1.type = o_imm;
	insn->Op1.value = data >> 4;
	set_reg(insn->Op2, data & 0xf);
}

void b1_imm5_rd(insn_t *insn)
{
	ushort data0 = (get_hl8( insn->ea ) & 1) << 4;
	ushort data1 = get_hl8( insn->ea + 1 );

	insn->size = 2;
	insn->Op1.type = o_imm;
	insn->Op1.value = (data0 | (data1 >> 4)) & 0x1f;
	set_reg(insn->Op2, data1 & 0xf);
}

void b1_ld_rd_li_sz(insn_t *insn)
{
	ld_t ld = (ld_t)(get_hl8( insn->ea ) & 3);
	if (ld == 3)
		return;

	uchar data = get_hl8( insn->ea + 1);
	memex_t sz = (memex_t) (data & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	insn->size = 2 + set_displ(insn->Op2, ld, (memex_t)(sz|memex_tdont_show), data >> 4, insn->ea + 2);

	simm_t li = (simm_t)( (data >> 2) & 3);
	insn->size += set_imm(insn->Op1, li, insn->ea + insn->size );
}

void b1_ld_rs_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	ld_t ld = (ld_t)(data0 & 3);

	insn->size = 2 + set_displ(insn->Op1, ld, memex_tub, data1 >> 4, insn->ea + 2);
	set_reg(insn->Op2, data1 & 0xf);
}

void b1_li_rs2_rd(insn_t *insn)
{
	simm_t li = (simm_t)(get_hl8( insn->ea ) & 3);
	uchar data = get_hl8 ( insn->ea + 1 );

	insn->size = 2 + set_imm(insn->Op1, li, insn->ea + 2);
	set_reg(insn->Op2, data >> 4);
	set_reg(insn->Op3, data & 0xf );
}

void b1_no_args(insn_t *insn)
{
	insn->size = 1;
}

void b1_rd_rd2(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 1 );

	insn->Op1.type = o_phrase;
	insn->Op1.phrase_type = rx63_phrasesf_r_2_r;
	insn->Op1.value = data >> 4;
	insn->Op1.reg = data & 0xf;
	insn->size = 2;
}

void b1_rd_rd2_imm8(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 1 );

	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = get_hl8( insn->ea + 2 );

	insn->Op2.type = o_phrase;
	insn->Op2.phrase_type = rx63_phrasesf_r_2_r;
	insn->Op2.value = data >> 4;
	insn->Op1.reg = data & 0xf;
	insn->size = 3;
}

void b1_sz_ld_rd_rs(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 3);
	if (sz == 3)
		return;

	ld_t ld = (ld_t) ( (data0 >> 2) & 3 );
	if (ld == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	set_reg(insn->Op1, data1 & 0xf);
	insn->size = 2 + set_displ(insn->Op2, ld, (memex_t)(sz|memex_tdont_show), data1 >> 4, insn->ea + 2);
}

void b1_sz_ld_rs_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	ld_t ld = (ld_t) ( data0 & 3 );
	if (ld == 3)
		return;

	insn->size = 2 + set_displ(insn->Op1, ld, (memex_t)(sz|memex_tdont_show), data1 >> 4, insn->ea + 2);

	set_reg(insn->Op2, data1 & 0xf);
}

void b1_sz_ld_rs_rd_u(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data0 >> 2 ) & 1);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	ld_t ld = (ld_t) ( data0 & 3 );

	insn->size = 2 + set_displ(insn->Op1, ld, (memex_t)(sz|memex_tdont_show), data1 >> 4, insn->ea + 2);

	set_reg(insn->Op2, data1 & 0xf);
}


void b1_sz_ldd_lds_rs_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 3);
	if (sz == 3)
		return;

	ld_t ldd = (ld_t) ( (data0 >> 2) & 3 );
	if (ldd == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	ld_t lds = (ld_t) ( data0  & 3 );
	if (lds == 3)
		return;

	insn->size = 2 + set_displ(insn->Op1, lds, (memex_t)(sz|memex_tdont_show), data1 >> 4, insn->ea + 2);
	insn->size += set_displ(insn->Op2, ldd, (memex_t)(sz|memex_tdont_show), data1 & 0xf, insn->ea + insn->size);
}

void b1_sz_rd7_rs(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	set_reg(insn->Op1, data1 & 7);

	uchar dsp = get_scale( sz ) * ((data0 & 7) << 2) | ((data1 >> 6) & 2) | ((data1 >> 3) & 1);
	set_displ_reg(insn->Op2, (data1 >> 4) & 7, dsp, (memex_t)(sz|memex_tdont_show));
	insn->size = 2;
}

void b1_sz_rd7_rs7(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data0 >> 3 ) & 1);

	insn->auxpref |= (sz + 1) << 4;

	uchar dsp = get_scale( sz ) * ((data0 & 7) << 2) | ((data1 >> 6) & 2) | ((data1 >> 3) & 1);
	set_displ_reg(insn->Op1, (data1 >> 4) & 7, dsp, (memex_t)(sz|memex_tdont_show));

	set_reg(insn->Op2, data1 & 7);

	insn->size = 2;
}


void b1_sz_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)( data0 & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	insn->Op1.type = o_imm;
	insn->Op1.value = get_hl8( insn->ea + 2 );

	uchar dsp = get_scale( sz ) * (((data1 & 0x80) >> 3) | (data1  & 0xf));
	set_displ_reg(insn->Op2, (data1 >> 4) & 7, dsp, (memex_t)(sz|memex_tdont_show));

	insn->size = 3;
}

void b1_sz_rs_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	set_reg(insn->Op1, data1 >> 4);
	set_reg(insn->Op2, data1 & 0xf);

	insn->size = 2;
}

void b1_sz_rs7_rd7(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea );
	uchar data1 = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	set_reg(insn->Op2, data1 & 7);

	uchar dsp = get_scale( sz ) * ((data0 & 7) << 2) | ((data1 >> 6) & 2) | ((data1 >> 3) & 1);
	set_displ_reg(insn->Op1, (data1 >> 4) & 7, dsp, (memex_t)(sz|memex_tdont_show));
	insn->size = 2;
}

void b1_uimm8(insn_t *insn)
{
	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = get_hl8( insn->ea + 1 );
	insn->size = 2;
}

void b2_ad_sz_rds(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	memex_t sz = (memex_t)( data0 & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	uchar ad = (data0 >> 2) & 3;

	if (ad < 2)
	{
		set_reg(insn->Op1, data1 & 0xf);
	
		insn->Op2.type = o_phrase;
		insn->Op2.phrase_type = ad == 0 ? rx63_phrasesf_r_plus : rx63_phrasesf_r_minus;
		insn->Op2.reg = data1 >> 4;
	}
	else
	{
		set_reg(insn->Op2, data1 & 0x0f);
	
		insn->Op1.type = o_phrase;
		insn->Op1.phrase_type = ad == 2 ? rx63_phrasesf_r_plus : rx63_phrasesf_r_minus;
		insn->Op1.reg = data1 >> 4;
	}

	insn->size = 3;
}

void b2_ad_sz_rs_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	memex_t sz = (memex_t)( data0 & 1);

	insn->auxpref |= (sz + 1) << 4;

	uchar ad = (data0 >> 2) & 3;
	if (ad < 2)
		return;

	set_reg(insn->Op2, data1 & 0x0f);
	
	insn->Op1.type = o_phrase;
	insn->Op1.phrase_type = ad == 2 ? rx63_phrasesf_r_plus : rx63_phrasesf_r_minus;
	insn->Op1.reg = data1 >> 4;

	insn->size = 3;
}

void b2_cb(insn_t *insn)
{
	uchar flag = get_hl8( insn->ea + 1 ) & 0xf;
	if ((flag > 3 && flag < 8) || flag > 9)
		return;

	insn->size = 2;
	insn->Op1.type = o_flag;
	insn->Op1.value = flag;
}

void b2_cr(insn_t *insn)
{
	uchar cr =  get_hl8( insn->ea + 1 ) & 0xf;

	if (cr == 1 || cr > 3 && cr < 8 || cr > 12)
		return;

	insn->size = 2;
	insn->Op1.type = o_creg;
	insn->Op1.value = cr;
}

void b2_cr_(insn_t *insn)
{
	uchar cr =  get_hl8( insn->ea + 1 ) & 0xf;

	if (cr > 3 && cr < 8 || cr > 12)
		return;

	insn->size = 2;
	insn->Op1.type = o_creg;
	insn->Op1.value = cr;
}

void b2_cr_rd(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 2 );

	uchar creg =  data >> 4;

	if ((creg > 3 && creg < 8) || creg > 12)
		return;

	insn->size = 3;
	insn->Op1.type = o_creg;
	insn->Op1.value = creg;

	set_reg(insn->Op2, data & 0xf);
}

void b2_imm3_ld_rd_cd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	uchar cd = data1 & 0xf;

	if (cd == 0xe || cd == 0xf)
		return;

	ld_t ld = (ld_t) ( data0 & 3 );

	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = (data0 >> 2) & 7;

	insn->auxpref = cd;

	insn->size = 3 + set_displ(insn->Op2, ld, memex_tb, data1 >> 4, insn->ea + 3);
}

void b2_imm5_cd_rd(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 2 );

	uchar cd = data >> 4;
	if (cd == 0xe || cd == 0xf)
		return;

	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = get_hl8( insn->ea + 1 ) & 0x1f;

	insn->auxpref = cd;

	set_reg(insn->Op2, data & 0xf);
	insn->size = 3;
}

void b2_imm5_rd(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 2 );

	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = (data >> 4) & 0x1f;

	set_reg( insn->Op2, data & 0xf);
	insn->size = 3;
}

void b2_imm5_rs2_rd(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 2 );

	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = get_hl8( insn->ea + 1 ) & 0x1f;

	set_reg( insn->Op2, data >> 4 );
	set_reg( insn->Op3, data & 0xf );

	insn->size = 3;
}

void b2_imm8(insn_t *insn)
{
	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = get_hl8( insn->ea + 2 );
	insn->size = 3;
}

void b2_ld_rd_imm3(insn_t *insn)
{
	ld_t ld = (ld_t) ( get_hl8( insn->ea ) & 3 );
	uchar data = get_hl8( insn->ea + 1 );

	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = data & 7;

	insn->size = 2 + set_displ( insn->Op2, ld, memex_tb, data >> 4, insn->ea + 2);
}

void b2_ld_rd_rs(insn_t *insn)
{
	ld_t ld = (ld_t) ( get_hl8( insn->ea + 1) & 3 );
	uchar data = get_hl8( insn->ea + 2 );

	set_reg(insn->Op1, data & 0xf);
	insn->size = 3 + set_displ(insn->Op2, ld, memex_tb, data >> 4, insn->ea + 3);
}

void b2_ld_rs_rd_ub(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 2 );

	ld_t ld = (ld_t)(get_hl8( insn->ea + 1 ) & 3);

	insn->size = 3 + set_displ(insn->Op1, ld, memex_tub, data >> 4, insn->ea + 3);
	set_reg(insn->Op2, data & 0xf);
}

void b2_ld_rs_rd_l(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 2 );

	ld_t ld = (ld_t)(get_hl8( insn->ea + 1 ) & 3);

	insn->size = 3 + set_displ(insn->Op1, ld, memex_tl, data >> 4, insn->ea + 3);
	set_reg(insn->Op2, data & 0xf);
}


void b2_ld_rs_sz(insn_t *insn)
{
	ld_t ld = (ld_t)(get_hl8( insn->ea ) & 3);
	if (ld == 3)
		return;

	uchar data = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)( data & 3 );
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	insn->size = 2 + set_displ(insn->Op1, ld, (memex_t)(sz|memex_tdont_show), data >> 4, insn->ea + 2);
}

void b2_li_rd(insn_t *insn)
{
	simm_t li = (simm_t)(get_hl8( insn->ea ) & 3);
	uchar data = get_hl8 ( insn->ea + 1 );

	insn->size = 2 + set_imm(insn->Op1, li, insn->ea + 2);
	set_reg(insn->Op2, data & 0xf);
}

void b2_mi_ld_rs_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	memex_t memex = (memex_t)(data0 >> 6);
	ld_t ld = (ld_t)(data0 & 3);
	
	insn->size = 3 + set_displ(insn->Op1, ld, memex, data1 >> 4, insn->ea + 3);
	set_reg(insn->Op2, data1 & 0xf);
}

void b2_no_args(insn_t *insn)
{
	insn->size = 2;
}

void b2_rd(insn_t *insn)
{
	set_reg(insn->Op1,  get_hl8( insn->ea + 1 ) & 0x0f);
	insn->size = 2;
}

void b2_rd_li(insn_t *insn)
{
	uchar data = get_hl8 ( insn->ea + 1 );
	insn->auxpref |= (memex_tl + 1) << 4;
	simm_t li = (simm_t)( (data >> 2) & 3);
	insn->size = 2 + set_imm(insn->Op1, li, insn->ea + 2);
	set_reg(insn->Op2, (data >> 4) & 0xf);
}

void b2_rd_rs_rs2(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	set_reg(insn->Op1, data1 >> 4);
	set_reg(insn->Op2, data1 & 0xf);
	set_reg(insn->Op3, data0 & 0xf );

	insn->size = 3;
}

void b2_rs(insn_t *insn)
{
	insn->auxpref |= (memex_tl + 1) << 4;
	set_reg(insn->Op1, get_hl8( insn->ea + 1) & 0xf);
	insn->size = 2;
}

void b2_rs_(insn_t *insn)
{
	set_reg(insn->Op1, get_hl8( insn->ea + 1) & 0xf);
	insn->size = 2;
}

void b2_rs_rd(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 2 );
	insn->size = 3;
	set_reg(insn->Op1, data >> 4);
	set_reg(insn->Op2, data & 0xf);
}

void b2_rs2_uimm8(insn_t *insn)
{
	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = get_hl8( insn->ea + 2 );
	set_reg(insn->Op2, get_hl8( insn->ea + 1 ) & 0xf);
	insn->size = 3;
}

void b2_rs2_uimm8_(insn_t *insn)
{
	insn->auxpref |= (memex_tl + 1) << 4;
	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = get_hl8( insn->ea + 2 );
	set_reg(insn->Op2, get_hl8( insn->ea + 1 ) & 0xf);
	insn->size = 3;
}

void b2_rs_cr(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 2 );

	uchar cr = data & 0xf;

	if (cr == 1 || cr > 3 && cr < 8 || cr > 12)
		return;

	set_reg(insn->Op1, data >> 4);

	insn->Op2.type = o_creg;
	insn->Op2.value = cr;
	insn->Op2.dtype = dt_byte;
	insn->size = 3;
}

void b2_sz(insn_t *insn)
{
	memex_t sz = (memex_t)( get_hl8( insn->ea + 1 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;
	insn->size = 2;
}

void b2_sz_ld_rd_cd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	memex_t sz = (memex_t)(( data0 >> 2 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	ld_t ld = (ld_t)(data0 & 3);

	uchar cd = data1 & 0xf;

	if (cd == 0xf || cd == 0xe)
		return;

	insn->auxpref = cd;
	insn->auxpref |= (sz + 1) << 4;

	insn->size = 3 + set_displ(insn->Op1, ld, (memex_t)(sz|memex_tdont_show), data1 >> 4, insn->ea + 3);
}

void b2_sz_ri_rb_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	insn->Op1.type = o_phrase;
	insn->Op1.phrase_type = rx63_phrasesf_r_r;
	insn->Op1.reg = data1 >> 4;
	insn->Op1.value = data0 & 0xf;
	insn->Op1.dtype = dt_word;

	set_reg(insn->Op2, data1 & 0x0f);
	insn->size = 3;
}

void b2_sz_ri_rb_rd_u(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 1);

	insn->auxpref |= (sz + 1) << 4;

	insn->Op1.type = o_phrase;
	insn->Op1.phrase_type = rx63_phrasesf_r_r;
	insn->Op1.reg = data1 >> 4;
	insn->Op1.value = data0 & 0xf;
	insn->Op1.dtype = dt_word;

	set_reg(insn->Op2, data1 & 0x0f);
	insn->size = 3;
}

void b2_sz_ri_rb_rs(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	memex_t sz = (memex_t)(( data0 >> 4 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	insn->Op2.type = o_phrase;
	insn->Op2.phrase_type = rx63_phrasesf_r_r;
	insn->Op2.reg = data1 >> 4;
	insn->Op2.value = data0 & 0xf;
	insn->Op2.dtype = dt_word;

	set_reg(insn->Op1, data1 & 0x0f);
	insn->size = 3;
}

void b2_sz_rs(insn_t *insn)
{
	uchar data = get_hl8( insn->ea + 1 );

	memex_t sz = (memex_t)(( data >> 4 ) & 3);
	if (sz == 3)
		return;

	insn->auxpref |= (sz + 1) << 4;

	set_reg(insn->Op1, data & 0xf);
	insn->size = 2;
}

void b3_imm1(insn_t *insn)
{
	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = (get_hl8( insn->ea + 2) & 0x10) == 0 ? 1 : 2;
	insn->size = 3;
}

void b3_imm3_ld_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 2 );

	ld_t ld = (ld_t)(data0 & 3);

	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = (data0 >> 2) & 7;

	insn->size = 3 + set_displ(insn->Op2, ld, memex_tb, data1 >> 4, insn->ea + 3);
}

void b3_imm4(insn_t *insn)
{
	insn->Op1.type = o_imm;
	insn->Op1.value = get_hl8( insn->ea + 2) & 0xf;
	insn->Op1.dtype = dt_byte;
	insn->size = 3;
}

void b3_imm5_rd(insn_t *insn)
{
	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_byte;
	insn->Op1.value = get_hl8( insn->ea + 1 ) & 0x1f;

	set_reg( insn->Op2, get_hl8( insn->ea + 2 ) & 0xf );
	insn->size = 3;
}

void b3_ld_rs_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 3 );

	ld_t ld = (ld_t)(data0 & 3);
	if (ld == 3)
		return;

	memex_t memex = (memex_t)(data0 >> 6);

	insn->size = 4 + set_displ(insn->Op1, ld, memex, data1 >> 4, insn->ea + 4);
	set_reg(insn->Op2, data1 & 0xf);
}

void b3_li_cr(insn_t *insn)
{
	simm_t li = (simm_t)((get_hl8( insn->ea + 1 ) >> 2) & 3);
	uchar cr = get_hl8( insn->ea + 2) & 0xf;

	if (cr == 1 || cr >3 && cr < 8 || cr > 12)
		return;

	insn->size = 3 + set_imm(insn->Op1, li, insn->ea + 3);

	insn->Op2.type = o_creg;
	insn->Op2.value = cr;
	insn->Op2.dtype = dt_byte;
}

void b3_li_rd(insn_t *insn)
{
	simm_t li = (simm_t)((get_hl8( insn->ea + 1 ) >> 2) & 3);
	uchar data = get_hl8( insn->ea + 2 ) & 0xf;

	insn->size = 3 + set_imm(insn->Op1, li, insn->ea + 3);
	set_reg(insn->Op2,  data);
}

void b3_mi_ld_rs_rd(insn_t *insn)
{
	uchar data0 = get_hl8( insn->ea + 1 );
	uchar data1 = get_hl8( insn->ea + 3 );

	memex_t memex = (memex_t)(data0 >> 6);
	ld_t ld = (ld_t)(data0 & 3);
	
	insn->size = 4 + set_displ(insn->Op1, ld, memex, data1 >> 4, insn->ea + 3);
	set_reg(insn->Op2, data1 & 0xf);
}

void b3_rd_imm32(insn_t *insn)
{
	insn->Op1.type = o_imm;
	insn->Op1.dtype = dt_dword;
	insn->Op1.value = get_hl32( insn->ea + 3 );

	set_reg(insn->Op2, get_hl8( insn->ea + 2) & 0xf);
	insn->size = 7;
}

void b3_rd(insn_t *insn)
{
	set_reg( insn->Op1, get_hl8( insn->ea + 2) & 0xf);
	insn->size = 3;
}

struct function_desc_t
{
	void (*function)(insn_t*);
	int opcode_count;
	uchar mask[4];
};

static struct function_desc_t function_descs[] = 
{
	{ &b1_cd_dsp_s,		1, { 0xf0 } },
	{ &b1_cd_dsp_b,		1, { 0xf0 } },
	{ &b1_cd_dsp_w,		1, { 0xfe } },
	{ &b1_dsp,			1, { 0xf8 } },
	{ &b1_imm4_rd,		1, { 0xff } },
	{ &b1_imm4_rd_,		1, { 0xff } },
	{ &b1_imm5_rd,		1, { 0xfe } },
	{ &b1_ld_rd_li_sz,	1, { 0xfc } },
	{ &b1_ld_rs_rd,		1, { 0xfc } },
	{ &b1_li_rs2_rd,	1, { 0xfc } },
	{ &b1_no_args,		1, { 0xff } },
	{ &b1_pcdsp8,		1, { 0xff } },
	{ &b1_pcdsp16,		1, { 0xff } },
	{ &b1_pcdsp24,		1, { 0xff } },
	{ &b1_rd_rd2,		1, { 0xff } },
	{ &b1_rd_rd2_imm8,	1, { 0xff } },
	{ &b1_sz_ld_rd_rs,	1, { 0xc3 } },
	{ &b1_sz_ld_rs_rd,	1, { 0xcc } },
	{ &b1_sz_ld_rs_rd_u,1, {0xf8 } },
	{ &b1_sz_ldd_lds_rs_rd, 1, { 0xc0 } },
	{ &b1_sz_rd,		1, { 0xfc } },
	{ &b1_sz_rd7_rs,	1, { 0xc8 } },
	{ &b1_sz_rs_rd,		1, { 0xcf } },
	{ &b1_sz_rd7_rs7,	1, { 0xf0 } },
	{ &b1_sz_rs7_rd7,	1, { 0xc8 } },
	{ &b1_uimm8,		1, { 0xff } },
	{ &b2_ad_sz_rds,	2, { 0xff, 0xf0 } },
	{ &b2_ad_sz_rs_rd,	2, { 0xff, 0xf2 } },
	{ &b2_cb,			2, { 0xff, 0xf0 } },
	{ &b2_cr,			2, { 0xff, 0xf0 } },
	{ &b2_cr_,			2, { 0xff, 0xf0 } },
	{ &b2_cr_rd,		2, { 0xff, 0xff } },
	{ &b2_imm3_ld_rd_cd,2, { 0xff, 0xe0 } },
	{ &b2_imm5_cd_rd,	2, { 0xff, 0xe0 } }, 
	{ &b2_imm5_rd,		2, { 0xff, 0xfe } },
	{ &b2_imm5_rs2_rd,	2, { 0xff, 0xe0 } },
	{ &b2_imm8,			2, { 0xff, 0xff } },
	{ &b2_ld_rd_imm3,	2, { 0xfc, 0x08 } },
	{ &b2_ld_rd_rs,		2, { 0xff, 0xfc } },
	{ &b2_ld_rs_rd_ub,	2, { 0xff, 0xfc } },
	{ &b2_ld_rs_rd_l,	2, { 0xff, 0xfc } },
	{ &b2_ld_rs_sz,		2, { 0xfc, 0x0c } },
	{ &b2_li_rd,		2, { 0xfc, 0xf0 } },
	{ &b2_mi_ld_rs_rd,	2, { 0xff, 0x3c } },
	{ &b2_no_args,		2, { 0xff, 0xff } },
	{ &b2_rd,			2, { 0xff, 0xf0 } },
	{ &b2_rd_li,		2, { 0xff, 0x03 } },
	{ &b2_rd_rs_rs2,	2, { 0xff, 0xf0 } }, 
	{ &b2_rs2_uimm8,	2, { 0xff, 0xf0 } },
	{ &b2_rs2_uimm8_,	2, { 0xff, 0xf0 } },
	{ &b2_rs_cr,		2, { 0xff, 0xff } },
	{ &b2_rs,			2, { 0xff, 0xf0 } },
	{ &b2_rs_,			2, { 0xff, 0xf0 } },
	{ &b2_rs_rd,		2, { 0xff, 0xff } },
	{ &b2_sz,			2, { 0xff, 0xfc } },
	{ &b2_sz_ld_rd_cd,	2, { 0xff, 0xf0 } },
	{ &b2_sz_ri_rb_rd,	2, { 0xff, 0xc0 } },
	{ &b2_sz_ri_rb_rd_u,2, { 0xff, 0xe0 } },
	{ &b2_sz_ri_rb_rs,	2, { 0xff, 0xc0 } },
	{ &b2_sz_rs,		2, { 0xff, 0xc0 } },
	{ &b3_imm1,			3, { 0xff, 0xff, 0xef } },
	{ &b3_imm3_ld_rd,	3, { 0xff, 0xe0, 0x0f } },
	{ &b3_imm4,			3, { 0xff, 0xff, 0xf0 } },
	{ &b3_imm5_rd,		3, { 0xff, 0xe0, 0xf0 } },
	{ &b3_ld_rs_rd,		3, { 0xff, 0xfc, 0xff } },
	{ &b3_li_cr,		3, { 0xff, 0xf3, 0xf0 } },
	{ &b3_li_rd,		3, { 0xff, 0xf3, 0xf0 } },
	{ &b3_mi_ld_rs_rd,	3, { 0xff, 0x3c, 0xff } },
	{ &b3_rd_imm32,		3, { 0xff, 0xff, 0xf0 } },
	{ &b3_rd,			3, { 0xff, 0xff, 0xf0 } }
};

struct opcode_t
{
	int instruction;
	void (*function)(insn_t*);
	uchar opcodes[4];
};

static struct opcode_t opcodes[] = {
	{ RX63_abs,		&b2_rd,				{ 0x7e, 0x20 } },
	{ RX63_abs_,	&b2_rs_rd,			{ 0xfc, 0x0f } },

	{ RX63_adc,		&b3_li_rd,			{ 0xfd, 0x70, 0x20 } },
	{ RX63_adc,		&b2_rs_rd,			{ 0xfc, 0x0b } },
	{ RX63_adc,		&b3_ld_rs_rd,		{ 0x06,  0xa0, 0x02 } },

	{ RX63_add,		&b1_imm4_rd,		{ 0x62 } },
	{ RX63_add,		&b1_ld_rs_rd,		{ 0x48 } },
	{ RX63_add,		&b2_mi_ld_rs_rd,	{ 0x06, 0x08 } },
	{ RX63_add_,	&b1_li_rs2_rd,		{ 0x70 } },
	{ RX63_add_,	&b2_rd_rs_rs2,		{ 0xff, 0x20 } },

	{ RX63_and,		&b1_imm4_rd,		{ 0x64 } },
	{ RX63_and,		&b2_li_rd,			{ 0x74, 0x20 } },
	{ RX63_and,		&b1_ld_rs_rd,		{ 0x50 } },
	{ RX63_and,		&b2_mi_ld_rs_rd,	{ 0x06, 0x10 } },
	{ RX63_and_,	&b2_rd_rs_rs2,		{ 0xff, 0x40 } },

	{ RX63_bclr,	&b2_ld_rd_imm3,		{ 0xf0, 0x08 } },
	{ RX63_bclr,	&b2_ld_rd_rs,		{ 0xfc, 0x64 } },
	{ RX63_bclr,	&b1_imm5_rd,		{ 0x7a } },

	{ RX63_b,		&b1_cd_dsp_s,		{ 0x10 } },
	{ RX63_b,		&b1_cd_dsp_b,		{ 0x20 } },
	{ RX63_b,		&b1_cd_dsp_w,		{ 0x3a } },

	{ RX63_bm,		&b2_imm3_ld_rd_cd,	{ 0xfc, 0xe0 } },
	{ RX63_bm,		&b2_imm5_cd_rd,		{ 0xfd, 0xe0 } },

	{ RX63_bnot,	&b3_imm3_ld_rd,		{ 0xfc, 0xe0, 0x0f } },
	{ RX63_bnot,	&b2_ld_rd_rs,		{ 0xfc, 0x6c } },
	{ RX63_bnot,	&b3_imm5_rd,		{ 0xfd, 0xe0, 0xf0 } },

	{ RX63_bra,		&b1_dsp,			{ 0x08 } },
	{ RX63_bra,		&b1_pcdsp8,			{ 0x2e } },
	{ RX63_bra,		&b1_pcdsp16,		{ 0x38 } },
	{ RX63_bra,		&b1_pcdsp24,		{ 0x04 } },
	{ RX63_bra,		&b2_rs,				{ 0x7f, 0x40 } },

	{ RX63_brk,		&b1_no_args,		{ 0x00 } },

	{ RX63_bset,	&b2_ld_rd_imm3,		{ 0xf0, 0x00 } },
	{ RX63_bset,	&b2_ld_rd_rs,		{ 0xfc, 0x60 } },
	{ RX63_bset,	&b1_imm5_rd,		{ 0x78 } },

	{ RX63_bsr,		&b1_pcdsp16,		{ 0x39 } },
	{ RX63_bsr,		&b1_pcdsp24,		{ 0x05 } },
	{ RX63_bsr,		&b2_rs,				{ 0x7f, 0x50 } },

	{ RX63_btst,	&b2_ld_rd_imm3,		{ 0xf4, 0x00 } },
	{ RX63_btst,	&b2_ld_rd_rs,		{ 0xfc, 0x68 } },
	{ RX63_btst,	&b1_imm5_rd,		{ 0x7c } },

	{ RX63_clrpsw,	&b2_cb,				{ 0x7f, 0xb0 } },

	{ RX63_cmp,		&b1_imm4_rd,		{ 0x61 } },
	{ RX63_cmp,		&b2_rs2_uimm8,		{ 0x75, 0x50 } },
	{ RX63_cmp,		&b2_li_rd,			{ 0x74, 0x00 } },
	{ RX63_cmp,		&b1_ld_rs_rd,		{ 0x44 } },
	{ RX63_cmp,		&b2_mi_ld_rs_rd,	{ 0x06, 0x04 } },

	{ RX63_div,		&b3_li_rd,			{ 0xfd, 0x70, 0x80 } },
	{ RX63_div,		&b2_ld_rs_rd_ub,	{ 0xfc, 0x20 } },
	{ RX63_div,		&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x08 } },

	{ RX63_divu,	&b3_li_rd,			{ 0xfd, 0x70, 0x90 } },
	{ RX63_divu,	&b2_ld_rs_rd_ub,	{ 0xfc, 0x24 } },
	{ RX63_divu,	&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x09 } },

	{ RX63_emul,	&b3_li_rd,			{ 0xfd, 0x70, 0x60 } },
	{ RX63_emul,	&b2_ld_rs_rd_ub,	{ 0xfc, 0x18 } },
	{ RX63_emul,	&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x06 } },

	{ RX63_emulu,	&b3_li_rd,			{ 0xfd, 0x70, 0x70 } },
	{ RX63_emulu,	&b2_ld_rs_rd_ub,	{ 0xfc, 0x1c } },
	{ RX63_emulu,	&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x07 } },

	{ RX63_fadd,	&b3_rd_imm32,		{ 0xfd, 0x72, 0x20 } },
	{ RX63_fadd,	&b2_ld_rs_rd_l,		{ 0xfc, 0x88 } },

	{ RX63_fcmp,	&b3_rd_imm32,		{ 0xfd, 0x72, 0x10 } },
	{ RX63_fcmp,	&b2_ld_rs_rd_l,		{ 0xfc, 0x84 } },

	{ RX63_fdiv,	&b3_rd_imm32,		{ 0xfd, 0x72, 0x40 } },
	{ RX63_fdiv,	&b2_ld_rs_rd_l,		{ 0xfc, 0x90 } },

	{ RX63_fmul,	&b3_rd_imm32,		{ 0xfd, 0x72, 0x30 } },
	{ RX63_fmul,	&b2_ld_rs_rd_l,		{ 0xfc, 0x8c } },

	{ RX63_fsub,	&b3_rd_imm32,		{ 0xfd, 0x72, 0x00 } },
	{ RX63_fsub,	&b2_ld_rs_rd_l,		{ 0xfc, 0x80 } },

	{ RX63_ftoi,	&b2_ld_rs_rd_l,		{ 0xfc, 0x94 } },

	{ RX63_int,		&b2_imm8,			{ 0x75, 0x60 } },

	{ RX63_itof,	&b2_ld_rs_rd_ub,	{ 0xfc, 0x44 } },
	{ RX63_itof,	&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x11 } },

	{ RX63_jmp,		&b2_rs_,			{ 0x7f, 0x00 } },
	{ RX63_jsr,		&b2_rs_,			{ 0x7f, 0x10 } },

	{ RX63_machi,	&b2_rs_rd,			{ 0xfd, 0x04 } },
	{ RX63_maclo,	&b2_rs_rd,			{ 0xfd, 0x05 } },

	{ RX63_max,		&b3_li_rd,			{ 0xfd, 0x70, 0x40 } },
	{ RX63_max,		&b2_ld_rs_rd_ub,	{ 0xfc, 0x10 } },
	{ RX63_max,		&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x04 } },

	{ RX63_min,		&b3_li_rd,			{ 0xfd, 0x70, 0x50 } },
	{ RX63_min,		&b2_ld_rs_rd_ub,	{ 0xfc, 0x14 } },
	{ RX63_min,		&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x05 } },

	{ RX63_mov,		&b1_sz_rd7_rs,		{ 0x80 } },
	{ RX63_mov,		&b1_sz_rs7_rd7,		{ 0x88 } },
	{ RX63_mov,		&b1_imm4_rd_,		{ 0x66 } },
	{ RX63_mov,		&b1_sz_rd,			{ 0x3c } },
	{ RX63_mov,		&b2_rs2_uimm8_,		{ 0x75, 0x40 } },
	{ RX63_mov,		&b2_rd_li,			{ 0xfb, 0x02 } },
	{ RX63_mov,		&b1_sz_rs_rd,		{ 0xcf } },
	{ RX63_mov,		&b1_ld_rd_li_sz,	{ 0xf8 } },
	{ RX63_mov,		&b1_sz_ld_rs_rd,	{ 0xcc } },
	{ RX63_mov,		&b2_sz_ri_rb_rd,	{ 0xfe, 0x40 } },
	{ RX63_mov,		&b1_sz_ld_rd_rs,	{ 0xc3 } },
	{ RX63_mov,		&b2_sz_ri_rb_rs,	{ 0xfe, 0x00 } },
	{ RX63_mov,		&b1_sz_ldd_lds_rs_rd, { 0xc0 } },
	{ RX63_mov,		&b2_ad_sz_rds,		{ 0xfd, 0x20 } },
	
	{ RX63_movu,	&b1_sz_rd7_rs7,		{ 0xb0 } },
	{ RX63_movu,	&b1_sz_ld_rs_rd_u,	{ 0x58 } },
	{ RX63_movu,	&b2_sz_ri_rb_rd_u,	{ 0xfe, 0xc0 } },
	{ RX63_movu,	&b2_ad_sz_rs_rd,	{ 0xfd, 0x30 } },

	{ RX63_mul,		&b1_imm4_rd,		{ 0x63 } },
	{ RX63_mul,		&b2_li_rd,			{ 0x74, 0x10 } },
	{ RX63_mul,		&b1_ld_rs_rd,		{ 0x4c } },
	{ RX63_mul,		&b2_mi_ld_rs_rd,	{ 0x06, 0x0c } },
	{ RX63_mul,		&b2_rd_rs_rs2,		{ 0xff, 0x30 } },

	{ RX63_mulhi,	&b2_rs_rd,			{ 0xfd, 0x00 } },
	{ RX63_mullo,	&b2_rs_rd,			{ 0xfd, 0x01 } },
	{ RX63_mvfachi,	&b3_rd,				{ 0xfd, 0x1f, 0x00 } },
	{ RX63_mvfacmi,	&b3_rd,				{ 0xfd, 0x1f, 0x20 } },
	{ RX63_mvfc,	&b2_cr_rd,			{ 0xfd, 0x6a } },
	{ RX63_mvtachi,	&b3_rd,				{ 0xfd, 0x17, 0x00 } },
	{ RX63_mvtaclo,	&b3_rd,				{ 0xfd, 0x17, 0x10 } },

	{ RX63_mvtc,	&b3_li_cr,			{ 0xfd, 0x73, 0x00 } },
	{ RX63_mvtc,	&b2_rs_cr,			{ 0xfd, 0x68 } },
	{ RX63_mvtipl,	&b3_imm4,			{ 0x75, 0x70, 0x00 } },

	{ RX63_neg,		&b2_rd,				{ 0x7e, 0x10 } },
	{ RX63_neg_,	&b2_rs_rd,			{ 0xfc, 0x07 } },
	{ RX63_nop,		&b1_no_args,		{ 0x03 } },
	
	{ RX63_not,		&b2_rd,				{ 0x7e, 0x00 } },
	{ RX63_not_,	&b2_rs_rd,			{ 0xfc, 0x3b } },

	{ RX63_or,		&b1_imm4_rd,		{ 0x65 } },
	{ RX63_or,		&b2_li_rd,			{ 0x74, 0x30 } },
	{ RX63_or,		&b1_ld_rs_rd,		{ 0x54 } },
	{ RX63_or,		&b2_mi_ld_rs_rd,	{ 0x06, 0x14 } },
	{ RX63_or_,		&b2_rd_rs_rs2,		{ 0xff, 0x50 } },

	{ RX63_pop,		&b2_rd,				{ 0x7e, 0xb0 } },
	{ RX63_popc,	&b2_cr,				{ 0x7e, 0xe0 } },
	{ RX63_popm,	&b1_rd_rd2,			{ 0x6f } },

	{ RX63_push,	&b2_sz_rs,			{ 0x7e, 0x80 } },
	{ RX63_push,	&b2_ld_rs_sz,		{ 0xf4, 0x08 } },
	{ RX63_pushc,	&b2_cr_,			{ 0x7e, 0xc0 } },
	{ RX63_pushm,	&b1_rd_rd2,			{ 0x6e } },

	{ RX63_racw,	&b3_imm1,			{ 0xfd, 0x18, 0x00 } },
	{ RX63_revl,	&b2_rs_rd,			{ 0xfd, 0x67 } },
	{ RX63_revw,	&b2_rs_rd,			{ 0xfd, 0x65 } },
	{ RX63_rmpa,	&b2_sz,				{ 0x7f, 0x8c } },
	{ RX63_rolc,	&b2_rd,				{ 0x7e, 0x50 } },
	{ RX63_rorc,	&b2_rd,				{ 0x7e, 0x40 } },
	{ RX63_rotl,	&b2_imm5_rd,		{ 0xfd, 0x6e } },
	{ RX63_rotl,	&b2_rs_rd,			{ 0xfd, 0x66 } },
	{ RX63_rotr,	&b2_imm5_rd,		{ 0xfd, 0x6c } },
	{ RX63_rotr,	&b2_rs_rd,			{ 0xfd, 0x64 } },
	{ RX63_round,	&b2_ld_rs_rd_l,		{ 0xfc, 0x98 } },

	{ RX63_rte,		&b2_no_args,		{ 0x7f, 0x95 } },
	{ RX63_rtfi,	&b2_no_args,		{ 0x7f, 0x94 } },
	{ RX63_rts,		&b1_no_args,		{ 0x02 } },
	{ RX63_rtsd,	&b1_uimm8,			{ 0x67 } },
	{ RX63_rtsd,	&b1_rd_rd2_imm8,	{ 0x3f } },
	{ RX63_sat,		&b2_rd,				{ 0x7e, 0x30 } },
	{ RX63_satr,	&b2_no_args,		{ 0x7f, 0x93 } },
	{ RX63_sbb,		&b2_rs_rd,			{ 0xfc, 0x03 } },
	{ RX63_sbb,		&b3_ld_rs_rd,		{ 0x06, 0xa0, 0x00 } },
	{ RX63_sc,		&b2_sz_ld_rd_cd,	{ 0xfc, 0xd0 } },
	{ RX63_scmpu,	&b2_no_args,		{ 0x7f, 0x83 } },
	{ RX63_setpsw,	&b2_cb,				{ 0x7f, 0xa0 } },

	{ RX63_shar,	&b1_imm5_rd,		{ 0x6a } },
	{ RX63_shar,	&b2_rs_rd,			{ 0xfd, 0x61} },
	{ RX63_shar_,	&b2_imm5_rs2_rd,	{ 0xfd, 0xa0} },

	{ RX63_shll,	&b1_imm5_rd,		{ 0x6c } },
	{ RX63_shll,	&b2_rs_rd,			{ 0xfd, 0x62} },
	{ RX63_shll_,	&b2_imm5_rs2_rd,	{ 0xfd, 0xc0} },

	{ RX63_shlr,	&b1_imm5_rd,		{ 0x68 } },
	{ RX63_shlr,	&b2_rs_rd,			{ 0xfd, 0x60} },
	{ RX63_shlr_,	&b2_imm5_rs2_rd,	{ 0xfd, 0x80} },

	{ RX63_smovb,	&b2_no_args,		{ 0x7f, 0x8b } },
	{ RX63_smovf,	&b2_no_args,		{ 0x7f, 0x8f } },
	{ RX63_smovu,	&b2_no_args,		{ 0x7f, 0x87 } },

	{ RX63_sstr,	&b2_sz,				{ 0x7f, 0x88 } },

	{ RX63_stnz,	&b3_li_rd,			{ 0xfd, 0x70, 0xf0 } },
	{ RX63_stz,		&b3_li_rd,			{ 0xfd, 0x70, 0xe0 } },

	{ RX63_sub,		&b1_imm4_rd,		{ 0x60 } },
	{ RX63_sub,		&b1_ld_rs_rd,		{ 0x40 } },
	{ RX63_sub,		&b2_mi_ld_rs_rd,	{ 0x06, 0x00 } },
	{ RX63_sub_,	&b2_rd_rs_rs2,		{ 0xff, 0x00 } },

	{ RX63_suntil,	&b2_sz,				{ 0x7f, 0x80 } },
	{ RX63_swhile,	&b2_sz,				{ 0x7f, 0x84 } },

	{ RX63_tst,		&b3_li_rd,			{ 0xfd, 0x70, 0xc0 } },
	{ RX63_tst,		&b2_ld_rs_rd_ub,	{ 0xfc, 0x30 } },
	{ RX63_tst,		&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x0c } },

	{ RX63_wait,	&b2_no_args,		{ 0x7f, 0x96 } },

	{ RX63_xchg,	&b2_ld_rs_rd_ub,	{ 0xfc, 0x40 } },
	{ RX63_xchg,	&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x10 } },

	{ RX63_xor,		&b3_li_rd,			{ 0xfd, 0x70, 0xd0 } },
	{ RX63_xor,		&b2_ld_rs_rd_ub,	{ 0xfc, 0x34 } },
	{ RX63_xor,		&b3_mi_ld_rs_rd,	{ 0x06, 0x20, 0x0d } },
};

enum parse_result_t 
{
	parse_result_tnomatch = 0,
	parse_result_tmatch = 1,
	parse_result_tparty_match = 2
};

parse_result_t parse_opcode(int stage,insn_t *insn, uchar opcode, void (*function)(insn_t*))
{
	bool function_found = false;
	int i;
	for(i = 0; i < qnumber( function_descs ); i++)
	{
		if ( function_descs[i].function == function)
		{
			function_found = true;
			break;
		}
	}

	if (!function_found || function_descs[i].opcode_count < stage+1)
		return parse_result_tnomatch;

	uchar code = get_hl8( insn->ea + stage);

	if ((code & function_descs[i].mask[stage]) != opcode)
		return parse_result_tnomatch;

	return stage + 1 == function_descs[i].opcode_count ? parse_result_tmatch : parse_result_tparty_match;
}

int idaapi ana( insn_t *_insn ) {
	insn_t &insn = *_insn;
	bool nomatches [ qnumber(opcodes) ]= {false};
	bool fl_exit = false;
	int stage = 0;
	insn.size = 0;
	insn.auxpref = (uint32)condition_tnone;

	while(stage < 3 && !fl_exit)
	{
		for (int i = 0; i < qnumber(opcodes); i++)
		{
			if (nomatches[i])
				continue;

			parse_result_t res = parse_opcode(stage, &insn, opcodes[i].opcodes[stage], opcodes[i].function);

			if (res == parse_result_tmatch)
			{
				insn.itype = opcodes[i].instruction;
				opcodes[i].function(&insn);
				if (insn.size != 0)
				{
					fl_exit = true;;
					break;
				}
				else
				{
					nomatches[i] = true;
				}
			}
			else if (res == parse_result_tnomatch)
			{
				nomatches[i] = true;
			}
		}	

		stage++;
	}

	return insn.size;
}
