#include "common.h"
#include "syscall.h"

static inline _RegSet* sys_none(_RegSet *r){
  SYSCALL_ARG1(r) = 1;
  return NULL;
}

static inline _RegSet* sys_exit(_RegSet *r){
  _halt(SYSCALL_ARG2(r)); 
  return NULL;
}

static inline _RegSet* sys_write(_RegSet *r){
  int fd = SYSCALL_ARG2(r);
  void *buf = (void*)SYSCALL_ARG3(r);
  size_t len = SYSCALL_ARG4(r);
  Log("buffer:%s", (char*)buf);
  if(fd == 1 || fd == 2) {
    for(int i = 0; i < len; i++) {
      _putc(((char*)buf)[i]);
    }
    SYSCALL_ARG1(r) = len;
  }
  else {
    panic("Unhandled fd=%d in sys_write()", fd);
  }

  return NULL;
}

static inline _RegSet* sys_brk(_RegSet *r) {
  SYSCALL_ARG1(r) = 0;
  return NULL;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none: return sys_none(r);
    case SYS_exit:return sys_exit(r);
    case SYS_write:return sys_write(r);
    case SYS_brk:return sys_brk(r);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
