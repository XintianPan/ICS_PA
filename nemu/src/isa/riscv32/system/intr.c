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
#include <utils.h>

#ifdef CONFIG_TRACE
  char etr_buf[1024];
  bool etr_en = false;
#endif

#define IRQ_TIMER 0x80000007
#define MIE_CLEAR 0xfffffff7

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger a n  interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
//  printf("exception %u\n", NO);
#ifdef CONFIG_ETRACE
		switch(NO){  
			case 0xffffffff: log_write("Excection happen at 0x%08x, Number: 0x%08x, Event name: %s\n", cpu.pc, NO, "Yeild");
			                 break;
		}
#endif
  word_t mie = BITS(cpu.sys[0], 3, 3);
  cpu.sys[0] = cpu.sys[0] & MIE_CLEAR;
  cpu.sys[0] |= (mie << 7);
  cpu.sys[3] = NO;
  cpu.sys[2] = cpu.pc;
  return cpu.sys[epc];
}

word_t isa_query_intr() {
//	if(BITS(cpu.sys[0], 3, 3) == 0) puts("closed");
	if(cpu.INTR && (BITS(cpu.sys[0], 3, 3))){
		cpu.INTR = false;
		return IRQ_TIMER;
	}
	return INTR_EMPTY;
}
