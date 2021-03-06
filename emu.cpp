#include "rx63.hpp"

static bool flow;

// handle immediate values
static void handle_imm(void) {

    doImmd(cmd.ea);
}

static void handle_operand(const op_t &op, bool loading)
{
  switch ( op.type )
  {
    // Address
    case o_near:
      {
        ea_t ea = toEA(cmd.cs, op.addr);
		int iscall = InstrIsSet(cmd.itype, CF_CALL);
        ua_add_cref(op.offb, ea, iscall ? fl_CN : fl_JN);

        if ( flow && iscall )
        {
          if ( !func_does_return(ea) )
            flow = false;
        }
     }
      break;

    // Immediate
    case o_imm:
		handle_imm();

		if ( op_adds_xrefs(uFlag, op.n) )
			ua_add_off_drefs2(op, dr_O, OOFW_IMM|OOF_SIGNED);

    //  // create a comment if this immediate is represented in the .cfg file
    //  {
    //    const ioport_t *port = find_sym(op.value);
    //    if ( port != NULL && !has_cmt(uFlag) )
    //        set_cmt(cmd.ea, port->cmt, false);
    //  }
      break;

    // Displ
    case o_displ:
      handle_imm();
    //  // if the value was converted to an offset, then create a data xref:
    //  if ( op_adds_xrefs(uFlag, op.n) )
    //    ua_add_off_drefs2(op, loading ? dr_R : dr_W, OOF_SIGNED|OOF_ADDR|OOFW_32);

    //  // create stack variables if required
    //  if ( may_create_stkvars() && !isDefArg(uFlag, op.n) )
    //  {
    //    func_t *pfn = get_func(cmd.ea);
    //    if ( pfn != NULL && (op.reg == rFP || op.reg == rSP) && pfn->flags & FUNC_FRAME )
    //    {
    //      if ( ua_stkvar2(op, op.addr, STKVAR_VALID_SIZE) )
    //        op_stkvar(cmd.ea, op.n);
    //    }
    //  }
      break;

    case o_phrase:
    case o_reg:
    case o_void:
	case o_flag:
	case o_creg:
        break;

    // Others types should never be called
    default:
      INTERR(10136);
  }
}

int idaapi emu(void)
{
	uint32 feature = cmd.get_canon_feature();
	flow = ((feature & CF_STOP) == 0);

	if ( feature & CF_USE1)    handle_operand(cmd.Op1, 1 );
	if ( feature & CF_USE2)    handle_operand(cmd.Op2, 1 );
	if ( feature & CF_USE3)    handle_operand(cmd.Op3, 1 );

	if ( feature & CF_JUMP)    QueueSet(Q_jumps, cmd.ea );

	if ( feature & CF_CHG1)    handle_operand(cmd.Op1, 0 );
	if ( feature & CF_CHG2)    handle_operand(cmd.Op2, 0 );
	if ( feature & CF_CHG3)    handle_operand(cmd.Op3, 0 );

	if ( flow)    ua_add_cref(0, cmd.ea + cmd.size, fl_F );

	if (cmd.itype == RX63_jsr)
	{
		uchar reg = cmd.Op1.reg;
		if (decode_prev_insn( cmd.ea ) != BADADDR)
		{
			if (cmd.itype == RX63_mov && 
				cmd.Op1.type == o_imm &&
				cmd.Op2.type == o_reg &&
				cmd.Op2.reg == reg)
			{
				ua_add_cref(0, cmd.Op1.value, fl_CF);
			}
		}
	}

	return 1;
}
