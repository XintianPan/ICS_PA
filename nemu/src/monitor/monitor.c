/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>
#include <elf.h>


#ifdef CONFIG_FTRACE

Func_info elf_func[2048];

int elf_func_num = 0;
#endif
void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
#ifdef CONFIG_FTRACE
static char *elf_file = NULL;
#endif
static int difftest_port = 1234;

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

#ifdef CONFIG_FTRACE
static void fetch_elf() {
	FILE *fp;
	fp = fopen(elf_file, "rb");
	if(fp == NULL){  
		Log("No elf file found");
	}else{
		Elf32_Ehdr elf_head;
		uint32_t num_byte;
		num_byte = fread(&elf_head, sizeof(Elf32_Ehdr), 1, fp);
		assert(num_byte);
		Elf32_Shdr *elf_shdr = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr) * elf_head.e_shnum);
		assert(elf_shdr != NULL);
		fseek(fp, elf_head.e_shoff, SEEK_SET);
		num_byte = fread(elf_shdr, sizeof(Elf32_Shdr) * elf_head.e_shnum, 1, fp);
		assert(num_byte);
		Elf32_Sym *elf_sym = NULL;
		char *elf_str = NULL;
		size_t sym_size = 0;
		for(int i = 0; i < elf_head.e_shnum; ++i){
			if(elf_shdr[i].sh_type == SHT_SYMTAB){
				elf_sym = (Elf32_Sym *)malloc(elf_shdr[i].sh_size);
				assert(elf_sym);
				sym_size = elf_shdr[i].sh_size / elf_shdr[i].sh_entsize;
				rewind(fp);
				fseek(fp, elf_shdr[i].sh_offset, SEEK_SET);
			    num_byte = fread(elf_sym, elf_shdr[i].sh_size, 1, fp);
				assert(num_byte);
		 	}
		    else if(elf_shdr[i].sh_type == SHT_STRTAB){
				if(i == elf_head.e_shstrndx)
					continue;
			 	else{ 
					elf_str = (char *)malloc(elf_shdr[i].sh_size);
					assert(elf_str);
					rewind(fp);
					fseek(fp, elf_shdr[i].sh_offset, SEEK_SET);
					num_byte = fread(elf_str, elf_shdr[i].sh_size, 1, fp); 
					assert(num_byte);
			 	}
			}  
		} 
		char *temp;
		elf_func_num = 0;
		for(int i = 0; i < sym_size; ++i){
			if(ELF32_ST_TYPE(elf_sym[i].st_info) == STT_FUNC){
				temp = elf_str + elf_sym[i].st_name;
				strcpy(elf_func[elf_func_num].func_name, temp);
				elf_func[elf_func_num].start_addr = elf_sym[i].st_value;
				elf_func[elf_func_num].func_size = elf_sym[i].st_size;
				++elf_func_num;
		  	}
		}  
		free(elf_sym);
		free(elf_str);
		free(elf_shdr);
		fclose(fp);
 	}
}
#endif
static int parse_args(int argc, char *argv[]) {
   const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
	{"elf"		, required_argument, NULL, 'e'},
	{"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhe:l:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
	  case 'e': 
#ifdef CONFIG_FTRACE
				elf_file = optarg;
#endif				
				break;
      case 'd': diff_so_file = optarg; break;
      case 1: img_file = optarg;
			  return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

#ifdef CONFIG_FTRACE  
  fetch_elf();
#endif

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv32, "riscv32",
    MUXDEF(CONFIG_ISA_riscv64, "riscv64", "bad")))) "-pc-linux-gnu"
  ));

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
