#include "rx63.hpp"
#include <diskio.hpp>
#include "notify_codes.hpp"
#include <ieee.h>

netnode helper;

static const char *const reg_names[] =
{
	"sp"/*"r0"*/, "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"isp", "usp", "intb", "pc", "psw", "bpc", "bpsw", "fintv", "fpsw",
	"cs", "ds",
};

static qstring device;
static ioports_t ports;

#include "../iocommon.cpp"
static ssize_t idaapi notify(void *, int msgid, va_list va)
{
	outctx_t *ctx;
	ea_t *ea;
	op_t *op;

	switch (msgid) {
		case processor_t::ev_init:
			helper.create("$ RX63");
			return 1;

		case processor_t::ev_newfile:
			set_device_name(device.c_str(), IORESP_AREA);
			return 1;

		case processor_t::ev_out_header:
			ctx = va_arg(va, outctx_t*);
			rx63_header(*ctx);
			return 1;

		case processor_t::ev_out_footer:
			ctx = va_arg(va, outctx_t*);
			rx63_footer(*ctx);
			return 1;

		case processor_t::ev_out_segstart:
			ctx = va_arg(va, outctx_t*);
			ea = va_arg(va, ea_t*);
			rx63_segstart(*ctx, *ea);
			return 1;

		case processor_t::ev_out_segend:
			ctx = va_arg(va, outctx_t*);
			ea = va_arg(va, ea_t*);
			rx63_segend(*ctx, *ea);
			return 1;

		case processor_t::ev_ana_insn:
			{
				insn_t *insn = va_arg(va, insn_t *);
				return ana(insn);
			}

		case processor_t::ev_emu_insn:
			{
				const insn_t *insn = va_arg(va, const insn_t *);
				return emu(insn)?1:-1;
			}

		case processor_t::ev_out_insn:
			ctx = va_arg(va, outctx_t*);
			out_insn(*ctx);
			return 1;

		case processor_t::ev_out_operand:
			ctx = va_arg(va, outctx_t*);
			op = va_arg(va, op_t*);
			return out_opnd(*ctx, *op)?1:-1;

		case rx63_module_t::ev_set_machine_type:
			{
				int subarch = va_arg(va, int);
				bool imageFile = va_arg(va, int);
				bool nonBinary = true;
				return 0;
			}

		default:
			break;
	}

	return 0;
}

//--------------------------------------------------------------------------
static asm_t rx63_asm =
{
	AS_COLON  |AS_ASCIIZ | AS_ASCIIC | AS_1TEXT,
	0,
	"Renesas RX family assembler",
	0,            // help screen number
	NULL,         // header
	NULL,         // origin
	".end",       // end

	";",          // comment string
	'"',          // string delimiter
	'\'',         // char delimiter (no char consts)
	"\\\"'",      // special symbols in char and string constants

	".string",    // ascii string directive
	".byte",      // byte directive
	".word",      // word directive
	".dword",     // dword  (4 bytes)
	NULL,         // qword  (8 bytes)
	NULL,         // oword  (16 bytes)
	".float",     // float  (4 bytes)
	".double",    // double (8 bytes)
	NULL,         // tbyte  (10/12 bytes)
	NULL,         // packed decimal real
	NULL,         // arrays (#h,#d,#v,#s(...)
	".block %s",  // uninited arrays
	".equ",       // Equ
	NULL,         // seg prefix
	"$",          // current IP
	NULL,			// func_header
	NULL,			// func_footer
	".global",		// public
	NULL,			// weak
	NULL,			// extrn
	NULL,			// comm
	NULL,			// get_type_name
	NULL,			// align
	'(', ')',		// lbrace, rbrace
	"%",			// mod
	"&",			// and
	"|",			// or
	"^",			// xor
	"!",			// not
	"<<",			// shl
	">>",			// shr
	NULL,			// sizeof
};

static const asm_t *const asms[] = { &rx63_asm, NULL };

//--------------------------------------------------------------------------

#define FAMILY "Renesas RX63 series:"
static const char *const shnames[] = { "RX63", NULL };
static const char *const lnames[]  = { "Renesas RX63 MCU", NULL };

static const uchar rtscode[]  =     { 0x2 };        // rts
static const uchar rtsdcode1[] =    { 0x67 };       // rtsd
static const uchar rtsdcode2[] =    { 0x4f };       // rtsd
static const uchar rtecode[] =      { 0x7f, 0x95 }; // rte
static const uchar rtfcode[] =      { 0x7f, 0x94 }; // rtf

static const bytes_t retcodes[] =
{
	{ sizeof(rtscode),      rtscode },
	{ sizeof(rtsdcode1),    rtsdcode1 },
	{ sizeof(rtsdcode2),    rtsdcode2 },
	{ sizeof(rtecode),      rtecode },
	{ sizeof(rtfcode),      rtfcode },
	{ 0,                    NULL }
};

processor_t LPH =
{
	IDP_INTERFACE_VERSION,  // version
	PLFM_RX63,              // id
	PRN_HEX |
	PR_USE32 |
	PR_BINMEM |
	PR_RNAMESOK |
	PR_DEFSEG32,            // flag
	0,                      // flag2
	8,                      // 8 bits in a byte for code segments
	8,                      // 8 bits in a byte for other segments

	shnames,                // short processor names (null term)
	lnames,                 // long processor names (null term)
	asms,                   // array of enabled assemblers
	notify,                 // Various messages:

	reg_names,              // Register names
	qnumber(reg_names),     // Number of registers

	r_cs,r_ds,              // reg_{first|last}_sreg
	0,                      // size of a segment register
	r_cs,r_ds,              // reg_{code|data}_sreg

	NULL,                   // No known code start sequences
	retcodes,

	0, RX63_last,
	Instructions,

	0,                      // int tbyte_size;  -- doesn't exist
	{ 0, 7, 15, 0 },        // char real_width[4];
	RX63_rte,               // Icode of return instruction. It is ok to give any of possible return instructions
	NULL,                   // unused_slot
};
