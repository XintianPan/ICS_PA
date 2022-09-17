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
#ifdef CONFIG_EXPRTEST
	FILE *fp = fopen("../nemu/tools/gen-expr/build/input", "r");
  if(fp == NULL)
	  puts("No file!");
  else{ 
	  word_t ret = 0;
	  char buf[300] = {};
	  memset(buf, 0, sizeof buf);
	  if(fscanf(fp, "%u %[^\n]%*c", &ret, buf) != EOF){
		  printf("%u %s\n", ret, buf);
		  bool succ = false;
		  word_t cmp = expr((char *)buf, &succ);
		  if(succ)
			  if(cmp == ret)
				  puts("YES");
		  memset(buf, 0, sizeof buf);
 	  }
 	}
  fclose(fp);
#endif

#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
