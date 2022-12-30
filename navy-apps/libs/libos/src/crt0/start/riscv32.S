.globl  _start
_start:
  add sp, a0, zero
  mv a0, a1
  move s0, zero
  jal call_main
