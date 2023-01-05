#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

//static int time_seg = 0;

extern char _std_out_buf[10000 + 5];

extern char _std_num_buf[20];

static PCB *kpcb[4] = {NULL};

static int id = 0;

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
	kpcb[id++] = pcb;
	pcb->cp = kcontext(kstack, entry, arg);
//	Log("%p", pcb);
}

//static char *parse_arg[] = {NULL };

//static char *parse_envp[] = {NULL };

void init_proc() {
    Log("Initializing processes...");
  	context_kload(&pcb[0], hello_fun, "NO");
//  Log("%p", &pcb[0].cp);
//  Log("%p", pcb[0].cp);
    context_kload(&pcb[1], hello_fun, "YES");
//  Log("%p", pcb[0].cp);
  switch_boot_pcb();
//  Log("%p %p", pcb[0].cp, pcb[1].cp->mepc);

  // load program here
//  naive_uload(NULL, "/bin/menu");
}

Context* schedule(Context *prev) {
//	prev->mepc += 4;
//	pcb[0].cp->pdir = NULL;
//	Log("%p", prev);
	current->cp = prev;
//	for(int i = 0; i < 4; ++i){
//		if(current == kpcb[i]){
//			current->cp->pdir = NULL;
//			break;
//		}
//	}
//	if(time_seg < 9)
//	current =  &pcb[1], ++time_seg;
//	else
//	current = &pcb[0], time_seg = 0;
//  Log("%p", _std_num_buf);	
//  Log("%p %p", pcb[0].cp->pdir, pcb[1].cp->pdir);
  Log("%p %p %p %p", pcb[0].cp, pcb[1].cp ,pcb[0].cp->gpr[2], pcb[1].cp->gpr[2]);
//	Log("%p %p %p %p %p %p", pcb[0].cp, pcb[1].cp, pcb[0].cp->gpr[10], pcb[1].cp->gpr[10], pcb[0].cp->mepc, pcb[1].cp->mepc);
    current = (current == &pcb[0]) ? &pcb[1] : &pcb[0];
	return current->cp;
}
