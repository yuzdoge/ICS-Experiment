static inline def_EHelper(ld) {
  rtl_lm(s, ddest, dsrc1, id_src2->imm, s->width);

  print_Dop(id_src1->str, OP_STR_SIZE, "%d(%s)", id_src2->imm, reg_name(id_src1->reg, 4));
  switch (s->width) {
    case 4: print_asm_template2(lw); break; //lw rd,imm(rs)
    case 2: print_asm_template2(lhu); break; //lhu rd,imm(rs)
    case 1: print_asm_template2(lbu); break; //lhu rd,imm(rs)
    default: assert(0);
  }
}

static inline def_EHelper(lds){
  rtl_lms(s, ddest, dsrc1, id_src2->imm, s->width);

  print_Dop(id_src1->str, OP_STR_SIZE, "%d(%s)", id_src2->imm, reg_name(id_src1->reg, 4));
  switch (s->width){
    case 2: print_asm_template2(lh); break; //lh rd,imm(rs)
    case 1: print_asm_template2(lb); break; //lb rd,imm(rs)
    default: assert(0);
  }
} 

static inline def_EHelper(st) {
  rtl_sm(s, dsrc1, id_src2->imm, ddest, s->width);

  print_Dop(id_src1->str, OP_STR_SIZE, "%d(%s)", id_src2->imm, reg_name(id_src1->reg, 4));
  switch (s->width) {
    case 4: print_asm_template2(sw); break; //sw rs,imm(rd)
    case 2: print_asm_template2(sh); break; //sh rs,imm(rd)
    case 1: print_asm_template2(sb); break; //sb rs,imm(rd)
    default: assert(0);
  }
}
