.globl  _start
_start:
  addi sp, a0, zero
  move s0, zero
  jal call_main
