#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

extern uintptr_t kernel_thread;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)
#define ADDRMASK(bits) ((1ull << (bits)) - 1)
#define VPN(x, hi, lo) (((x) >> (lo)) &ADDRMASK((hi) - (lo) + 1))

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);
//  printf("dir:%p\n", kas.ptr);
  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
 // printf("get addr:%p\n", c->pdir);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
	//  printf("addr:%p\n", c->pdir);
	  set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
	PTE *dir_addr = (PTE*)as->ptr;
	uintptr_t va_addr = (uintptr_t)(va);
	uintptr_t pa_addr = (uintptr_t)(pa);
	uintptr_t vpn1 = VPN(va_addr, 31, 22);
	uintptr_t vpn0 = VPN(va_addr, 21, 12);
	PTE *fir_dir = dir_addr + vpn1;
//	printf("addr:%p 0x%x\n", fir_dir, vpn1);
	PTE *sec_dir;
    if((*fir_dir & 1) == 0){
		void* new_pg = pgalloc_usr(PGSIZE);
		uintptr_t ppn_fir = (uintptr_t)new_pg >> 12;
		*fir_dir = (ppn_fir << 10) | PTE_V;
		sec_dir = (uintptr_t *)new_pg + vpn0;
	}
	else{
		PTE fir = *fir_dir;
		fir >>= 10;
		fir <<= 12;
		sec_dir = (uintptr_t *)fir + vpn0;
	}
//	printf("sec_pos:%p\n", sec_dir);
//	if((*sec_dir & 1) == 1)
//		return ;
	uintptr_t final_ppn = pa_addr >> 12;
	*sec_dir = (final_ppn << 10) | PTE_R | PTE_W | PTE_X | PTE_V;	
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
	protect(as);
	Context* c = (Context *)(kstack.end + 1 - sizeof(Context));
	for(int i = 0; i < 32; ++i){
		c->gpr[i] = 0;
	}
	c->pdir = as->ptr;
//	c->gpr[10] = (uintptr_t)heap.end;
	c->mepc = (uintptr_t)(entry);
	c->mstatus = 0x1800;
	c->mstatus |= (1 << 3);
	c->mcause = 0;	
    return c;
}
