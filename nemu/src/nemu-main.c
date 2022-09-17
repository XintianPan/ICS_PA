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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
word_t expr(char* e, bool* success);
char buf[65536 + 128];

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  FILE *fp = fopen("../nemu/tools/gen-expr/build/input", "r");
  if(fp == NULL)
	  puts("No file!");
  else{
	 word_t ret;
	word_t cmp = 0; 
	while(fscanf(fp,"%d %s",cmp, buf) != EOF){
		bool succ = false;
		ret = expr(buf, &succ);
		if(succ){
			if(cmp == ret)
				puts("YES");
			else
				puts("NO");
		}else
			puts("NO");
	}
  }
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
