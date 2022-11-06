#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>
enum {
  SYS_exit,
  SYS_yield,
  SYS_open,
  SYS_read,
  SYS_write,
  SYS_kill,
  SYS_getpid,
  SYS_close,
  SYS_lseek,
  SYS_brk,
  SYS_fstat,
  SYS_time,
  SYS_signal,
  SYS_execve,
  SYS_fork,
  SYS_link,
  SYS_unlink,
  SYS_wait,
  SYS_times,
  SYS_gettimeofday
}; 

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
//  for(int i = 1; i < 32; ++i){
//	printf("%x\n", c->gpr[i]);
//  }  
//  printf("%x\n", c->mstatus);
//  printf("%x\n", c->mepc);
//  printf("%x\n", c->mcause);
	
  if (user_handler) {
    Event ev = {0};
	printf("%d\n", c->mcause);
    switch (c->mcause) {
	  case   SYS_exit:
      case   SYS_yield:
	  case   SYS_open:
	  case   SYS_read:
	  case  SYS_write:
	  case   SYS_kill:
	  case SYS_getpid:
	  case  SYS_close:
	  case  SYS_lseek:
	  case    SYS_brk:
	  case  SYS_fstat:
	  case   SYS_time:
	  case SYS_signal:
	  case SYS_execve:
	  case   SYS_fork:
	  case   SYS_link:
	  case SYS_unlink:
	  case   SYS_wait:
      case  SYS_times:
      case  SYS_gettimeofday: ev.event = EVENT_SYSCALL; break;
	  case 0xffffffff: ev.event = EVENT_YIELD; break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
