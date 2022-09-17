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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char numbuf[33] = {};
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static void gen_num(int start, int end){
	if(start == end)
		buf[start] = '1';
	uint32_t chonum = rand();
	int count = 0;
	while(chonum != 0){
		numbuf[count] = chonum % 10 + '0';
		count += 1;
		chonum /= 10;
	}
	int endpos = (count > (end - start)) ? end : count + start; 
	for(int i = start; i < endpos; ++i)
		buf[i] = numbuf[count - 1], --count;
	buf[endpos] = 'U';
	if(endpos != end){
		for(int i = endpos + 1; i <= end; ++i)
			buf[i] = ' ';
	} 
}

static char gen_op(){
	int cho = rand() % 4;
	switch(cho){
		case 0:
			return '+';
		case 1:
			return '-';
		case 2:
			return '*';
		case 3:
			return '/';
		default:
			break;
	} 
	return 0;
}

static void gen_rand_expr(int start, int end) {
 if(start > end || end == start + 1)
	 return;
  int choice = rand() % 3;
  switch(choice){
    case 0:
		gen_num(start, end);
		break;
	case 1:
		if(end != start + 1){
			buf[start] = 40;
			buf[end] = 41;
			gen_rand_expr(start + 1, end - 1);
		}else{
			return;
 		}
		break;
	case 2:
		int mid = (start + end) / 2;
		buf[mid] = gen_op();
		gen_num(start, mid -1);
		gen_num(mid + 1, end);
		break;
	default: break;
  }
  
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  for(int i = 0; i < 65535; ++i)
	  buf[i] = ' ';
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  } 
  int i;
  for (i = 0; i < loop; i ++) {
		buf[300] = '\0';
	  gen_rand_expr(0, 299);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr -Wall -Werror");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    if(fscanf (fp, "%d", &result) != EOF){
    pclose(fp);

    printf("%u %s\n", result, buf);
	}
  }
  return 0;
}
