#include "rx63.hpp"

enum simm_t 
{
	imm32	= 0,
	simm8	= 1,
	simm16	= 2,
	simm24	= 3,
};

enum memex_t
{
	b = 0,
	w = 1,
	l = 2,
	uw = 3
};

enum ld_t
{
	in_reg = 0, // @reg
	dsp8 = 1,
	dsp16 = 2,
	reg = 3	// reg
};

void set_reg(op_t &op, uint16 reg) 
{
	op.type = o_reg;
	op.reg = reg;
	op.dtyp = dt_word;
}

inline ea_t real_address( ea_t ea )
{
	return ( ea >> 2 << 2 ) + 3 - ( ea & 3 );
}

static uchar get_hl8( ea_t ea)
{
	uint save_ea = cmd.ea;
	uint save_size = cmd.size;
	cmd.ea = ea; //real_address( ea );
	uchar code = ua_next_byte();	
	cmd.ea = save_ea;
	cmd.size = save_size;
	return code;
}

static uint get_hl32( ea_t ea )
{
	uint res = 0;
	for( int i=3; i>=0; i-- )
	{
		res = res << 8;
		res = res | get_hl8( ea + i );
	}
	return res;
}

static uint get_hl24( ea_t ea )
{
	uint res = 0;
	for( int i=2; i>=0; i-- )
	{
		res = res << 8;
		res = res | get_hl8( ea + i );
	}
	return res;
}

static ushort get_hl16( ea_t ea )
{
	return (get_hl8( ea + 1) << 8 ) | get_hl8 ( ea );
}

static int set_imm(op_t &op, int imm, ea_t ea) 
{
	int size = 0;
	op.type = o_imm;
	switch(imm)
	{
		case imm32:
			size = 4;
			op.value = get_hl32( ea );
			op.dtyp = dt_dword;
			break;
		case simm8:
			size = 1;
			op.value = get_hl8( ea );
			op.dtyp = dt_byte;
			break;
		case simm16:
			size = 2;
			op.value = get_hl16( ea );
			op.dtyp = dt_word;
			break;
		case simm24:
			size = 3;
			op.value = get_hl24( ea );
			op.dtyp = dt_3byte;
			break;
	}
	return size;
}

void b2_rd()
{
	uchar data = get_hl8( cmd.ea + 1 );
	cmd.size = 2;
	set_reg(cmd.Op1, data & 0x0f);
}

void b2_rs_rd()
{
	cmd.size = 3;

	uchar data = get_hl8( cmd.ea + 2 );

	set_reg(cmd.Op1, data >> 4);
	set_reg(cmd.Op2, data & 0xf);
}

void b3_li_rd()
{
	uchar li = get_hl8( cmd.ea + 1 ) & 3;

	cmd.size = 3 + set_imm(cmd.Op1, li, cmd.ea + 3);
	set_reg(cmd.Op2, get_hl8( cmd.ea + 2 ) & 0x0f);
}

int get_scale(int memex)
{
	int scale = 1;
	if (memex == memex_t::w || memex_t::uw)
		scale = 2;
	else if (memex == memex_t::l)
		scale = 4;
	return scale;
}

void b3_ld_rs_rd()
{
	uchar data0 = get_hl8( cmd.ea + 1 );
	uchar data1 = get_hl8( cmd.ea + 3 );

	cmd.Op1.memex = data0 >> 6;
	cmd.Op1.ld = data0 & 3;
	set_reg(cmd.Op1, data1 >> 4);
	set_reg(cmd.Op2, data1 & 0xf);

	cmd.size = 4;

	if (cmd.Op1.ld == ld_t::dsp8)
	{
		cmd.Op1.value = get_scale(cmd.Op1.memex) * get_hl8( cmd.ea + 4);
		cmd.size ++;
	}
	else if (cmd.Op1.ld == ld_t::dsp16)
	{
		cmd.Op1.value = get_scale(cmd.Op1.memex) * get_hl16( cmd.ea + 4);
		cmd.size += 2;
	}
}

void b1_imm4_rd()
{
	uchar data = get_hl8( cmd.ea + 1 );

	cmd.size = 2;
	cmd.Op1.type = o_imm;
	cmd.Op1.value = data >> 4;
	set_reg(cmd.Op2, data & 0xf);
}

