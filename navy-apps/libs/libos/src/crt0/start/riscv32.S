.globl  _start
_start:
  add sp, a0, zero
  addi a0, a0, -4
  move s0, zero
  jal call_main
