#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void context_kload(PCB *pcb, void(*entry)(void *), void *arg);

void naive_uload(PCB *pcb, const char *filename); 

void switch_boot_pcb() {
  current = &pcb_boot;
} 

void hello_fun(void *arg) {
  int j = 1;
  while (1) { 
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
    j ++;
    yield();
   }
} 

void context_kload(PCB *pcb, void(*entry)(void *), void *arg){
	Area kstack;
	kstack.start =(void *)pcb;
	kstack.end = (void *)pcb + sizeof(PCB) - 1;
	pcb->cp = kcontext(kstack, entry, arg);
	Log("%p", pcb->cp->mepc);
}


void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)" Hell this ");
  context_kload(&pcb[1], hello_fun, (void *)" Heaven this ");
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
//  naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
	prev->mepc += 4;
	current->cp = prev;
	current = (current == &pcb[0]) ? &pcb[1] : &pcb[0];
//	Log("%p", current->cp->mepc);
	return current->cp;
}
