#include "common.h"
#include "syscall.h"
#include "fs.h"

extern void _halt(int);

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
  //Log("buffer:%s", (char*)buf);
  if(fd == 1 || fd == 2) {
    for(int i = 0; i < len; i++) {
      _putc(((char*)buf)[i]);
    }
    SYSCALL_ARG1(r) = len;
  }
  
  else {
Log("fd=%d",fd);
    SYSCALL_ARG1(r) = fs_write(fd,buf,len);
    //panic("Unhandled fd=%d in sys_write()", fd);
  }

  return NULL;
}

static inline _RegSet* sys_brk(_RegSet *r) {
  SYSCALL_ARG1(r) = 0;
  return NULL;
}

static inline _RegSet* sys_open(_RegSet *r) {
  const char* pathname = (const char*)SYSCALL_ARG2(r);
  int flags = (int)SYSCALL_ARG3(r);
  int mode = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_open(pathname,flags,mode);
  return NULL;
}
static inline _RegSet* sys_read(_RegSet *r) {
  int fd = (int)SYSCALL_ARG2(r);
  char *buf = (char *)SYSCALL_ARG3(r);
  int count = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_read(fd,buf,count);
  return NULL;
}

static inline _RegSet* sys_lseek(_RegSet *r) {
  int fd = (int)SYSCALL_ARG2(r);
  off_t offset = (off_t)SYSCALL_ARG3(r);
  int whence = (int)SYSCALL_ARG4(r);
  SYSCALL_ARG1(r) = fs_lseek(fd,offset,whence);
  return NULL;
}

static inline _RegSet* sys_close(_RegSet *r) {
  int fd = (int)SYSCALL_ARG2(r);
  SYSCALL_ARG1(r) = fs_close(fd);
  return NULL;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  Log("enter syscall");
  switch (a[0]) {
    case SYS_none: return sys_none(r);
    case SYS_exit: return sys_exit(r);
    case SYS_write:
      Log("enter write");
      return sys_write(r);
    case SYS_brk:return sys_brk(r);
    case SYS_open:return sys_open(r);
    case SYS_read:return sys_read(r);
    case SYS_close:return sys_close(r);
    case SYS_lseek:
      Log("enter lseek");
      return sys_lseek(r);

    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
