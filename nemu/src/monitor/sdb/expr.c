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
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_AND, TK_NUM, TK_HEX, TK_L, TK_R, TK_NEG, TK_DR, TK_REG, TK_PC,

};

enum{
	OP_NEGPTR = 1,  OP_MD, OP_PM,  OP_EQN, OP_AND,
};

word_t vaddr_read(vaddr_t addr, int len);

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {


  {" +", TK_NOTYPE},    // spaces
  {"\\(", TK_L},			// left bracket
  {"\\)", TK_R},			// right bracket 
  {"\\+", '+'},         // plus
  {"-", '-'},  // minus or negative
  {"/", '/'},			// division
  {"\\*", '*'},						// multiply
  {"==", TK_EQ},        // equal
  {"!=", TK_NEQ},						// not equal
  {"&&", TK_AND},		// and
  {"0[x,X][0-9,a-f,A-F]+", TK_HEX},		// hex num
  {"[0-9]+U*", TK_NUM},	// number
  {"(\\$)([\\$,r,s,g,t,s,a])([0-9,a,p])([0,1]*)", TK_REG},						// register
  {"\\$pc", TK_PC},		//PC counter  
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

static Token tokens[256] __attribute__((used)) = {};
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


        switch (rules[i].token_type) {
			case TK_NOTYPE: break;
			case TK_PC:		++(*endpos);
							for(register int j = 0; j < substr_len; ++j) tokens[*endpos].str[j] = substr_start[j];
							tokens[*endpos].type = TK_REG;
							tokens[*endpos].str[substr_len] = '\0';
							break;
			case TK_REG:	++(*endpos);
							if(substr_len > 32) {puts("buffer overflow: register name"); return false;}
							for(register int j = 0; j < substr_len - 1; ++j) tokens[*endpos].str[j] = substr_start[j + 1];
							tokens[*endpos].type = TK_REG;
							tokens[*endpos].str[substr_len - 1] = '\0';
							break;
			case TK_NUM:	if(substr_len > 31) {puts("buffer overflow: integer is too big"); return false;}
							++(*endpos);
							for(register int j = 0; j < substr_len; ++j) tokens[*endpos].str[j] = substr_start[j];
							tokens[*endpos].type = TK_NUM;
							tokens[*endpos].str[substr_len] = '\0';
							break;
			case TK_HEX:	if(substr_len > 33) {puts("buffer overflow: hex num is too big");return false;}
							++(*endpos);
							for(register int j = 0; j < substr_len - 2; ++j) tokens[*endpos].str[j] = substr_start[j + 2];
							tokens[*endpos].type = TK_HEX;
							tokens[*endpos].str[substr_len - 2] = '\0';
							break;	
			case TK_L:	stacknum += 1;
						++(*endpos);
						tokens[*endpos].type = TK_L;
						break;
			case TK_R:	stacknum -= 1;
						if(stacknum < 0) {puts("Invaild expression: Bracket not matched(Right Bracket)"); return false;}
						++(*endpos);
						tokens[*endpos].type = TK_R;
						break;
			case '+':	++(*endpos);
						tokens[*endpos].type = '+';
						break;
			case '-':	++(*endpos);
						tokens[*endpos].type = '-';
						break;
			case '*':	++(*endpos);
						tokens[*endpos].type = '*';
						break;
			case '/':	++(*endpos);
						tokens[*endpos].type = '/';
						break;
			case TK_EQ: ++(*endpos);
						tokens[*endpos].type = TK_EQ;
						break;
			case TK_NEQ: ++(*endpos);
						tokens[*endpos].type = TK_NEQ;
						break;
			case TK_AND: ++(*endpos);
						tokens[*endpos].type = TK_AND;
						break;	 
		
			default: break;				   

       }
		break;
	 } 
	}
	 if (i == NR_REGEX) {
		 printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
		 return false;
		 }
  } 
	if(stacknum > 0){
	puts("Invalid Expression: Bracket not matched(Left)");
	return false;
	}
	for(int i = 0; i <= *endpos; ++i){
		if(tokens[i].type == '-'){
			if((i == 0) || ((tokens[i-1].type != TK_NUM) && (tokens[i-1].type != TK_R) && (tokens[i-1].type != TK_HEX) && (tokens[i-1].type != TK_REG) && (tokens[i-1].type != TK_PC))){
				tokens[i].type = TK_NEG;
		  	}
		}  
		if(tokens[i].type == '*'){
			if((i == 0) || ((tokens[i-1].type != TK_NUM) && (tokens[i-1].type != TK_R) && (tokens[i-1].type != TK_HEX) && (tokens[i-1].type != TK_REG) && (tokens[i-1].type != TK_PC))){
				tokens[i].type = TK_DR;
			}
		}
	}
