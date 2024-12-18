#include <common.h>

void do_syscall(Context *c); 
Context* schedule(Context *prev);

static Context* do_event(Event e, Context* c) {
   switch (e.event) {
	case EVENT_YIELD: 
//		printf("catch this!\n");
//		Log("yield");
		c->mepc += 4;
	   	return schedule(c);
	   	break;
	case EVENT_IRQ_TIMER:
		Log("timer");
		return schedule(c);
		break;
	case EVENT_SYSCALL: 
//		printf("got this!\n");
	   	do_syscall(c);
//		Log("sys:%p %p", c, c->gpr[2], c->pdir);
	   	break; 
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
