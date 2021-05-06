#include "watchpoint.h"
#include "expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

static inline void element_exchange(WP **dest_list_p, WP **src_list_p){
  WP* target_node_p;
  target_node_p = *src_list_p; 
  *src_list_p = target_node_p->next;

  target_node_p->next = *dest_list_p;
  *dest_list_p = target_node_p;
} 

WP* new_wp(){
  Assert(free_, "resources of the watchpoint-pool run out\n");
  element_exchange(&head, &free_);
  return head;
}

void free_wp(int NO){
  WP *current = head;
  WP *front = current;
  for (; current; front = current, current = current->next)
    if (current->NO == NO){
	  element_exchange(&free_, &front);
	  return; 
	}
  
  printf("no watchpoint number %d\n", NO);
}


WP* get_next_wp(WP *wp){ 
  if (wp == NULL)
	return head;
  return wp->next;
}