//	for(int i = 0; i <= *endpos; ++i)
//		printf("%d\n", tokens[i].type);
	return true; 
}

static word_t eval(int start, int end, bool *success);

static bool ifmatched(int pos);

word_t expr(char *e, bool *success) {
	int endpos = -1;
  if (!make_token(e, &endpos)) {
    *success = false;
    return 0;
  }  
	*success = true;
	word_t ret;
	ret = eval(0, endpos, success);
  return ret;
}

static word_t eval(int start, int end, bool *success){
	word_t ret = 0;
	if(!(*success))
		return 0;
	if(start > end){ 
		*success = false;
		printf("Invaild Expression\n");
		return 0;
	}else if(start == end){
		if(tokens[start].type == TK_NUM){
			ret = atoi(tokens[start].str);
			return ret;
		}else if(tokens[start].type == TK_HEX){
			ret = strtol(tokens[start].str, NULL, 16);
			return ret;
		}else if(tokens[start].type == TK_REG){
			bool succ = false;
			ret = isa_reg_str2val(tokens[start].str, &succ);
			if(!succ)
				*success = false, printf("no such register!\n");
			return ret;
		}else{
			*success = false;
			puts("This is not a number");
			return 0;
		} 
	}else{
		if (tokens[start].type == TK_L && tokens[end].type == TK_R){
			int s = 1;
			bool flag = true;
			for(int i = start + 1; i <= end - 1; ++i){
				if(tokens[i].type == TK_L) ++s;
				if(tokens[i].type == TK_R) --s;
				if(s == 0){
					flag = false;
					break;
				}
			}	
			if(flag)
				return eval(start + 1, end - 1, success);
		}
		int pri = -1;
		int index = -1;
		int num = 0;
		for(int i = start; i <= end; ++i){
		//	printf("stack: %d, index: %d\n", num, i);
	 		switch(tokens[i].type){
				case '+':
				   if(i == start)
					  *success = false,  puts("bad expression: + nothing to match on left");
				   else if(i == end)
					   *success = false, puts("bad expression: + nothing to match on right");
	 			   else{
	 				   if(num == 0){
						 //  printf("reach here %d \n", i);
	 					if(ifmatched(i)){
	 						if(pri <= OP_PM){
								pri = OP_PM;
								index = i;
					 		}
						}else{
							*success = false;
							puts("bad expression");
					 	}	
					    }
				   }  
				   break;
				case '-':
				   if(i == start)
					  *success = false,  puts("bad expression: - nothing to match on left");
				   else if(i == end)
					   *success = false, puts("bad expression: - nothing to match on right");
	 			   else{
	 				   if(num == 0){
						 //  printf("reach here %d \n", i);
	 					if(ifmatched(i)){
	 						if(pri <= OP_PM){
								pri = OP_PM;
								index = i;
							}
						}else{
							*success = false;
							puts("bad expression");
						}	
					   }
				   } 
				   break;
				case '*':
				   if(i == start)
					  *success = false,  puts("bad expression: * nothing to match on left");
				   else if(i == end)
						   *success = false, puts("bad expression: / nothing to match on right");
				   else{
					if(num == 0){
						if(ifmatched(i)){
							if(pri <= OP_MD){
								pri = OP_MD;
								index = i;
							}
						}else{
							*success = false;
							puts("bad expression *");
						}
					} 	
				   } 
				   break;
				case '/':		
				   if(i == start)
					  *success = false,  puts("bad expression: / nothing to match on left");
				   else if(i == end)
					   	   *success = false, puts("bad expression: / nothing to match on right");
	 			   else{
	 				if(num == 0){
	 					if(ifmatched(i)){
	 						if(pri <= OP_MD){
								pri = OP_MD;
								index = i;
							}
						}else{
							*success = false;
							puts("bad expression");
						}
					} 	
				   } 
				   break;
				case TK_EQ:		
				   if(i == start)
					  *success = false,  puts("bad expression: ==  nothing to match on left");
				   else if(i == end)
					   	   *success = false, puts("bad expression: == nothing to match on right");
	 			   else{
	 				if(num == 0){
	 					if(ifmatched(i)){
	 						if(pri <= OP_EQN){
								pri = OP_EQN;
								index = i;
							}
						}else{
							*success = false;
							puts("bad expression");
						}
					} 	
				   } 
				   break;
				case TK_NEQ:		
				   if(i == start)
					  *success = false,  puts("bad expression: !=  nothing to match on left");
				   else if(i == end)
					   	   *success = false, puts("bad expression: != nothing to match on right");
	 			   else{
	 				if(num == 0){
	 					if(ifmatched(i)){
	 						if(pri <= OP_EQN){
								pri = OP_EQN;
								index = i;
							}
						}else{
							*success = false;
							puts("bad expression");
						}
					} 	
				   } 
				   break;
				case TK_AND:		
				   if(i == start)
					  *success = false,  puts("bad expression: &&  nothing to match on left");
				   else if(i == end)
					   	   *success = false, puts("bad expression: && nothing to match on right");
	 			   else{
	 				if(num == 0){
	 					if(ifmatched(i)){
	 						if(pri <= OP_AND){
								pri = OP_AND;
								index = i;
							}
						}else{
							*success = false;
							puts("bad expression");
						}
					} 	
				   } 
				   break;
				case TK_NEG:
				   if(num == 0){
					if(pri < OP_NEGPTR){
						pri = OP_NEGPTR;
						index = i;
	 				}	
	 			   } 
				   break;
				case TK_DR:
				   if(num == 0){
					if(pri < OP_NEGPTR){
						pri = OP_NEGPTR;
						index = i;
					}
	 			   }
				   break;
				case TK_NUM:
				   break;
				case TK_L:
				  ++num;
				   break;
				case TK_R:
				   --num;
				   break;
				case TK_HEX:
				   break;
				case TK_REG:
				   break;
				default: break;
					
						   
 			}
 	 	} 
		if(index == -1){printf("%d %d\n", start, end);	puts("no operator"); *success = false; return 0;}
		switch(tokens[index].type){
			case '+':
				return eval(start, index - 1, success) + eval(index + 1, end, success);
			case '-':
				return eval(start, index - 1, success) - eval(index + 1, end, success);
			case '*':
				return eval(start, index - 1, success) * eval(index + 1, end, success);
			case '/':;
				uint32_t fir; fir = eval(start, index - 1, success);
				uint32_t sec; sec = eval(index + 1, end, success);
				if(sec == 0)
					*success = false, puts("zero division error"), sec = 1;
				return fir / sec;
			case TK_EQ:
				return (eval(start, index - 1, success) == eval(index + 1, end, success));
			case TK_NEQ:
				return (eval(start, index - 1, success) != eval(index + 1, end, success));
			case TK_AND:
				return (eval(start, index - 1, success) && eval(index + 1, end, success));
			case TK_NEG:
				return -eval(index + 1, end, success);
			case TK_DR:
				return vaddr_read(eval(index + 1, end, success), 4);
			default: break;
 		}
	 } 
	return 0;
}

static bool ifmatched(int pos){
	int left = tokens[pos - 1].type;
	int right = tokens[pos + 1].type;
	return ((left == TK_NUM) || (left == TK_REG) || (left == TK_HEX) || (left == TK_R)) && ((right == TK_NUM) || (right == TK_HEX)  || (right == TK_REG) || (right == TK_L) || (right == TK_NEG));
}
