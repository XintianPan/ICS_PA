/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>


#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write
#define S(i) cpu.sys[i]

word_t isa_raise_intr(word_t NO, vaddr_t epc);

static inline void csrw(vaddr_t rd, word_t rs_imm, vaddr_t csr){
	vaddr_t id = -1;
	switch(csr){
		case 0x300: id = 0; break; //mstatus
		case 0x305: id = 1; break; //mtvec
		case 0x341: id = 2; break; //mepc
		case 0x342: id = 3; break; //mcause
		case 0x180: id = 4; break; //satp
		case 0x340: id = 5; break; //mscratch
	} 
	if(rd != 0) R(rd) = S(id);
	S(id) = rs_imm;
}
static inline void csrs(vaddr_t rd, word_t rs_imm, vaddr_t csr){
	vaddr_t id = -1;
	switch(csr){
		case 0x300: id = 0; break; //mstatus
		case 0x305: id = 1; break; //mtvec
		case 0x341: id = 2; break; //mepc
		case 0x342: id = 3; break; //mcause
        case 0x180: id = 4; break; //satp
		case 0x340: id = 5; break; //mscratch
	}
	if(rd != 0) R(rd) = S(id);
	uint32_t t;
	for(int i = 0; i < 32; ++i){
		t = 1 << i;
		if((t & rs_imm)) S(id) |= t;
	}
}
static inline void csrc(vaddr_t rd, word_t rs_imm, vaddr_t csr){
	vaddr_t id = -1;
	switch(csr){
		case 0x300: id = 0; break; //mstatus
		case 0x305: id = 1; break; //mtvec
		case 0x341: id = 2; break; //mepc
		case 0x342: id = 3; break; //mcause
	    case 0x180: id = 4; break; //satp
		case 0x304: id = 5; break; //mscratch
	} 
	if(rd != 0) R(rd) = S(id);
	uint32_t t;
	for(int i = 0; i < 32; ++i){
		t = 1 << i;
		if((t & rs_imm)){
			if((t & S(id))){
				S(id) ^= t;
	 		}
	 	}
	} 
}

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_R, TYPE_J, TYPE_B, TYPE_IS, TYPE_CS, TYPE_CSI,// I commands for shifting
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immB() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | (BITS(i, 30, 25) << 5) | (BITS(i, 11, 8) << 1) | (BITS(i, 7, 7) << 11);} while(0)
#define immJ() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | (BITS(i, 30, 21) << 1) | (BITS(i, 20, 20) << 11) | (BITS(i, 19, 12) << 12);} while(0)
#define immIS() do { *imm = BITS(i, 24, 20); } while(0)
#define immCS() do { *imm = BITS(i, 19, 15); } while(0)
#define addrCSR() do { *src2 = BITS(i, 31, 20); } while(0)
static void decode_operand(Decode *s, int *dest, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rd  = BITS(i, 11, 7);
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *dest = rd;
  switch (type) { 
    case TYPE_I: src1R();          immI(); break;
	case TYPE_IS: src1R();        immIS(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
	case TYPE_R: src1R(); src2R();         break;
	case TYPE_J:				   immJ(); break;
	case TYPE_B: src1R(); src2R(); immB(); break;
	case TYPE_CS:addrCSR(); src1R();       break;
	case TYPE_CSI:addrCSR();      immCS(); break;
  }
}

//static void che(paddr_t data, paddr_t pc){
//	if(data == 0x83076874) printf("0x%08x\n", pc);
//}

#define MPIE_REC 0x00000080
#define MPIE_EPC 0xffffff7f
void mstatus_r(){
	word_t mpie = BITS(S(0), 7, 7);
	S(0) |= (mpie << 3);
	S(0) &= MPIE_EPC;
}


static int decode_exec(Decode *s, bool *jmp) {  
  int dest = 0;
  uint64_t rel = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(dest) = imm);
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(dest) = imm + cpu.pc);
 
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, R(dest) = s->snpc, s->dnpc = imm + cpu.pc, *jmp = true);
 
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(dest) = src1 + imm);
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(dest) = ((int)src1 < (int)imm));
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(dest) = ((word_t)src1 < (word_t)imm));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(dest) = (src1 ^ imm));
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(dest) = (src1 | imm));
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(dest) = (src1 & imm));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(dest) = SEXT(Mr(src1 + imm, 1), 8));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(dest) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(dest) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(dest) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, R(dest) = s->snpc, s->dnpc = src1 + imm, *jmp = true);
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , IS, R(dest) = (src1 << imm));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , IS, R(dest) = ((word_t)src1 >> imm));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , IS, R(dest) = ((int)src1 >> imm));

  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(dest) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(dest) = src1 - src2); 
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(dest) = (src1 << (BITS(src2, 4, 0))));
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(dest) = ((int)src1 < (int)src2));
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(dest) = ((word_t)src1 < (word_t)src2));
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(dest) = (src1 ^ src2));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(dest) = ((word_t)src1 >> (BITS(src2, 4, 0))));
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(dest) = ((int)src1 >> (BITS(src2, 4, 0))));
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(dest) = (src1 | src2));
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(dest) = (src1 & src2));
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, rel = (uint64_t)src1 * (uint64_t)src2, R(dest) = BITS(rel, 31, 0));
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, rel = (int64_t)((int64_t)((int)src1) * (int64_t)((int)src2)), R(dest) = BITS(rel, 63, 32));
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu , R, rel = (int64_t)((int)src1) * (uint64_t)src2, R(dest) = BITS(rel, 63, 32));
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, rel = (uint64_t)src1 * (uint64_t)src2, R(dest) = BITS(rel, 63, 32));
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(dest) = (int)src1 / (int)src2);
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(dest) = (word_t)src1 / (word_t)src2);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", remi   , R, R(dest) = (int)src1 % (int)src2);
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(dest) = (word_t)src1 % (word_t)src2);



  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, s->dnpc = (src1 == src2 ? cpu.pc + imm : s->snpc));
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, s->dnpc = (src1 != src2 ? cpu.pc + imm : s->snpc)); 
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, s->dnpc = ((int)src1 < (int)src2 ? cpu.pc + imm : s->snpc));
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, s->dnpc = ((int)src1 >= (int)src2 ? cpu.pc + imm : s->snpc));
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, s->dnpc = ((word_t)src1 < (word_t)src2 ? cpu.pc + imm : s->snpc));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, s->dnpc = ((word_t)src1 >= (word_t)src2 ? cpu.pc + imm : s->snpc));

  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));

  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , CS, csrw(dest, src1, src2));
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  , CS, csrs(dest, src1, src2));
  INSTPAT("??????? ????? ????? 011 ????? 11100 11", csrrc  , CS, csrc(dest, src1, src2));

  INSTPAT("??????? ????? ????? 101 ????? 11100 11", csrrwi , CSI, csrw(dest, imm, src2));
  INSTPAT("??????? ????? ????? 110 ????? 11100 11", csrrsi , CSI, csrs(dest, imm, src2));
  INSTPAT("??????? ????? ????? 111 ????? 11100 11", csrrci , CSI, csrc(dest, imm, src2));

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , N, s->dnpc = isa_raise_intr(R(17), 1)); //R(17) is $a7 
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , N, s->dnpc = cpu.sys[2], mstatus_r());
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s, bool *jmp) {
//  printf("0x%08x ", s->snpc);
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
//  printf("0x%08x\n", s->isa.inst.val);
  return decode_exec(s, jmp);
}
