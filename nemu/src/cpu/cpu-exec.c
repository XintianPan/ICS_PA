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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();
bool ifchange();


#ifdef CONFIG_FTRACE

extern Func_info elf_func[2048];
extern int elf_func_num;


typedef struct __func_trace{
	char func_name[64];
	paddr_t cur_addr;
	paddr_t func_addr;
	size_t size;
	int type;
    int space_len;
	struct __func_trace *nxt;

} __FUNC_TRACE;

__FUNC_TRACE *ft_head = NULL;
__FUNC_TRACE *ft_tail = NULL;

void destruct(){
	__FUNC_TRACE *pre;
	while(ft_head != NULL){
		pre = ft_head;
		ft_head = ft_head->nxt;
		free(pre);
	} 
	ft_head = ft_tail = NULL;
}

void f_trace(){
	__FUNC_TRACE *cur = ft_head;
	while(cur != NULL){
	//	printf("0x%08x:", cur->cur_addr);
		log_write("0x%08x:", cur->cur_addr);
		for(int i = 0; i < cur->space_len; ++i){
		//	printf("  ");
			log_write("  ");
		}
		if(cur->type == 1){
		//	printf("call[%s@0x%08x]\n", cur->func_name, cur->func_addr);
			log_write("call[%s@0x%08x]\n", cur->func_name, cur->func_addr);
		}else{
		//	printf("ret[%s]\n", cur->func_name);
			log_write("ret[%s]\n", cur->func_name);
		}
		cur = cur->nxt;
	}
}

void trace_rec(paddr_t cur_pc, paddr_t jmp_addr){
	int c_addr = -1; int n_addr = -1;
	for(int i = 0; i < elf_func_num; ++i){
		if(elf_func[i].start_addr <= cur_pc && cur_pc < elf_func[i].start_addr + elf_func[i].func_size){
			c_addr = i;
			break;
		}
	}
	for(int i = 0; i < elf_func_num; ++i){
		if(elf_func[i].start_addr <= jmp_addr && jmp_addr < elf_func[i].start_addr + elf_func[i].func_size){
			n_addr = i;
			break;
		}
	}
	if(n_addr == c_addr)
		return;
	else{
		if(jmp_addr == elf_func[n_addr].start_addr){
			if(ft_head == NULL){
				ft_head = (__FUNC_TRACE *)malloc(sizeof(__FUNC_TRACE));
			    strcpy(ft_head->func_name, elf_func[n_addr].func_name);
				ft_head->cur_addr = cur_pc;
			    ft_head->space_len = 0;
			    ft_head->func_addr = jmp_addr;
				ft_head->type = 1;
			    ft_head->nxt = NULL;
			    ft_tail = ft_head;	
			}else{
				ft_tail->nxt = (__FUNC_TRACE *)malloc(sizeof(__FUNC_TRACE));
				if(ft_tail->type == 1)
					ft_tail->nxt->space_len = ft_tail->space_len + 1;
				else
					ft_tail->nxt->space_len = ft_tail->space_len;
				ft_tail = ft_tail->nxt;
				strcpy(ft_tail->func_name, elf_func[n_addr].func_name);
				ft_tail->cur_addr = cur_pc;
				ft_tail->func_addr = jmp_addr;
				ft_tail->type = 1;
				ft_tail->nxt = NULL;
			}	
		}else{
				if(ft_head == NULL){
				ft_head = (__FUNC_TRACE *)malloc(sizeof(__FUNC_TRACE));
			    strcpy(ft_head->func_name, elf_func[n_addr].func_name);
				ft_head->cur_addr = cur_pc;
			    ft_head->space_len = 0;
			    ft_head->func_addr = jmp_addr;
				ft_head->type = 0;
			    ft_head->nxt = NULL;
			    ft_tail = ft_head;	
			}else{
				ft_tail->nxt = (__FUNC_TRACE *)malloc(sizeof(__FUNC_TRACE));
				if(ft_tail->type == 0)
					ft_tail->nxt->space_len = ft_tail->space_len - 1;
				else
					ft_tail->nxt->space_len = ft_tail->space_len;
				ft_tail = ft_tail->nxt;
				strcpy(ft_tail->func_name, elf_func[n_addr].func_name);
				ft_tail->cur_addr = cur_pc;
				ft_tail->type = 0;
				ft_tail->nxt = NULL;
			}	
		
		}
	} 
}
#endif

bool jmp_check = false;


#ifdef CONFIG_IRINGBUF
#define BUFLEN 20
static char iringbuf[BUFLEN][128];
static int buf_index = -1;
#endif


static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
#ifdef CONFIG_WATCHPOINT	
  if (ifchange()) {nemu_state.state = NEMU_STOP; return;}
#endif
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
#ifdef CONFIG_FTRACE
  if (jmp_check) trace_rec(_this->pc, dnpc);
#endif

}

static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  jmp_check = false;
  isa_exec_once(s, &jmp_check);
  cpu.pc = s->dnpc;
#if defined(CONFIG_ITRACE) || defined(CONFIG_IRINGBUF)
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
#endif
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }  
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#endif
#ifdef CONFIG_IRINGBUF
  buf_index = (buf_index + 1) % BUFLEN;
  char *buf = iringbuf[buf_index];
  memset(buf, 0, sizeof(iringbuf[buf_index]));
  buf += snprintf(buf, sizeof(iringbuf[buf_index]), FMT_WORD ":", s->pc);
  int inslen = s->snpc - s->pc;
  int j;
  uint8_t *instr = (uint8_t *)&s->isa.inst.val;
  for(j = inslen - 1; j >= 0; --j){
    buf += snprintf(buf, 4, " %02x", instr[j]);
  }
  int inslen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int s_len = inslen_max - inslen;
  if(s_len < 0) s_len = 0;
  s_len = s_len * 3 + 1;
  memset(buf, ' ', s_len);
  buf += s_len;
  disassemble(buf, iringbuf[buf_index] + sizeof(iringbuf[buf_index]) - buf,
		 MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, inslen); 
#endif
}

static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  } 
}

static void statistic() {
  #ifdef CONFIG_FTRACE
    f_trace();
	destruct();
  #endif
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
} 

#ifdef CONFIG_IRINGBUF
static void iringbufshow(){
	for(int i = 0; i < BUFLEN; ++i){
		if(iringbuf[i][0] == '\0')
			break;
		if(i != buf_index){
			printf("    %s\n", iringbuf[i]);
			log_write("    %s\n", iringbuf[i]);
		}
		else{
			printf("--> %s\n", iringbuf[i]);
			log_write("--> %s\n", iringbuf[i]);
		}
	}
}
#endif

void assert_fail_msg() {
  printf("Register Value:\n");
  isa_reg_display();
#ifdef CONFIG_IRINGBUF
  puts("Recent called instructions:");
  iringbufshow();
#endif
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
