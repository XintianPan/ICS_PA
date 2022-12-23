.globl  _start
_start:
  add sp, a0, zero
  move s0, zero
  addi a0, a0, -4
  jal call_main
