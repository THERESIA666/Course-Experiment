/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
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
  
  char *expr;
  word_t last_value;
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}
WP* new_wp() {
  if (free_ == NULL) {
    printf("No free watchpoints available\n");
    return NULL;
  }
  
  WP *wp = free_;
  free_ = free_->next;
   
  wp->next = head;
  head = wp;
  
  return wp;
}
void free_wp(WP *wp) {
  if (wp == NULL) return;
  
  if (head == wp) {
    head = head->next;
  } else {
    WP *prev = head;
    while (prev != NULL && prev->next != wp) {
      prev = prev->next;
    }
    if (prev != NULL) {
      prev->next = wp->next;
    }
  }
  
  if (wp->expr != NULL) {
    free(wp->expr);
    wp->expr = NULL;
  }
  
  wp->next = free_;
  free_ = wp;
}
void wp_creat(char *args, int32_t wpt_value) {
  if (args == NULL || strlen(args) == 0) {
    printf("No expression provided\n");
    return;
  }
  
  WP *wp = new_wp();
  if (wp == NULL) {
    printf("Failed to create new watchpoint\n");
    return;
  }
  
  wp->expr = strdup(args);  
  wp->last_value = wpt_value;
  
  printf("Watchpoint %d: %s (Initial Value = %d)\n", 
         wp->NO, wp->expr, wpt_value);
}

void wp_remove(int del_g) {
  WP *wp = head;
  while (wp != NULL) {
    if (wp->NO == del_g) {
      free_wp(wp);
      printf("Deleted Watchpoint %d\n", del_g);
      return;
    }
    wp = wp->next;
  }
  printf("Watchpoint %d Not Found\n", del_g);
}
void wp_display_all(void) {
  WP *wp = head;
  if (wp == NULL) {
    printf("No Watchpoints\n");
    return;
  }
  
  printf("Num     What\n");
  while (wp != NULL) {
    if (wp->expr != NULL) {
      printf("%-8d%s\n", wp->NO, wp->expr);
    }
    wp = wp->next;
  }
}
void check_watchpoints() {
  WP *wp = head;
  while (wp != NULL) {
    if (wp->expr != NULL) {
      bool success = true;
      word_t current_value = expr(wp->expr, &success);
      
      if (success && current_value != wp->last_value) {
        printf("Watchpoint %d (%s) triggered: \n", wp->NO, wp->expr);
        printf("Last value = %d, Current value = %d\n", 
               wp->last_value, current_value);
        wp->last_value = current_value;
        nemu_state.state = NEMU_STOP;
      }
    }
    wp = wp->next;
  }
}
/* TODO: Implement the functionality of watchpoint */

