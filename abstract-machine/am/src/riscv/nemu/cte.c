#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

#define HARD_ZERO 0

/*
 * zero register is implemented as hard-wire zero,
 * so gpr[0] in context struct is of no use
 * thus we use c->gpr[0] to store np
 * and restore it to zero when we restore context*/


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

static inline uintptr_t get_ms(){
	uintptr_t ms;
	asm volatile("csrr %0, mscratch" : "=r"(ms));
	return ms;
}

static inline void set_ms(uintptr_t val){
	asm volatile("csrw mscratch, %0" : :"r"(val));
}

void __am_get_cur_as(Context *c);

void __am_switch(Context *c);

Context* __am_irq_handle(Context *c) {
//  for(int i = 1; i < 32; ++i){
//	printf("%x\n", c->gpr[i]);
//  }  
//  printf("%x\n", c->mstatus);
//  printf("%x\n", c->mepc);
  printf("context size: %x\n", sizeof(Context));
  printf("here %x\n", c->gpr[2]);
  uintptr_t pre_sp = get_ms();
//  printf("os %x %x\n", pre_sp, sizeof(Context));
  if(pre_sp - sizeof(Context) != c->gpr[2]){
	printf("user\n");
	c->gpr[0] = 1;
  }else{
		printf("kernel\n");
	c->gpr[0] = 0;
  }
  c->gpr[2] = pre_sp;
  set_ms(0); 
//  printf("ksp:%x \n", get_ms()); 
  __am_get_cur_as(c);	
  if (user_handler) {
    Event ev = {0};
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
	  case 0x80000007: ev.event = EVENT_IRQ_TIMER; break;
	  case 0xffffffff: ev.event = EVENT_YIELD; break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
//  printf("new addr:%p\n", c);
//  printf("new sp:%p\n", c->gpr[2]);
  __am_switch(c);
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
	printf("%p\n", kstack.end);
	Context* c = (Context *)(kstack.end + 1 - sizeof(Context));
	for(int i = 0; i < 32; ++i){
		c->gpr[i] = 0;
 	}
//	c->mtvec = (uintptr_t)(__am_asm_trap);
	c->gpr[0] = 0;
	c->GPRx = (uintptr_t)(arg);
	c->mepc = (uintptr_t)(entry);
	c->mstatus = 0x1800;
	c->gpr[2] = (uintptr_t)(kstack.end - sizeof(Context));
//	c->mstatus |= (1 << 7);
	c->mcause = 0;
	c->pdir = NULL;	
	return c;
} 

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
