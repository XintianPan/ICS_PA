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

#ifdef CONFIG_TARGET_NATIVE_ELF
  char etr_buf[1024];
  bool etr_en = false;
#endif

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger a n  interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
//  printf("exception %u\n", NO);
#ifdef CONFIG_ETRACE
	    Log("fuck\nfuck\n");
		switch(NO){  
			case 0xffffffff: sprintf(etr_buf,"Excection happen at 0x%08x, Number: 0x%08x, Event name: %s", cpu.pc, NO, "Yeild");
							 etr_en = true;
			                 break;
		}
#endif
  cpu.sys[3] = NO;
  cpu.sys[2] = cpu.pc;
  return cpu.sys[epc];
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