void b1_ld_rs_rd()
{
	uchar data0 = get_hl8( cmd.ea );
	uchar data1 = get_hl8( cmd.ea + 1 );

	cmd.size = 2;

	cmd.Op1.ld = data0 & 3;
	set_reg(cmd.Op1, data1 >> 4);
	set_reg(cmd.Op2, data1 & 0xf);

	if (cmd.Op1.ld == ld_t::dsp8)
	{
		cmd.Op1.value = get_hl8( cmd.ea + 2 );
		cmd.size ++;
	}
	else if (cmd.Op1.ld == ld_t::dsp16)
	{
		cmd.Op1.value = get_hl16( cmd.ea + 2 );
		cmd.size += 2;
	}
}

void b2_mi_ld_rs_rd()
{
	uchar data0 = get_hl8( cmd.ea + 1 );
	uchar data1 = get_hl8( cmd.ea + 2 );

	cmd.Op1.memex = data0 >> 6;
	cmd.Op1.ld = data0 & 3;
	
	set_reg(cmd.Op1, data1 >> 4);
	set_reg(cmd.Op2, data1 & 0xf);

	cmd.size = 3;

	if (cmd.Op1.ld == ld_t::dsp8)
	{
		cmd.Op1.value = get_scale(cmd.Op1.memex) * get_hl8( cmd.ea + 3);
		cmd.size ++;
	}
	else if (cmd.Op1.ld == ld_t::dsp16)
	{
		cmd.Op1.value = get_scale(cmd.Op1.memex) * get_hl16( cmd.ea + 3);
		cmd.size += 2;
	}
}

void b1_li_rs2_rd()
{
	uchar li = get_hl8( cmd.ea ) & 3;
	uchar data = get_hl8 ( cmd.ea + 1 );

	cmd.size = 2 + set_imm(cmd.Op1, li, cmd.ea + 2);
	set_reg(cmd.Op2, data >> 4);
	set_reg(cmd.Op3, data && 0xf );
}

void b2_rd_rs_rs2()
{
	uchar data0 = get_hl8( cmd.ea + 1 );
	uchar data1 = get_hl8( cmd.ea + 2 );

	set_reg(cmd.Op1, data1 >> 4);
	set_reg(cmd.Op2, data1 && 0xf);
	set_reg(cmd.Op3, data0 && 0xf );

	cmd.size = 3;
}

struct opcode_t
{
	int instruction;
	void (*function)();
	uchar opcodes[4];
};

struct function_desc_t
{
	void (*function)();
	int opcode_count;
	uchar mask[4];
};

void b1_imm5_rd()
{
}

void b1_no_args()
{
}

void b1_pc_dsp16()
{
}

void b1_pc_dsp24()
{
}

void b1_rd_rd2_imm8()
{
}

void b1_rs_rs2()
{
}

void b1_uimm8()
{
}

void b2_cb()
{
}

void b2_cr()
{
}

void b2_cr_rd()
{
}

void b2_imm5_rd()
{
}

void b2_imm5_rs2_rd()
{
}

void b2_imm8()
{
}

void b2_ld_rd_imm3()
{
}

void b2_ld_rd_rs()
{
}

void b2_ld_rs_rd()
{
}

void b2_ld_rs_sz()
{
}

void b2_li_rd()
{
}

void b2_no_args()
{
}

void b2_rs2_uimm8()
{
}

void b2_rs_cr()
{
}

void b2_rs()
{
}

void b2_sz()
{
}

void b2_sz_rs()
{
}

void b3_imm3_ld_rd()
{
}

void b3_imm5_rd()
{
}

void b3_li_cr()
{
}

void b3_mi_ld_rs_rd()
{
}

void b3_rd_imm32()
{
}

void b3_rd()
{
}

