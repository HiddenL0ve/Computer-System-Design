#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

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
WP* new_wp(){
  if(!free_){
    printf("No available watchpoint\n");
    assert(0);
  }
  WP* newWP = free_;
  free_ = free_->next;
  newWP->next = head;
  head = newWP;
  return head;
}

void free_wp(int num){
  WP* wp = NULL;
  for(WP* i = head; i != NULL; i = i->next) {
    if(i->NO == num) {
      wp = &wp_pool[num];
    }
  }
  if(wp == NULL) {
    printf("invalid watchpoint to free\n");
    return;
  }
  
  if(head == wp){
    head = wp->next;
  }
  else {
    for(WP* i = head;i!=NULL;i = i->next){
      if(i->next == wp){
        i->next = wp->next;
      }
    }
  }
  wp->next = free_;
  free_ = wp;
}

bool check_wp(){
  bool ret = false;
  for(WP* i = head; i != NULL; i = i->next){
    bool success = true;
    uint32_t ans = expr(i->expr, &success);
    if(success){
      if(ans != i->value){
        printf("Watchpoint : %d\texpr: %s\t0x%08d->0x%08d\n", i->NO, i->expr, i->value, ans);
        i->value = ans;
        ret = true;
      }
    }
    else{
      assert(0);
    }
  }
  return ret;
}

void print_wp() {
  if(head == NULL) {
    printf("no watchpoint now\n"); 
    return; 
  } 
  for(WP* i = head;i != NULL; i = i->next) {
    printf("Watchpoint %d : %s\n", i->NO, i->expr);
  }
  return;
}

