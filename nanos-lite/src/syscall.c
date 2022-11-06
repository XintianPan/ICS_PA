#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[5];
  a[0] = c->GPR1; //a7
  a[1] = c->GPR2; //a0 used for argument
  a[2] = c->GPR3; //a1 used for argument
  a[3] = c->GPR4; //a2 used for argument
  a[4] = c->GPRx; //a0 used for return
  switch (a[0]) {
	case SYS_exit: Log("syscall:%s 1st arg:%d 2nd arg:%d 3rd arg:%d ret val:%d", syscall_name[a[0]], a[1], a[2], a[3], a[1]);  halt(a[1]); break;
	case SYS_yield: Log("syscall:%s 1st arg:%d 2nd arg:%d 3rd arg:%d ret val:%d", syscall_name[a[0]], a[1], a[2], a[3], a[1]);yield(); c->mepc += 4; a[4] = 0; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
