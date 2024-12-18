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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
void print_wp();
void delete_wp(int num);
void add_wp(char* express);
word_t vaddr_read(vaddr_t addr, int len);
word_t expr(char *e, bool *success);
word_t paddr_read(paddr_t addr, int len);
void paddr_write(paddr_t addr, int len, word_t data);


/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
	nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_p(char *args){
	if(args == NULL){
		puts("No args!");
	}else{
		char *express = strtok(NULL, "");
		bool succ = false;
		word_t ret = expr(express, &succ);
		if(!succ)
			puts("Bad expression");
		else
			printf("%u\n", ret);
	}
	return 0;
}
#ifdef CONFIG_WATCHPOINT
static int cmd_w(char *args){
	if(args == NULL){
		puts("No args!");
	}else{
		char *express = strtok(NULL, "");
		add_wp(express);
	}
	return 0;
}
#endif
static int cmd_d(char *args){
	if (args == NULL){
		puts("No args!");
	}else{
		char *number = strtok(NULL, " ");
		int num = atoi(number);
		delete_wp(num);
	}
	return 0;
}
static int cmd_si(char *args){
	char *number = strtok(NULL, " ");
	if(number == NULL)
		cpu_exec(1);
	else
		cpu_exec(atoi(number));
	return 0;
}

static int cmd_save(char *args){
	FILE* fp = fopen("/home/paxintic/nemu-save.txt", "w+");
	fprintf(fp, "%u\n", cpu.pc);
	for(int i = 0; i < CONFIG_MSIZE; ++i){
		fprintf(fp, "%u\n", paddr_read(CONFIG_MBASE + i, 1));	
	}
	return 0;
}

static int cmd_load(char *args){
	FILE* fp = fopen("/home/paxintic/nemu-save.txt", "r");
	word_t val;
	int r;
	r = fscanf(fp, "%u", &val);
	cpu.pc = val;
	for( int i = 0; i < CONFIG_MSIZE; ++i){
		r = fscanf(fp, "%u\n", &val);
		r += 1;
		paddr_write(CONFIG_MBASE + i, 1, val);
	}
	return 0;
}

static int cmd_info(char *args){
	if( args == NULL){
		printf("please pass parameter!\n");
		return 0;
	}
	char *status = strtok(NULL, " ");
	if(strcmp("r", status) == 0){
		isa_reg_display();
	}
#ifdef CONFIG_WATCHPOINT
	else if(strcmp("w", status) == 0){
		print_wp();
	}
#endif	
	else{
		printf("Invalid parameter\n");
	} 
	return 0;
}

static int cmd_x(char *args){
	char *number = strtok(args, " ");
	if(number == NULL){
		printf("No parameter received\n");
		return 0;
	}else{
	 	char *express = strtok(NULL, "");
		if(express  == NULL){
			printf("Please give an valid address!\n");
		}else{
			int n = atoi(number);
				// vaddr_t addr = strtol(args + 2, NULL, 16);
			bool succ = false;
			vaddr_t addr = expr(express, &succ);
			if(succ){
				for(int i = 0; i < n; ++i){
					printf("address: 0x%08x\t\tvalue: 0x%08x\n", addr, vaddr_read(addr, 4));
					addr += 4;
				} 
				return 0;
			}else{
				printf("Invalid expression!\n");
				return 0;
			}	
 		}
 	}
	return 0;
} 

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute program with the given number of steps(default value 1)", cmd_si },
  { "info", "Check registers' or watch point's information", cmd_info },
  { "x", "Scan memory", cmd_x },
  { "p", "Evaluate expression", cmd_p},
#ifdef CONFIG_WATCHPOINT
  { "w", "Add watchpoint", cmd_w},
#endif
  { "d", "Delete a watchpoint", cmd_d},
  { "save", "save state", cmd_save},
  { "load", "load state", cmd_load},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
