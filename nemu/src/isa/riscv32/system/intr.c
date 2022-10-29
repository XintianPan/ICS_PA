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

#ifdef CONFIG_ETRACE
	char etr_buf[20][256];
	int id = -1;
#endif

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an  interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
//  printf("exception %u\n", NO);
#ifdef CONFIG_ETRACE
	if(id != 19){
		puts("here");
		printf("0x%08x %d\n", NO, id);
		switch(NO){
			case 0xffffffff: sprintf(etr_buf[++id], "Expection happen at 0x%08x, Number: 0x%08x, Event name: %s\n", cpu.pc, NO, "Yeild"); break;
		}
		printf("0x%08x %d\n", NO, id);
	}else{
		for(int i =  0; i < 20; ++i){
			log_write("%s", etr_buf[i]);
		}
		id = -1;
		switch(NO){
			case 0xffffffff: sprintf(etr_buf[++id], "Expection happen at 0x%08x, Number: 0x%08x, Event name: %s\n", cpu.pc, NO, "Yeild"); break;
		}
	}
#endif
  cpu.sys[3] = NO;
  cpu.sys[2] = cpu.pc;
  return cpu.sys[epc];
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
