#include "rx63.hpp"
#include <diskio.hpp>

netnode helper;

static const char *const reg_names[] =
{
	"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"isp", "usp", "intb", "pc", "psw", "bpc", "bpsw", "fintv", "fpsw", 
	"cs", "ds",
};

char device[MAXSTR] = "RX63";
static size_t numports = 0;
static ioport_t *ports = NULL;

#include "../iocommon.cpp"

static int idaapi notify(processor_t::idp_notify msgid, ...)
{
	va_list va;
	va_start(va, msgid);

	int code = invoke_callbacks(HT_IDP, msgid, va);
	if (code)
		return code;

	switch (msgid)
	{
		case processor_t::init:
			helper.create("$ RX63");
			// little endian
			inf.mf = 0;
			break;

		case processor_t::newfile:
			set_device_name(device, IORESP_AREA);
			break;

		default:
			break;
	}

	va_end(va);
	return(1);
}

//--------------------------------------------------------------------------
static const asm_t rx63_asm =
{
	AS_COLON  |AS_ASCIIZ | AS_ASCIIC | AS_1TEXT,
	0,
	"Renesas RX family assembler",
	0,
	NULL,
	NULL,
	NULL,
	".end",

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
	NULL, 
	NULL, 
	NULL,
	NULL,
	"$",
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

#define FAMILY "_Renesas RX63 series:"
static const char *const shnames[] = { "RX63", NULL };
static const char *const lnames[]  = { FAMILY"_Renesas RX63 MCU", NULL };

static const uchar rtscode[]  =		{ 0x2 };		// rts
static const uchar rtsdcode1[] =	{ 0x67 };		// rtsd
static const uchar rtsdcode2[] =	{ 0x4f };		// rtsd
static const uchar rtecode[] =		{ 0x7f, 0x95 }; // rte
static const uchar rtfcode[] =		{ 0x7f, 0x94 }; // rtf

static const bytes_t retcodes[] =
{
	{ sizeof(rtscode),		rtscode },
	{ sizeof(rtsdcode1),	rtsdcode1 },
	{ sizeof(rtsdcode2),	rtsdcode2 },
	{ sizeof(rtecode),		rtecode },
	{ sizeof(rtfcode),		rtfcode },
	{ 0,					NULL }
};

processor_t LPH =
{
	IDP_INTERFACE_VERSION,	// version
	PLFM_RX63,				// id
	PRN_HEX |
	PR_USE32 |
	PR_BINMEM |
	PR_RNAMESOK |
	PR_DEFSEG32,
	8,						// 8 bits in a byte for code segments
	8,						// 8 bits in a byte for other segments

	shnames,				// short processor names (null term)
	lnames,					// long processor names (null term)
	asms,					// array of enabled assemblers
	notify,					// Various messages:
	header,					// produce start of text file
	footer,					// produce end of text file

	segstart,				// produce start of segment
	segend,					// produce end of segment

	NULL,

	ana,
	emu,

	out,
	outop,

	intel_data,
	NULL,					// compare operands
	NULL,					// can have type

	qnumber(reg_names),		// Number of registers
	reg_names,				// Register names
	NULL,					// get abstract register

	0,						// Number of register files
	NULL,					// Register file names
	NULL,					// Register descriptions
	NULL,					// Pointer to CPU registers

	r_cs,r_ds,
	0,						// size of a segment register
	r_cs,r_ds,

	NULL,					// No known code start sequences
	retcodes,

	0, RX63_last,
	Instructions,
	NULL,					// int  (*is_far_jump)(int icode);
	NULL,			        // Translation function for offsets
	0,						// int tbyte_size;  -- doesn't exist
	NULL,					// int (*realcvt)(void *m, ushort *e, ushort swt);
	{ 0, 0, 0, 0 },       // char real_width[4];
						  // number of symbols after decimal point
						  // 2byte float (0-does not exist)
						  // normal float
						  // normal double
						  // long double
	NULL,                 // int (*is_switch)(switch_info_t *si);
	NULL,                 // int32 (*gen_map_file)(FILE *fp);
	NULL,                 // ea_t (*extract_address)(ea_t ea,const char *string,int x);
	NULL,                 // int (*is_sp_based)(op_t &x); -- always, so leave it NULL
	NULL,                 // int (*create_func_frame)(func_t *pfn);
	NULL,                 // int (*get_frame_retsize(func_t *pfn)
	NULL,                 // void (*gen_stkvar_def)(char *buf,const member_t *mptr,int32 v);
	NULL,					// Generate text representation of an item in a special segment
	NULL,					// Icode of return instruction. It is ok to give any of possible return instructions
	NULL,					// const char *(*set_idp_options)(const char *keyword,int value_type,const void *value);
	NULL,                 // int (*is_align_insn)(ea_t ea);
	NULL,                 // mvm_t *mvm;
};
