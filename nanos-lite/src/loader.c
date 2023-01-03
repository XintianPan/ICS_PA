#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_MIPS32__)
#define EXPECT_TYPE EM_MIPS_RS3_LE
#elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__)
#define EXPECT_TYPE EM_RISCV
#else
# error Unsupported ISA
#endif 

void *new_page(size_t nr_page);

int fs_open(const char *pathname, int flags, int mode);

int fs_close(int fd);

size_t fs_read(int fd, void *buf, size_t len);

size_t fs_lseek(int fd, size_t offset, int whence);

static uint8_t page_cache[PGSIZE];

static uintptr_t loader(PCB *pcb, const char *filename) {
	Elf_Ehdr elf_ehdr;
	Elf_Phdr elf_phdr;
	size_t bytes;
	int fd = fs_open(filename, 1, 1);
	bytes = fs_read(fd, &elf_ehdr, sizeof(Elf_Ehdr));
	assert(bytes == sizeof(Elf_Ehdr));
	assert(*(uint32_t *)elf_ehdr.e_ident = 0x7f454c46);
	assert(elf_ehdr.e_machine == EXPECT_TYPE);
    size_t i = 0;
	size_t j = 0;
//	size_t off = 0;
	size_t file, mem;
//	uint8_t byte;
	uint32_t vaddr;
	uint32_t off = elf_ehdr.e_phoff;
	uint32_t remain_num = 0;
	void *pa = NULL;
 	for(; i < elf_ehdr.e_phnum; ++i){
		fs_lseek(fd, off + i * (sizeof(Elf_Phdr)), SEEK_SET);
		bytes = fs_read(fd, &elf_phdr, sizeof(Elf_Phdr));
	    assert(bytes == sizeof(Elf_Phdr));
 	    if(elf_phdr.p_type == PT_LOAD){
	//		Log("0x%x", elf_phdr.p_align);
			vaddr = elf_phdr.p_vaddr;
		    file = elf_phdr.p_filesz;
			mem = elf_phdr.p_memsz;
	//		Log("start addr:0x%x memsize: 0x%x filesize: 0x%x", vaddr, mem, file);
			j = 1;
			fs_lseek(fd, elf_phdr.p_offset, SEEK_SET);
			size_t pre_page = vaddr % PGSIZE;
			if(pre_page > 0){
				size_t start_addr = PGSIZE - pre_page;
				size_t fillpre = PGSIZE - pre_page;
				vaddr = ROUNDUP(vaddr, PGSIZE);
				size_t freadpre, memreadpre;
				freadpre = memreadpre = 0;
				if(file < fillpre) freadpre = file, file = 0;
				else freadpre = fillpre, file -= freadpre;
				mem -= freadpre;
				if(freadpre < fillpre && mem > 0){
					fillpre -= freadpre;
					if(mem < fillpre) memreadpre = mem, mem = 0;
					else memreadpre = fillpre, mem -= memreadpre;
				}
				size_t loads = freadpre;
				fs_read(fd, page_cache + start_addr, freadpre);
				for(; loads < memreadpre + freadpre; ++loads) page_cache[loads + start_addr] = 0;
				memcpy(pa, page_cache, PGSIZE);
			}
			remain_num = mem  % PGSIZE;
			size_t pg_count = mem / PGSIZE;
			size_t fpg_count = file / PGSIZE;
			size_t fremain = file % PGSIZE;
			for(; j <= pg_count ; ++j){
				pa = new_page(1);
//				Log("run this");
				map(&pcb->as, (void *)vaddr, pa, 0);
				if(j <= fpg_count){
					fs_read(fd, page_cache, PGSIZE);
					memcpy(pa, page_cache, PGSIZE);
				}else{
					size_t l = 0;
					if(j == fpg_count + 1 && fremain > 0){
						fs_read(fd, page_cache, fremain);
						l = fremain;
					}
					for(; l < PGSIZE; ++l){
						page_cache[l] = 0;
					}
					Log("%p", pa);
					memcpy(pa, page_cache, PGSIZE);
				}
				vaddr += PGSIZE;
			}
			if(remain_num > 0){
				pa = new_page(1);
				map(&pcb->as, (void *)vaddr, pa, 0);
				if(fpg_count < pg_count){
					size_t u = 0;
					for(; u < remain_num; ++u){
						page_cache[u] = 0;
					}
				}else{
					size_t u = fremain;
					if(fremain > 0){
						fs_read(fd, page_cache, fremain);
					}
					for(; u < remain_num; ++u){
						page_cache[u] = 0;
					}
				}
				Log("%p", pa);
				memcpy(pa, page_cache, PGSIZE);
			}
		}
	}
	fs_close(fd);
	return elf_ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]){
  Log("%p", pcb);
  pcb->max_brk = 0;
  Elf_Ehdr elf_ehdr;
  int fd = fs_open(filename, 1, 1);
  size_t bytes = fs_read(fd, &elf_ehdr, sizeof(Elf_Ehdr));
  assert(bytes == sizeof(Elf_Ehdr));
  assert(*(uint32_t *)elf_ehdr.e_ident = 0x7f454c46);
  assert(elf_ehdr.e_machine == EXPECT_TYPE);
  fs_lseek(fd, 0, SEEK_SET);
  uintptr_t entry = elf_ehdr.e_entry;
  Area kustack;
  kustack.start = (void *)pcb;
  kustack.end = (void *)pcb + sizeof(PCB) - 1;
  pcb->cp = ucontext(&pcb->as, kustack, (void *)entry);
  void *ustack = new_page(8);
  void *curpos = pcb->as.area.end - STACK_SIZE;
  for(int i = 0; i < 8; ++i){
	map(&pcb->as, curpos + i * PGSIZE, ustack + i * PGSIZE, 0);
  }
  pcb->cp->gpr[10] = (uintptr_t)pcb->as.area.end;
//  Log("%p", pcb->cp->gpr[10]);
  uintptr_t actuall_addr = (uintptr_t)ustack + STACK_SIZE; 
  int argc = 0;
  int envpc = 0;
  uintptr_t str_len = 0;
  if(argv != NULL){ 
	while(*(argv + argc) != (char *)NULL) ++argc, str_len += (strlen(argv[argc - 1]) + 1);
  }
  if(envp != NULL){
	while(envp[envpc] != NULL) ++envpc, str_len += (strlen(envp[envpc - 1]) + 1);
  }
  str_len = ROUNDUP(str_len, 4); // align the address of int and pointer
  pcb->cp->gpr[10] -= (sizeof(int) + (argc + envpc + 2) * sizeof(uintptr_t) + str_len);
  uintptr_t* arg_env_pos = (uintptr_t *)(actuall_addr  - (sizeof(int) + (argc + envpc + 2) * sizeof(uintptr_t) + str_len));
  char* string_area = (char *)(actuall_addr - str_len);
//  Log("%p", arg_env_pos);
  *(int *)arg_env_pos = argc;
//  Log("%p", arg_env_pos);
//  Log("argc:%d", *arg_env_pos);
  ++arg_env_pos;
  for(int i = 0; i < argc; ++i, ++arg_env_pos){
	*arg_env_pos = (uintptr_t)string_area;  
	for(char* c = argv[i]; *c != '\0'; ++c, ++string_area){
	  *string_area = *c;
	}
	*string_area = '\0';
	++string_area;
//	Log("argv[%d]:%s", i, *(char **)arg_env_pos);
  }
  *arg_env_pos = (uintptr_t)NULL;
  ++arg_env_pos;
  for(int i = 0; i < envpc; ++i, ++arg_env_pos){
    *arg_env_pos = (uintptr_t)string_area;
	for(char* c = envp[i]; *c != '\0'; ++c, ++string_area){
	  *string_area = *c;
	}
	*string_area = '\0';
	++string_area;
//	Log("envp[%d]:%s", i, *(char **)arg_env_pos);
  }
  *arg_env_pos = (uintptr_t)NULL;
  // Log("%p", pcb->cp->gpr[11]);
  entry = loader(pcb, filename);
}
