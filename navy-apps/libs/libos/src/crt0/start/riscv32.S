.globl  _start
_start:
  move a0, sp
  move s0, zero
  jal call_main
