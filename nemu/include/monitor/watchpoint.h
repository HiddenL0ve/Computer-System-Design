#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  int value;
  char expr[128];
} WP;

WP* new_wp();
void free_wp(int num);
void print_wp();
bool watch_wp();
#endif

