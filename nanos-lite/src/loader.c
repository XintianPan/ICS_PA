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

int fs_open(const char *pathname, int flags, int mode);

int fs_close(int fd);

size_t fs_read(int fd, void *buf, size_t len);

size_t fs_lseek(int fd, size_t offset, int whence);

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
	size_t file, mem;
	uint8_t byte;
	uint32_t vaddr;
	uint32_t off = elf_ehdr.e_phoff;
 	for(; i < elf_ehdr.e_phnum; ++i){
		fs_lseek(fd, off + i * (sizeof(Elf_Phdr)), SEEK_SET);
		bytes = fs_read(fd, &elf_phdr, sizeof(Elf_Phdr));
	    assert(bytes == sizeof(Elf_Phdr));
 	    if(elf_phdr.p_type == PT_LOAD){
			vaddr = elf_phdr.p_vaddr;
		    file = elf_phdr.p_filesz;
			mem = elf_phdr.p_memsz;
			j = 0;
			fs_lseek(fd, elf_phdr.p_offset, SEEK_SET);
			for(; j < file; ++j){
				fs_read(fd, &byte, 1);
				*(volatile uint8_t *)(vaddr + j) = byte;
			}
			for(; j < mem; ++j) *(volatile uint8_t *)(vaddr + j) = 0;	
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
  uintptr_t entry = loader(pcb, filename);
  Area kustack;
  kustack.start = (void *)pcb;
  kustack.end = (void *)pcb + sizeof(PCB) - 1;
  pcb->cp = ucontext(&pcb->as, kustack, (void *)entry);
  int argc = 0;
  int envpc = 0;
  if(argv != NULL){ 
	while(*(argv + argc) != (char *)NULL) ++argc, Log("%d", argc);
  }
  if(envp != NULL){
	while(envp[envpc] != NULL) ++envpc;
  }
  int *argc_pos = (int *)pcb->cp->GPRx;
  Log("%p", pcb->cp->GPRx);
  uintptr_t *arg_env_pos = (uintptr_t *)((void *)pcb->cp->GPRx + sizeof(int));
  Log("%p", argc_pos);
  *argc_pos = argc;
  char *string_area = (char *)((void *)pcb->cp->GPRx + sizeof(int) + argc + envpc + 2);
  for(int i = 0; i < argc; ++i, ++arg_env_pos){
    *arg_env_pos = (uintptr_t)string_area;
	for(char *c = argv[i]; *c != '\0'; ++c, ++string_area){
	  *string_area = *c;
	}
	*string_area = '\0';
	Log("%s", (char *)(arg_env_pos));
	++string_area;	
  }
  *arg_env_pos = (uintptr_t)NULL;
  ++arg_env_pos;
  for(int i = 0; i < envpc; ++i, ++arg_env_pos){
    *arg_env_pos = (uintptr_t)string_area;
	for(char *c = envp[i]; *c != '\0'; ++c, ++string_area){
	  *string_area = *c;
	}
	*string_area = '\0';
	++string_area;	
  }
 *arg_env_pos = (uintptr_t)NULL; 
}
