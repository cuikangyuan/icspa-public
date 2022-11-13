#include "cpu/instr.h"

static void instr_execute_2op() 
{
	operand_read(&opr_src);
	opr_dest.val = opr_src.val;
	operand_write(&opr_dest);
}

make_instr_impl_2op(mov, r, rm, b)
make_instr_impl_2op(mov, r, rm, v)
make_instr_impl_2op(mov, rm, r, b)
make_instr_impl_2op(mov, rm, r, v)
make_instr_impl_2op(mov, i, rm, b)
make_instr_impl_2op(mov, i, rm, v)
make_instr_impl_2op(mov, i, r, b)
make_instr_impl_2op(mov, i, r, v)
make_instr_impl_2op(mov, a, o, b)
make_instr_impl_2op(mov, a, o, v)
make_instr_impl_2op(mov, o, a, b)
make_instr_impl_2op(mov, o, a, v)

make_instr_func(mov_zrm82r_v) {
	int len = 1;
	OPERAND r, rm;
	r.data_size = data_size;
	rm.data_size = 8;
	len += modrm_r_rm(eip + 1, &r, &rm);
	
	operand_read(&rm);
	r.val = rm.val;
	operand_write(&r);

	print_asm_2("mov", "", len, &rm, &r);
	return len;
}

make_instr_func(mov_zrm162r_l) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);

        operand_read(&rm);
        r.val = rm.val;
        operand_write(&r);
	print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_srm82r_v) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = data_size;
        rm.data_size = 8;
        len += modrm_r_rm(eip + 1, &r, &rm);
        
	operand_read(&rm);
        r.val = sign_ext(rm.val, 8);
        operand_write(&r);
	print_asm_2("mov", "", len, &rm, &r);
        return len;
}

make_instr_func(mov_srm162r_l) {
        int len = 1;
        OPERAND r, rm;
        r.data_size = 32;
        rm.data_size = 16;
        len += modrm_r_rm(eip + 1, &r, &rm);
        operand_read(&rm);
        r.val = sign_ext(rm.val, 16);
        operand_write(&r);

	print_asm_2("mov", "", len, &rm, &r);
        return len;
}

/**
 * 30012:	8e d8                	mov    %eax,%ds
   30014:	8e c0                	mov    %eax,%es
   30016:	8e d0                	mov    %eax,%ss
 * SegReg es, cs, ss, ds, fs, gs;
 */

make_instr_func(mov_rm2s_w) {
        int len = 1;
        OPERAND m;
        m.data_size = data_size;
        uint8_t op;

        len += modrm_opcode_rm(eip + 1, &op, &m);
	operand_read(&m);

        cpu.segReg[op].val = m.val;
        load_sreg(op); 

	print_asm_1("mov_rm2s_w", "", len, &m);

        return len;
}

//30018:	0f 20 c0             	mov    %cr0,%eax
//20 MOV Cd->Rd
make_instr_func(mov_cr2r_v) {
        int len = 1;
        OPERAND r, cr;
        r.data_size = data_size;
        cr.data_size = data_size;

        len += modrm_r_rm(eip + 1, &cr, &r);
        cr.type = OPR_CREG;
        operand_read(&cr);
        r.val = cr.val;
        operand_write(&r);
	print_asm_2("mov_cr2r_v", "", len, &cr, &r);

        return len;
}

//3001e:	0f 22 c0             	mov    %eax,%cr0
//22 MOV Rd->Cd
make_instr_func(mov_r2cr_v) {
        int len = 1;
        OPERAND r, cr;
        r.data_size = data_size;
        cr.data_size = data_size;

        len += modrm_r_rm(eip + 1, &cr, &r);
        cr.type = OPR_CREG;
        operand_read(&r);
        cr.val = r.val;
        operand_write(&cr);
	print_asm_2("mov_r2cr_v", "", len, &r, &cr);

        return len;
}

