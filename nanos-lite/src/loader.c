#include "common.h"
#include "fs.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void* new_page(void);

uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  //ramdisk_read(DEFAULT_ENTRY, 0, RAMDISK_SIZE);

  //return (uintptr_t)DEFAULT_ENTRY;
  
  int fd = fs_open(filename, 0, 0);
  Log("filename=%s,fd=%d",filename,fd);
  fs_read(fd, DEFAULT_ENTRY, fs_filesz(fd));
  int f_size = fs_filesz(fd);

  void* pa = DEFAULT_ENTRY;
  void* va = DEFAULT_ENTRY;
  while(f_size > 0){
    pa = new_page();
    _map(as, va, pa);
    fs_read(fd, pa, PGSIZE);
    va += PGSIZE;
    f_size -= PGSIZE;
  }
  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
