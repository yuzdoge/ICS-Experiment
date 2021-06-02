//J-Type
static inline def_EHelper(jal){
  rtl_j(s, (cpu.pc + id_src1->imm));
  rtl_li(s, ddest, s->seq_pc);
  print_Dop(id_src1->str, OP_STR_SIZE, FMT_WORD, cpu.pc + id_src1->imm);
  print_asm_template2(jal); //jal rd,imm
} 

//I-Type
static inline def_EHelper(jalr){
  switch (s->isa.instr.i.funct3){
    case 0b000:
	  rtl_addi(s, s0, dsrc1, cpu.pc + id_src2->imm);
	  rtl_andi(s, s0, s0, ~(0b1));
	  rtl_jr(s, s0);
	  rtl_li(s, ddest, s->seq_pc);
      print_Dop(id_src1->str, OP_STR_SIZE, "%d(%s)", id_src2->imm, reg_name(id_src1->reg, 4));
	  print_asm_template2(jalr); //jalr rd,imm(rs1)
	  break;
	default: assert(0);
  }
}

//B-Type
static inline def_EHelper(beq){
  rtl_setrelop(s, RELOP_EQ, s0, dsrc1, dsrc2); 
  if (*s0 == true)
    rtl_j(s, (cpu.pc + id_dest->imm)); 
  print_asm_template3(beq); //beq rs1,rs2,imm
}

static inline def_EHelper(bne){
  rtl_setrelop(s, RELOP_NE, s0, dsrc1, dsrc2);
  if (*s0 == true)
    rtl_j(s, (cpu.pc + id_dest->imm));
  print_asm_template3(bne); //bne, rs1,rs2,imm
}

static inline def_EHelper(blt){
  rtl_setrelop(s, RELOP_LT, s0, dsrc1, dsrc2);
  if (*s0 == true) 
	rtl_j(s, (cpu.pc + id_dest->imm));
  print_asm_template3(blt); //blt rs1,rs2,imm
}

static inline def_EHelper(bge){
  rtl_setrelop(s, RELOP_GE, s0, dsrc1, dsrc2);
  if (*s0 == true)
    rtl_j(s, (cpu.pc + id_dest->imm));
  print_asm_template3(bge); //bge rs1,rs2,imm
}

static inline def_EHelper(bltu){
  rtl_setrelop(s, RELOP_LTU, s0, dsrc1, dsrc2);
  if (*s0 == true)
    rtl_j(s, (cpu.pc + id_dest->imm));
  print_asm_template3(bltu); //bltu rs1,rs2,imm
}

static inline def_EHelper(bgeu){
  rtl_setrelop(s, RELOP_GEU, s0, dsrc1, dsrc2);
  if (*s0 == true)
    rtl_j(s, (cpu.pc + id_dest->imm));
  print_asm_template3(bgeu); //bgeu rs1,rs2,imm
}