static const struct function_desc_t function_descs[] = 
{
	{ &b1_imm4_rd,		1, { 0xff } },
	{ &b1_imm5_rd,		1, { 0xfe } },
	{ &b1_ld_rs_rd,		1, { 0xfc } },
	{ &b1_li_rs2_rd,	1, { 0xfc } },
	{ &b1_no_args,		1, { 0xff } },
	{ &b1_pc_dsp16,		1, { 0xff } },
	{ &b1_pc_dsp24,		1, { 0xff } },
	{ &b1_rd_rd2_imm8,	1, { 0xff } },
	{ &b1_rs_rs2,		1, { 0xff } },
	{ &b1_uimm8,		1, { 0xff } },
	{ &b2_cb,			2, { 0xff, 0xf0 } },
	{ &b2_cr,			2, { 0xff, 0xf0 } },
	{ &b2_cr_rd,		2, { 0xff, 0xff } },
	{ &b2_imm5_rd,		2, { 0xff, 0xfe } },
	{ &b2_imm5_rs2_rd,	2, { 0xff, 0xe0 } },
	{ &b2_imm8,			2, { 0xff, 0xff } },
	{ &b2_ld_rd_imm3,	2, { 0xfc, 0x08 } },
	{ &b2_ld_rd_rs,		2, { 0xff, 0xfc } },
	{ &b2_ld_rs_rd,		2, { 0xff, 0xfc } },
	{ &b2_ld_rs_sz,		2, { 0xfc, 0x0c } },
	{ &b2_li_rd,		2, { 0xfc, 0xf0 } },
	{ &b2_mi_ld_rs_rd,	2, { 0xff, 0x3c } },
	{ &b2_no_args,		2, { 0xff, 0xff } },
	{ &b2_rd,			2, { 0xff, 0xf0 } },
	{ &b2_rd_rs_rs2,	2, { 0xff, 0xf0 } }, 
	{ &b2_rs2_uimm8,	2, { 0xff, 0xf0 } },
	{ &b2_rs_cr,		2, { 0xff, 0xff } },
	{ &b2_rs,			2, { 0xff, 0xf0 } },
	{ &b2_rs_rd,		2, { 0xff, 0xff } },
	{ &b2_sz,			2, { 0xff, 0xfc } },
	{ &b2_sz_rs,		2, { 0xff, 0xc0 } },
	{ &b3_imm3_ld_rd,	3, { 0xff, 0xe0, 0x0f } },
	{ &b3_imm5_rd,		3, { 0xff, 0xe0, 0xf0 } },
	{ &b3_ld_rs_rd,		3, { 0xff, 0xfc, 0xff } },
	{ &b3_li_cr,		3, { 0xff, 0xf3, 0xf0 } },
	{ &b3_li_rd,		3, { 0xff, 0xf3, 0xf0 } },
	{ &b3_mi_ld_rs_rd,	3, { 0xff, 0x3c, 0xff } },
	{ &b3_rd_imm32,		3, { 0xff, 0xff, 0xf0 } },
	{ &b3_rd,			3, { 0xff, 0xff, 0xf0 } }
};

static const struct opcode_t opcodes[] = {
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
};

enum parse_result_t 
{
	nomatch = 0,
	match = 1,
	party_match = 2
};

parse_result_t parse_opcode(int stage, uchar code, void (*function))
{
	bool function_found = false;
	int i;
	for(i=0; i < qnumber( function_descs ); i++)
	{
		if ( function_descs[i].function == function)
		{
			function_found = true;
			break;
		}
	}

	if ( !function_found )
		return parse_result_t::nomatch;

}

int idaapi ana( void )
{
	bool nomatches [ qnumber(opcodes) ]= {false};
	bool fl_exit = false;
	int opcode_byte_count = 0;
	cmd.size = 0;

	while(opcode_byte_count < 3 && !fl_exit)
	{
		uchar code = get_hl8( cmd.ea + opcode_byte_count);
		for (int i = 0; i < qnumber(opcodes); i++)
		{
			if (nomatches[i])
				continue;
	
			parse_result_t res = parse_opcode(opcode_byte_count, code, opcodes[i].function);

			if (res == parse_result_t::match)
			{
				opcodes[i].function();
				fl_exit = true;;
				break;
			}
			else if (res == parse_result_t::nomatch)
			{
				nomatches[i] = true;
			}
		}	

		opcode_byte_count++;
	}

	return cmd.size;
}
