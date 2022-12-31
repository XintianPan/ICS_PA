#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
f(10) f(11) f(12) f(13) f(14) f(15) f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((32 + 3 + 1) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  (32 * XLEN)
#define OFFSET_STATUS (33 * XLEN)
#define OFFSET_EPC    (34 * XLEN)

.align 3
.globl __am_asm_trap
__am_asm_trap:
  csrrw sp, mscratch, sp
  bnez sp, save_context
  csrr sp, mscratch

save_context:
  addi sp, sp, -CONTEXT_SIZE

  MAP(REGS, PUSH)

  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)
  STORE sp, OFFSET_SP(sp)
#  csrr t0, mscratch
  # save pre sp, next to save np 
#  STORE t0, OFFSET_SP(sp) # mscratch saves the value of sp before using the kernel stack
#  mv t1, sp # now t1 has the value of sp
#  addi t2, zero, 1 # user stack before entering
#  addi t0, t0, -CONTEXT_SIZE # mscratch value minus context_size if it is equal to sp, then it's kernel, otherwise it's user
#  sub t0, t0, sp # sub
#  bnez t0, save_np # not equal to 0 means it's user, otherwise it's user
#  addi t2, t2, -1 # kernel stack before entering
  # set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  mv a0, sp
  jal __am_irq_handle
  mv sp, a0
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw mstatus, t1
  csrw mepc, t2

  MAP(REGS, POP)

  addi sp, sp, CONTEXT_SIZE
#  STORE t0, 0(sp)
#  LOAD t0, -CONTEXT_SIZE(sp)
#  addi t0, t0, -1
#  bnez t0, restore_sp
#  csrw mscratch, sp	
 
#restore_sp:
#  LOAD t0, 0(sp)
#  LOAD sp, (-CONTEXT_SIZE + OFFSET_SP)(sp)

  mret
