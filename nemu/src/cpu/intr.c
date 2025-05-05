#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  //TODO();
  memcpy(&t1,&cpu.eflags,sizeof(cpu.eflags));
  rtl_li(&t0,t1);
  rtl_push(&t0);//eflags
  cpu.eflags.IF=0;
  rtl_push(&cpu.cs);//cs
  rtl_li(&t0,ret_addr);
  rtl_push(&t0);//eip

  if((t1 & 0x00008000) == 0)
      assert(0);
      
  decoding.jmp_eip = (t0&0xffff)|(t1&0xffff0000);
  decoding.is_jmp = 1;
}

void dev_raise_intr() {
  cpu.INTR = true;
}
