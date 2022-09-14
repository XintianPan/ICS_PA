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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_AND, TK_NUM, TK_L, TK_R

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\(", TK_L},			// left bracket
  {"\\)", TK_R},			// right bracket 
  {"\\+", '+'},         // plus
  {"-", '-'},			// minus or negative
  {"==", TK_EQ},        // equal
  {"&&", TK_AND},		// and
  {"[0-9]+", TK_NUM},	// number
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
     if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  } 
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e, int *endpos) {
  int position = 0;
  int i;
  int stacknum = 0;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
			case TK_NOTYPE: break;
			case TK_NUM:	if(substr_len > 31) panic("buffer overflow: integer is too big");
							++(*endpos);
							for(register int j = 0; j < substr_len; ++j) tokens[*endpos].str[j] = substr_start[j];
							tokens[*endpos].type = TK_NUM;
							break;
			case TK_L:	stacknum += 1;
						++(*endpos);
						tokens[*endpos].type = TK_L;
						break;
			case TK_R:	stacknum -= 1;
						if(stacknum < 0) panic("Invaild expression: Bracket not matched(Right Bracket)");
						++(*endpos);
						tokens[*endpos].type = TK_R;
						break;
			case '+':	++(*endpos);
						tokens[*endpos].type = '+';
						break;
			case '-':	++(*endpos);
						tokens[*endpos].type = '-';
		   	default: break;				   

        break;
       }
    } 
		if (i == NR_REGEX) {
		 printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
		 return false;
		 }
	}
  }
	if(stacknum > 0){
	panic("Invalid Expression: Bracket not matched(Left)");
	}
  return true;
}


word_t expr(char *e, bool *success) {
	int endpos = -1;
  if (!make_token(e, &endpos)) {
    *success = false;
    return 0;
  }
	printf("%d\n", endpos);
  /* TODO: Insert codes to evaluate the expression. */
	word_t ret = eval(0, endpos);
  return ret;
}
static word_t eval(int start, int end){
	word_t ret = 0;
	if(start > end){
		panic("Invaild Expression");
		return 0;
	}else if(start == end){
		if(tokens[start].type == TK_NUM){
			ret = atoi(tokens[start].str);
			return ret;
		}else{
			panic("This is not a number");
			return 0;
		}
	}
	return 0;
}
