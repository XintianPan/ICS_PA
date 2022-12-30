#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void* fir_addr = pf;
  pf += PGSIZE * nr_page;
  return fir_addr;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
    void* page_alloc = new_page(n / PGSIZE);
	Log("%p", page_alloc);
	memset(page_alloc, 0, n);
	return page_alloc;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(PCB *pcb, uintptr_t brk) {
	if(pcb->max_brk < brk){
		uintptr_t v_page_pre = pcb->max_brk / PGSIZE;
		uintptr_t v_page_cur = brk / PGSIZE;
		if(v_page_cur != v_page_pre){ //this indicates that we need alloc new pages
			size_t page_num = v_page_cur - v_page_pre;
			uintptr_t v_new_start = ROUNDUP(pcb->max_brk, PGSIZE);
			void* pa;
			for(size_t i = 0; i < page_num; ++i){
				Log("addr: %p %p", brk, v_new_start);
				pa = new_page(1);
				map(&pcb->as, (void *)v_new_start, pa, 0);
				v_new_start += PGSIZE;
			}
		}
		pcb->max_brk = brk;
	}
	return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  Log("here");
  vme_init(pg_alloc, free_page);
  Log("end");
#endif
}
