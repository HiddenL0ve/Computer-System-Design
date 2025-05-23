#include "nemu.h"
#include "device/mmio.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

#define PTE_ADDR(pte)  ((uint32_t)(pte)&~0xfff)
#define PDX(va) (((uint32_t)(va)>>22)&0x3ff)
#define PTX(va) (((uint32_t)(va)>>12)&0x3ff)
#define OFF(va) ((uint32_t)(va)&0xfff)

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */ 

uint32_t paddr_read(paddr_t addr, int len) { 
  int r = is_mmio(addr);
  if(r == -1) {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
  else {
    return mmio_read(addr, len, r);
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) { 
  int r = is_mmio(addr);
  if(r == -1){
    memcpy(guest_to_host(addr), &data, len);
  }
  else { 
    mmio_write(addr, len, data, r);
  }
}

paddr_t page_translate(vaddr_t addr, bool iswrite) {
 CR0 cr0 = (CR0)cpu.CR0;
 if(cr0.paging && cr0.protect_enable) {
   CR3 crs = (CR3)cpu.CR3;
   // set PDE
   PDE *pgdirs = (PDE*)PTE_ADDR(crs.val);
   PDE pde = (PDE)paddr_read((uint32_t)(pgdirs + PDX(addr)), 4);
   Assert(pde.present, "addr=0x%x", addr);
   pde.accessed=1;
  
   // set PTE
   PTE *ptable = (PTE*)PTE_ADDR(pde.val);
   PTE pte = (PTE)paddr_read((uint32_t)(ptable + PTX(addr)), 4);
   Assert(pte.present, "addr=0x%x", addr);
   pte.accessed=1;

   if(iswrite) {
    pte.dirty=1;
   }
   paddr_t paddr = PTE_ADDR(pte.val) | OFF(addr);
   return paddr;
 }
 return addr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  if(PTE_ADDR(addr)!=PTE_ADDR(addr+len-1)){
    //assert(0);
    uint32_t data = 0;
    for(int i=0;i<len;i++){ 
    	paddr_t paddr = page_translate(addr + i, false);
    	data += (paddr_read(paddr, 1))<<8*i;
    }
    return data;
  }
  else{
    paddr_t paddr = page_translate(addr,false);
    return paddr_read(paddr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  if(PTE_ADDR(addr)!=PTE_ADDR(addr+len-1)){
    //assert(0);
    for(int i=0;i<len;i++){ 
    	paddr_t paddr = page_translate(addr + i,true);
    	paddr_write(paddr,1,data>>8*i);
    }
    return;
  }
  else{
    paddr_t paddr = page_translate(addr,true);
    return paddr_write(paddr, len, data);
  }
}

