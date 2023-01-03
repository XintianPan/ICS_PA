#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

// static int time_seg = 0;

PCB *kpcb = NULL;
void context_kload(PCB *pcb, void(*entry)(void *), void *arg);

void naive_uload(PCB *pcb, const char *filename); 

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);

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
	kpcb = pcb;
	pcb->cp = kcontext(kstack, entry, arg);
//	Log("%p", pcb);
}

static char *parse_arg[] = {NULL };

static char *parse_envp[] = {NULL };

void init_proc() {
  context_uload(&pcb[0], "/bin/hello", parse_arg, parse_envp);
  context_uload(&pcb[1], "/bin/nterm", parse_arg, parse_envp);
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
//  naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
//	prev->mepc += 4;
//	pcb[0].cp->pdir = NULL;
	current->cp = prev;
	if(current == kpcb) current->cp->pdir = NULL;
//	if(time_seg < 9)
//	current =  &pcb[1], ++time_seg;
//	else
//	current = &pcb[0], time_seg = 0;	
	Log("%p %p %p %p %p %p", pcb[0].cp, pcb[1].cp, pcb[0].cp->gpr[2], pcb[1].cp->gpr[2], pcb[0].cp->mepc, pcb[1].cp->mepc);
    current = (current == &pcb[0]) ? &pcb[1] : &pcb[0];
	return current->cp;
}
