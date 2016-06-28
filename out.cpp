#include "rx63.hpp"

inline void out_reg(int rgnum)
{
	out_register(ph.regNames[rgnum]);
}

void idaapi header(void)
{
	gen_header(GH_PRINT_PROC_ASM_AND_BYTESEX);
}

void idaapi footer(void)
{
	char buf[MAXSTR];

	MakeNull();

	tag_addstr(buf, buf+sizeof(buf), COLOR_ASMDIR, ash.end);
	MakeLine(buf, inf.indent);

	gen_cmt_line("end of file");
}

void idaapi segstart(ea_t ea)
{
	segment_t *sarea = getseg(ea);

	char sname[MAXNAMELEN];
	get_segm_name(sarea, sname, sizeof(sname));

	gen_cmt_line(COLSTR("segment %s", SCOLOR_AUTOCMT), sname);

	ea_t org = ea - get_segm_base(sarea);
	if (org != 0)
	{
		char buf[MAX_NUMBUF];
		btoa(buf, sizeof(buf), org);
		gen_cmt_line("%s %s", ash.origin, buf);
	}
}

void idaapi segend(ea_t ea)
{
	char sname[MAXNAMELEN];
	get_segm_name(getseg(ea-1), sname, sizeof(sname));
	gen_cmt_line("end of '%s'", sname);
}

void idaapi out(void)
{
	char buf[MAXSTR];
	init_output_buffer(buf, sizeof(buf));     // setup the output pointer

	OutMnem();

	out_one_operand(0);

	if ( cmd.Op2.type != o_void )
	{
		out_symbol(',');
		OutChar(' ');
		out_one_operand(1);
	}

	if ( cmd.Op3.type != o_void )
	{
		out_symbol(',');
		OutChar(' ');
		out_one_operand(2);
	}

	term_output_buffer();
	gl_comm = 1;
	MakeLine(buf);
}

bool idaapi outop(op_t &x)
{
	switch ( x.type )
	{
		// register
		case o_reg:
			out_reg(x.reg);
			break;

		// immediate
		case o_imm:
			out_symbol('#');
			OutValue(x, OOFW_IMM|OOF_SIGNED);
			break;

		// displ @(imm, reg)
		case o_displ:
			out_symbol('@');
			out_symbol('(');
			OutValue(x, OOF_SIGNED | OOF_ADDR | OOFW_32);
			out_symbol(',');
			OutChar(' ');
			out_reg(x.reg);
			out_symbol(')');
			break;

		// address
		case o_near:
			if ( !out_name_expr(x, toEA(cmd.cs, x.addr), x.addr) )
				OutValue(x, OOF_ADDR | OOF_NUMBER | OOFS_NOSIGN | OOFW_32);
			break;
	}
	return 1;
}
