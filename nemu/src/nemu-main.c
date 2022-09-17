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

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  FILE *fp = fopen("../nemu/tools/gen-expr/build/input", "r");
  if(fp == NULL)
	  puts("No file!");
  else{ 
	 word_t ret = 0;
	word_t cmp = 0;
   puts("OK");
	char buf[65536];   
	while(fscanf(fp,"%s\n", &cmp, buf) != EOF){
		ret = atoi(strtok(buf, " "));
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
	fclose(fp);
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
