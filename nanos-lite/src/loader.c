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

