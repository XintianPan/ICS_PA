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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
	word_t oldval;
	char * express;
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

WP* new_wp();

word_t expr(char* e, bool* success);

void delete_wp(int number);

void add_wp(char * express);

void free_wp(WP *wp,WP* pre);

bool ifchange();

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp(){
	if(free_ == NULL){
		puts("pool is used up");
		assert(0);
		return NULL;
	}else{
		WP* new_node = free_;
		free_ = free_->next;

		return new_node;
	}
	return NULL;
}

void free_wp(WP* wp, WP* pre){
	if(pre == NULL){
		head = wp->next;
		wp->next = free_;
		free_ = wp;
	}else{
		pre->next = wp->next;
		wp->next = free_;
		free_ = wp;
	}
}

void add_wp(char *express){
	bool succ = false;
	word_t ret = expr(express, &succ);
	if(!succ){
		puts("bad expression!");
		return;
	}
	WP* new_node = new_wp();
	if(new_node == NULL){
		puts("pool is used up");
		return;
	}else{
		new_node->express = express;
		new_node->oldval = ret;
		new_node->next = head;
		head = new_node;
		return;
	}
}

void delete_wp(int num){
	WP* start = head;
	WP* pre = NULL;
	while(start != NULL && start->NO != num){
		pre = start;
		start = start->next;	
	}
	if(start == NULL)
		puts("Invalid number");
	else
		free_wp(start, pre);
}

bool ifchange(){
	WP* tmp = head;
	bool succ = false;
	word_t ret;
	while(tmp != NULL){
		ret = expr(tmp->express, &succ);
		if(ret != tmp->oldval){
			printf("Change at watchpoint %d\n Expression %s\nOld value: %u\nNew value: %u\n", tmp->NO, tmp->express, tmp->oldval, ret);
			return true;
		}		
	}
	return false;
}
