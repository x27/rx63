#include "rx63.hpp"

static bool flow;

// handle immediate values
static void handle_imm(ea_t ea) {
    set_immd(ea);
}

static void handle_operand(const op_t &op, bool /*loading*/, const insn_t *insn) {
    switch ( op.type ) {
        // Address
        case o_near:
            {
                ea_t ea = to_ea(insn->cs, op.addr);
                int iscall = has_insn_feature(insn->itype, CF_CALL);
                insn->add_cref(ea, op.offb, iscall ? fl_CN : fl_JN);

                if ( flow && iscall ) {
                    if ( !func_does_return(ea) ) {
                        flow = false;
                    }
                }
            }
            break;

            // Immediate
        case o_imm:
            handle_imm(insn->ea);

            if ( op_adds_xrefs(get_flags(insn->ea), op.n) ) {
                insn->add_off_drefs(op, dr_O, OOFW_IMM|OOF_SIGNED);
            }

            //  // create a comment if this immediate is represented in the .cfg file
            //  {
            //    const ioport_t *port = find_sym(op.value);
            //    if ( port != NULL && !has_cmt(get_flags(insn->ea)) )
            //        set_cmt(insn->ea, port->cmt, false);
            //  }
            break;

            // Displ
        case o_displ:
            handle_imm(insn->ea);
            //  // if the value was converted to an offset, then create a data xref:
            //  if ( op_adds_xrefs(get_flags(insn->ea), op.n) )
            //    insn->add_off_drefs(op, loading ? dr_R : dr_W, OOF_SIGNED|OOF_ADDR|OOFW_32);

            //  // create stack variables if required
            //  if ( may_create_stkvars() && !isDefArg(get_flags(insn->ea), op.n) )
            //  {
            //    func_t *pfn = get_func(insn->ea);
            //    if ( pfn != NULL && (op.reg == rFP || op.reg == rSP) && pfn->flags & FUNC_FRAME )
            //    {
            //      if ( ua_stkvar2(op, op.addr, STKVAR_VALID_SIZE) )
            //        op_stkvar(insn->ea, op.n);
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

int idaapi emu(const insn_t *insn) {
    uint32 feature = insn->get_canon_feature();
    flow = ((feature & CF_STOP) == 0);

    if ( feature & CF_USE1)    handle_operand(insn->Op1, 1, insn );
    if ( feature & CF_USE2)    handle_operand(insn->Op2, 1, insn );
    if ( feature & CF_USE3)    handle_operand(insn->Op3, 1, insn );

    if ( feature & CF_JUMP)    remember_problem(PR_JUMP, insn->ea );

    if ( feature & CF_CHG1)    handle_operand(insn->Op1, 0, insn );
    if ( feature & CF_CHG2)    handle_operand(insn->Op2, 0, insn );
    if ( feature & CF_CHG3)    handle_operand(insn->Op3, 0, insn );

    if ( flow)    add_cref(insn->ea, insn->ea + insn->size, fl_F );

    if (insn->itype == RX63_jsr) {
        uchar reg = insn->Op1.reg;
        insn_t prevInsn;
        if (decode_prev_insn( &prevInsn, insn->ea ) != BADADDR) {
            if (prevInsn.itype == RX63_mov
                    && prevInsn.Op1.type == o_imm
                    && prevInsn.Op2.type == o_reg
                    && prevInsn.Op2.reg == reg) {
                prevInsn.add_cref(0, prevInsn.Op1.value, fl_CF);
            }
        }
    }

    return 1;
}
