#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include <common.h>

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  /* TODO: Add more members if necessary */
  word_t val;
  char what[32];
} WP;

void free_wp(int NO);
WP* new_wp();
WP* get_next_wp();
#endif
