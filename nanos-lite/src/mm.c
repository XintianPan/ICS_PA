#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void* fir_addr = pf;
  pf += PGSIZE * nr_page;
  return fir_addr;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
    void* page_alloc = new_page(n / PGSIZE);
	memset(page_alloc, 0, n);
	return page_alloc;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
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
