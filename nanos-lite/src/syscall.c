#include <common.h>
#include "syscall.h"

void mywrite(char *buf, uint32_t count){
	uint32_t i = 0;
	for(; i < count; ++i){
		putch(*(buf + i));
	}
}

void do_syscall(Context *c) {
  uintptr_t a[5];
  a[0] = c->GPR1; //a7
  a[1] = c->GPR2; //a0 used for argument
  a[2] = c->GPR3; //a1 used for argument
  a[3] = c->GPR4; //a2 used for argument
  a[4] = c->GPRx; //a0 used for return
  switch (a[0]) {
	case SYS_exit:
	   	Log("syscall:%s 1st arg:%d 2nd arg:%d 3rd arg:%d ret val:%d", syscall_name[a[0]], a[1], a[2], a[3], a[1]);
	   	halt(a[1]);
	   	break;
	case SYS_yield: 
		Log("syscall:%s 1st arg:%d 2nd arg:%d 3rd arg:%d ret val:%d", syscall_name[a[0]], a[1], a[2], a[3], a[1]);
		yield(); 
		c->mepc += 4; a[4] = 0; break;
	case SYS_write: 
	   	// Log("syscall:%s 1st arg:%d 2nd arg:%p 3rd arg:%d ret val:%d", syscall_name[a[0]], a[1], a[2], a[3], a[3]); 
		if(a[1] == 1 || a[1] == 2)
		   	mywrite((char *)a[2], a[3]); 
	   	a[4] = a[3];
	   	c->mepc += 4;  
		break;
	case SYS_brk:
		Log("syscall:%s 1st arg:%d 2nd arg:%p 3rd arg:%d ret val:%d", syscall_name[a[0]], a[1], a[2], a[3], 0);
		a[4] = 0;
		Log("%p", *(volatile int*)a[2]);
	    *(volatile int *)a[2] = *(volatile int*)a[2] + a[1];
		Log("%p", *(volatile int*)a[2]);
		c->mepc += 4;
		break;
	default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
