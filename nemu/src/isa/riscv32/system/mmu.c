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
#include <memory/vaddr.h>

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
	paddr_t pg_off = BITS(vaddr, 11, 0);
	vaddr_t vpn1 = BITS(vaddr, 31, 22);
	vaddr_t vpn0 = BITS(vaddr, 21, 12);
	paddr_t dir = BITS(cpu.sys[4], 19, 0);
	dir <<= 12;
//	printf("dir: 0x%x\n", dir);
	paddr_t pte = paddr_read(dir + vpn1 * 4, 4);
	if((pte & 1) != 1){
		printf("0x%08x\n", cpu.sys[5]);
		printf("0x%08x\n", vaddr);
		printf("0x%08x\n", cpu.pc);
		printf("%d\n", type);
		assert(0);
	}
	paddr_t ppn = pte >> 10;
	ppn <<= 12;
//	printf("ppn: 0x%x\n", ppn);
	paddr_t pte0 = paddr_read(ppn + vpn0 * 4, 4);
	paddr_t pg = pte0 >> 10;
//	printf("final page: 0x%x\n", (pg << 12));
	paddr_t paddr = (pg << 12) | pg_off;
	if(paddr == 0x800029e8 && type != 0){
      Log("0x%08x %d", cpu.pc, type);
	}
	return paddr;
}

int isa_mmu_check(vaddr_t vaddr, int len, int type){
	if(BITS(cpu.sys[4], 31, 31))
	   return MMU_TRANSLATE;
	else
	   return MMU_DIRECT;	
}
