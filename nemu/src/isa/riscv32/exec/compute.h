// U-Type
static inline def_EHelper(lui) {
  rtl_li(s, ddest, id_src1->imm);
  print_asm_template2(lui); //lui rd,imm
}

static inline def_EHelper(auipc){
  rtl_li(s, ddest, id_src1->imm + cpu.pc);
  print_asm_template2(auipc); //auipc rd,imm
}

// I_Type
static inline def_EHelper(addi){
  rtl_addi(s, ddest, dsrc1, id_src2->imm);
  print_asm_template3(addi); //addi rd,rs1,imm
}

//#define smask 0x80000000
static inline def_EHelper(slti){
  /*
  word_t res = (sword_t)(*dsrc1) - (sword_t)(id_src2->imm);
  word_t sf = (*dsrc1) & smask;
  // res = a - b: overflow occurs if only if the sign of `a` 
  // different from `res` and `b` while excuting subtraction   
  word_t of = (*dsrc1 ^ id_src2->imm) & (*dsrc1 ^ res) & smask; 
  assert((sf != of) == ((sword_t)(*dsrc1) < (sword_t)(id_src2->imm)));
  rtl_addi(s, ddest, rz, (sf != of));
  */
  rtl_setrelopi(s, RELOP_LT, ddest, dsrc1, id_src2->imm);
  print_asm_template3(slti); //slti rd,rs1,imm
}

static inline def_EHelper(sltiu){
  //maybe it can complete with use Cout, instead of '<'
  rtl_setrelopi(s, RELOP_LTU, ddest, dsrc1, id_src2->imm);
  print_asm_template3(sltiu); //sltiu rd,rs1,imm
}

static inline def_EHelper(xori){
  rtl_xori(s, ddest, dsrc1, id_src2->imm);
  print_asm_template3(xori); //xori rd,rs1,imm
}

static inline def_EHelper(ori){
  rtl_ori(s, ddest, dsrc1, id_src2->imm);
  print_asm_template3(ori); //ori rd,rs1,imm
}

static inline def_EHelper(andi){
  rtl_andi(s, ddest, dsrc1, id_src2->imm);
  print_asm_template3(andi); //andi rd,rs1,imm
}

static inline def_EHelper(slli){
  switch ((id_src2->imm >> 5) & 0x7f){
    case 0b0000000:
	  rtl_shli(s, ddest, dsrc1, id_src2->imm);
	  print_asm_template3(slli); //ssli rd,rs1,imm
	  break;
	default: assert(0);
  }
}

static inline def_EHelper(srlai){
  switch ((id_src2->imm >> 5) & 0x7f){
    //imm[11:5]
	case 0b0000000:
	  rtl_shri(s, ddest, dsrc1, id_src2->imm);
	  print_asm_template3(srli); //srli rd,rs1,imm
	  break;
	case 0b0100000:	  
	  rtl_sari(s, ddest, dsrc1, id_src2->imm);
	  print_asm_template3(srai); //srai rd,rs1,imm
	  break;
	default: assert(0);	  
  }
}

// R-Type
static inline def_EHelper(add_sub_mul){
  switch (s->isa.instr.r.funct7){
	// Base 
    case 0b0000000:
	  rtl_add(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(add); //add rd,rs1,rs2
	  break;
	case 0b0100000:
	  rtl_sub(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(sub); //sub rd,rs1,rs2
	  break;

	// M
	case 0b0000001:
	  rtl_mul_lo(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(sub); //mul rd,rs1,rs2
	  break;
	default: assert(0);
  }
}

static inline def_EHelper(slt_mulhsu){
  switch (s->isa.instr.r.funct7){
    case 0b0000000:
	  rtl_setrelop(s, RELOP_LT, ddest, dsrc1, dsrc2);
	  print_asm_template3(slt); //slt rd,rs1,rs2
	  break;
	case 0b0000001:
	  /* for instance: (0b1010 * 0b1110u) -> (0b1111 1010 * 0b0000 1110u) 
	   *				-> (0b1111 0000u * 0b1110u) + (0b0000 1010u * 0b1110u)*/ 
	  rtl_mul_hi(s, s0, dsrc1, dsrc2);
	  rtl_li(s, s1, -1);
	  rtl_mul_hi(s, s1, s1, dsrc2);
	  rtl_add(s, ddest, s0, s1);
	  print_asm_template3(mulhsu);
	  break;
    default: assert(0);	  
  }
}

static inline def_EHelper(sltu_mulhu){
  switch (s->isa.instr.r.funct7){
    case 0b0000000:
	  rtl_setrelop(s, RELOP_LTU, ddest, dsrc1, dsrc2);
	  print_asm_template3(sltu); //sltu rd,rs1,rs2
	  break;
	case 0b0000001:
	  rtl_mul_hi(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(mulhu); //mulhu rd,rs1,rs2 
	  break;
    default: assert(0);	  
  }
}

static inline def_EHelper(xor_div){
  switch (s->isa.instr.r.funct7){
	case 0b0000000:
	  rtl_xor(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(xor); //xor rd,rs1,rs2
	  break;
	case 0b0000001:
	  (*dsrc2 == 0) ? rtl_li(s, ddest, -1) : 
	    ((*dsrc1 == 0x80000000) && (*dsrc2 == -1)) ? 
		  rtl_mv(s, ddest, dsrc1) : rtl_idiv_q(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(div); //div rd,rs1,rs2
	  break;
	default: assert(0);
  }
}

static inline def_EHelper(or_rem){
  switch (s->isa.instr.r.funct7){
    case 0b0000000:
	  rtl_or(s, ddest, dsrc1, dsrc2);	
	  print_asm_template3(or); //or rd,rs1,rs2
	  break;
	case 0b0000001:
	  (*dsrc2 == 0) ? rtl_mv(s, ddest, dsrc1) : 
	    ((*dsrc1 == 0x80000000) && (*dsrc2 == -1)) ? 
		  rtl_li(s, ddest, 0) : rtl_idiv_r(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(rem); //rem rd,rs1,rs2
	  break;
	default: assert(0);
  }
}

static inline def_EHelper(and_remu){
  switch (s->isa.instr.r.funct7){
    case 0b0000000:
	  rtl_and(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(and); //and rd,rs1,rs2
	  break;
	case 0b0000001:
	  (*dsrc2 == 0) ? rtl_mv(s, ddest, dsrc1) : 
		rtl_div_r(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(remu); //remu rd,rs1,rs2
	  break;
	default: assert(0);
  }
}

static inline def_EHelper(sll_mulh){
  switch (s->isa.instr.r.funct7){
    case 0b0000000:
	  rtl_shl(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(sll); //sll rd,rs1,rs2
	  break;
	case 0b0000001:
	  rtl_imul_hi(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(mulh); //mulh rd,rs1,rs2
	  break;
	default: assert(0);
  }
}

static inline def_EHelper(srla_divu){
  switch (s->isa.instr.r.funct7){
    case 0b0000000:
	  rtl_shr(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(srl); //srl rd,rs1,rs2
	  break;
    case 0b0100000:
	  rtl_sar(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(sra); //sra rd,rs1,rs2
      break;	   
	case 0b0000001:
	  (*dsrc2 == 0) ? rtl_li(s, ddest, -1) : 
		rtl_div_q(s, ddest, dsrc1, dsrc2);
	  print_asm_template3(divu); //divu rd,rs1,rs2
	  break;
	default: assert(0);
  }
}
