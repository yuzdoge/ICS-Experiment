#include <cpu/exec.h>
#include "../local-include/decode.h"
#include "all-instr.h"

static inline void set_width(DecodeExecState *s, int width) {
  if (width != 0) s->width = width;
}

static inline def_EHelper(load) {
  switch (s->isa.instr.i.funct3) {
	EXW  (0b000, lds, 1)
	EXW  (0b001, lds, 2)

    EXW  (0b010, ld, 4)
	EXW  (0b100, ld, 1)
	EXW  (0b101, ld, 2)
    default: exec_inv(s);
  }
}

static inline def_EHelper(store) {
  switch (s->isa.instr.s.funct3) {
	EXW  (0b000, st, 1)
	EXW  (0b001, st, 2)
    EXW  (0b010, st, 4)
    default: exec_inv(s);
  } 
}

static inline def_EHelper(op_imm){
  switch (s->isa.instr.i.funct3){
    EX  (0b000, addi)
	EX  (0b010, slti)
	EX  (0b011, sltiu)

	EX  (0b100, xori)
	EX  (0b110, ori)
	EX  (0b111, andi)
	
	EX  (0b001, slli)
	EX  (0b101, srlai) //logical or arithematic
	default: exec_inv(s);
  } 
}

static inline def_EHelper(op){
  switch (s->isa.instr.r.funct3){
    EX  (0b000, add_sub_mul)
	EX  (0b010, slt_mulhsu)
	EX  (0b011, sltu_mulhu)

    EX  (0b100, xor_div)
	EX  (0b110, or_rem)
    EX  (0b111, and_remu)
	
	EX  (0b001, sll_mulh)
	EX  (0b101, srla_divu) //logical or arithematic
	default: exec_inv(s);
  } 
}

static inline def_EHelper(branch){
  switch (s->isa.instr.b.funct3){
    EX (0b000, beq)
    EX (0b001, bne)
	EX (0b100, blt)
	EX (0b101, bge)
	EX (0b110, bltu)
	EX (0b111, bgeu)
  } 
}

static inline void fetch_decode_exec(DecodeExecState *s) {
  s->isa.instr.val = instr_fetch(&s->seq_pc, 4); 
  printf("%0x\n", s->isa.instr.val);
  assert(s->isa.instr.i.opcode1_0 == 0x3);
  switch (s->isa.instr.i.opcode6_2) {
    IDEX (0b00000, I, load)
	IDEX (0b00100, I, op_imm)
	IDEX (0b11001, I, jalr)

    IDEX (0b01000, S, store)

	IDEX (0b00101, U, auipc)
    IDEX (0b01101, U, lui)

	IDEX (0b01100, R, op)

	IDEX (0b11011, J, jal)
	IDEX (0b11000, B, branch)
    EX   (0b11010, nemu_trap)
    default: exec_inv(s);
  }
}

static inline void reset_zero() {
  reg_l(0) = 0;
}

vaddr_t isa_exec_once() {
  DecodeExecState s;
  s.is_jmp = 0;
  s.seq_pc = cpu.pc;

  fetch_decode_exec(&s);
  update_pc(&s);

  reset_zero();

  return s.seq_pc;
}
