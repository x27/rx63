#include "rx63.hpp"

//#define SHOWHEXCODES

//----------------------------------------------------------------------
class out_rx63_t : public outctx_t {
	void outreg(int r) { this->out_register(ph.reg_names[r]); }

public:
	bool out_operand(const op_t &x);
	void out_insn();
};

DECLARE_OUT_FUNCS_WITHOUT_OUTMNEM(out_rx63_t)

//----------------------------------------------------------------------

static const char * const ccode[] =
{
	"eq", "ne", "c", "nc", "gtu", "leu", "pz", "n", "ge", "lt", "gt", "le", "o", "no", "bra", ""
};

static const char * const cmemex[] =
{
	".b", ".w", ".l", ".uw", ".ub", ".s", ".a"
};

static const char * const cregs[] =
{
	"psw", "pc", "usp", "fpsw", "", "", "", "", "bpsw", "bpc", "isp", "fintv", "intb", "", "", ""
};

void idaapi rx63_header(outctx_t &ctx)
{
	ctx.gen_header(GH_PRINT_PROC_ASM_AND_BYTESEX);
}

void idaapi rx63_footer(outctx_t &ctx)
{
	char buf[MAXSTR];

	ctx.gen_empty_line();

	//tag_addstr(buf, buf+sizeof(buf), COLOR_ASMDIR, ash.end);
	ctx.flush_buf(buf, inf.indent);

	ctx.gen_cmt_line("end of file");
}

void idaapi rx63_segstart(outctx_t &ctx, ea_t ea)
{
	segment_t *sarea = getseg(ea);

	qstring sname[MAXNAMELEN];
	get_visible_segm_name(sname, sarea);

	ctx.gen_cmt_line(COLSTR("segment %s", SCOLOR_AUTOCMT), (char*)sname);

	ea_t org = ea - get_segm_base(sarea);
	if (org != 0)
	{
		char buf[MAX_NUMBUF];
		btoa(buf, sizeof(buf), org);
		ctx.gen_cmt_line("%s %s", ash.origin, buf);
	}
}

void idaapi rx63_segend(outctx_t &ctx, ea_t ea)
{
	qstring sname[MAXNAMELEN];
	get_visible_segm_name(sname,getseg(ea-1));
	ctx.gen_cmt_line("end of '%s'", (char*)sname);
}

inline ea_t real_address( ea_t ea )
{
	return ( ea >> 2 << 2 ) + 3 - ( ea & 3 );
}

bool is_need_add_empty_line(insn_t insn)
{
	const int empty_line_ops[] = { RX63_smovf, RX63_jsr, RX63_b, RX63_bsr  };

	if ( get_first_cref_from (insn.ea + insn.size ) != BADADDR)
		return false;

	for(int i=0; i< qnumber( empty_line_ops ); i++)
	{
		if( empty_line_ops[i] == insn.itype )
		{
			return true;
		}
	}
	return false;
}

bool out_rx63_t::out_operand(const op_t &x)
{
	switch ( x.type )
	{
		// register
		case o_reg:
			this->outreg(x.reg);
			break;

		// immediate
		case o_imm:
			this->out_symbol('#');
			this->out_value(x, OOFW_IMM);
			break;

		// displ dsp[reg]
		case o_displ:

			if (x.ld == ld_treg)
			{
				this->outreg(x.reg);
			}
			else {
				if (x.value != 0)
					this->out_value(x, OOFS_NOSIGN);
				this->out_symbol('[');
				this->outreg(x.reg);
				this->out_symbol(']');

				// show memex?
				if ((x.memex & 0x10) == 0)
				{
					this->out_symbol('.');
					switch(x.memex)
					{
						case memex_tl:
							this->out_symbol('l');
							break;
						case memex_tuw:
							this->out_symbol('u');
							this->out_symbol('w');
							break;
						case memex_tb:
							this->out_symbol('b');
							break;
						case memex_tw:
							this->out_symbol('w');
							break;
						case memex_tub:
							this->out_symbol('u');
							this->out_symbol('b');
							break;
					}
				}
			}
			break;

		// address
		case o_near:
                        {
                        ea_t ean = to_ea(insn.cs, x.addr);
                        if( !out_name_expr(x, ean, x.addr))
			this->out_value(x, OOF_ADDR | OOF_NUMBER | OOFS_NOSIGN | OOFW_32);
			break;
                        }

		case o_flag:
			switch(x.value & 0xf)
			{
				case cflag_tflag_c:
					this->out_symbol('c');
					break;
				case cflag_tflag_z:
					this->out_symbol('z');
					break;
				case cflag_tflag_s:
					this->out_symbol('s');
					break;
				case cflag_tflag_o:
					this->out_symbol('o');
					break;
				case cflag_tflag_i:
					this->out_symbol('i');
					break;
				case cflag_tflag_u:
					this->out_symbol('u');
					break;
			}
			break;
		case o_creg:
			this->outreg(x.value & 0xf);
			break;

		case o_phrase:
			switch(x.phrase_type)
			{
				case rx63_phrasesf_r_r:
					this->out_symbol('[');
					this->outreg(x.value&0xf);
					this->out_symbol(',');
					this->out_symbol(' ');
					this->outreg(x.reg);
					this->out_symbol(']');
					break;
				case rx63_phrasesf_r_plus:
					this->out_symbol('[');
					this->outreg(x.reg);
					this->out_symbol('+');
					this->out_symbol(']');
					break;
				case rx63_phrasesf_r_minus:
					this->out_symbol('[');
					this->out_symbol('-');
					this->outreg(x.reg);
					this->out_symbol(']');
					break;
				case rx63_phrasesf_r_2_r:
					this->outreg(x.value&0xf);
					this->out_symbol('-');
					this->outreg(x.reg);
					break;
			}
			break;

	}
	return 1;
}

void out_rx63_t::out_insn()
{
	char buf[MAXSTR];

#ifdef SHOWHEXCODES
	char bin[20];
	bin[0] = '\0';
	for(int i=0; i<insn.size; i++) {
		char temp[10];
		qsnprintf(temp, sizeof(temp), "%02X", get_wide_byte( real_address( insn.ea + i)));
		qstrncat(bin, temp, sizeof(bin));
	}
	qstrncat(bin, " ", sizeof(bin));
	out_line(bin);
#endif

	char cond_postfix[6];
	cond_postfix[0] = '\0';

	if ((insn.auxpref & 0xf) != condition_tnone)
	{
		qstrncat(cond_postfix, ccode[insn.auxpref & 0xf], sizeof(cond_postfix));
	}

	if ((insn.auxpref >> 4) != 0)
	{
		qstrncat(cond_postfix, cmemex[(insn.auxpref >> 4) - 1], sizeof(cond_postfix));
	}

	out_mnem(8, cond_postfix);

	out_one_operand(0);

	if ( insn.Op2.type != o_void )
	{
		this->out_symbol(',');
		this->out_char(' ');
		out_one_operand(1);
	}

	if ( insn.Op3.type != o_void )
	{
		this->out_symbol(',');
		this->out_char(' ');
		out_one_operand(2);
	}

	flush_outbuf();
	//this->flush_buf(buf); XXX why this line? just prints garbage...

	//if ( is_need_add_empty_line() )
	//	MakeLine("");
}

