#include "proc.h"

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC];
static int nr_proc = 0;
PCB *current = NULL;

uintptr_t loader(_Protect *as, const char *filename);

void load_prog(const char *filename) {
  int i = nr_proc ++;
  _protect(&pcb[i].as);

  uintptr_t entry = loader(&pcb[i].as, filename);

  // TODO: remove the following three lines after you have implemented _umake()
  //_switch(&pcb[i].as);

  //current = &pcb[i];
  //((void (*)(void))entry)();
  _Area stack;
  stack.start = pcb[i].stack;
  stack.end = stack.start + sizeof(pcb[i].stack);

  pcb[i].tf = _umake(&pcb[i].as, stack, stack, (void *)entry, NULL, NULL);
}

int current_game = 0;
void game_change(){
  current_game= 2-current_game;
}

int count = 0;
_RegSet* schedule(_RegSet *prev) {
  count++;
  // save the context pointer
  if(current != NULL){
    current->tf = prev;
  }
  // always select pcb[0] as the new process
  //current = &pcb[0];

  // Priority
  current = &pcb[current_game];
  if(count==1000){
    current = &pcb[1];
    count=0;
  }
  
  // TODO: switch to the new address space,
  // then return the new context
  _switch(&current->as);
  return current->tf;

  //return NULL;
}
