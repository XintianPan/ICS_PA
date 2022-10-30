#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_NATIVE__)
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

size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
	Elf_Ehdr elf_ehdr;
	Elf_Phdr elf_phdr;
	size_t byte;
	byte = ramdisk_read(&elf_ehdr, 0, sizeof(Elf_Ehdr));
	assert(byte == sizeof(Elf_Ehdr));
	assert(*(uint32_t *)elf_ehdr.e_ident = 0x7f454c46);
	assert(elf_ehdr.e_machine == EXPECT_TYPE);
    size_t i = 0;
//	size_t file, mem;
	uint32_t off = elf_ehdr.e_phoff;
	for(; i < elf_ehdr .e_phnum; ++i){
		byte = ramdisk_read(&elf_phdr, off + i * (sizeof(Elf_Phdr)), sizeof(Elf_Phdr));
	    assert(byte == sizeof(Elf_Phdr));
	    if(elf_phdr.p_type == PT_LOAD){
			
		}
	}
	return 0;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

