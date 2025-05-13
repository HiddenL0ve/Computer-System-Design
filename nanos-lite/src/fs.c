#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);

extern void fb_write(const void *buf, off_t offset, size_t len);
extern size_t events_read(void *, size_t);
extern void dispinfo_read(void *, off_t, size_t);

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = _screen.height * _screen.width * 4;
}

size_t fs_filesz(int fd) {
  return file_table[fd].size;
}

int fs_open(const char*filename, int flags, int mode) {
  for(int i = 0; i < NR_FILES; i++){
    if(strcmp(filename, file_table[i].name) == 0) {
      return i;
    }
  }
  panic("this file not exist:%s",filename);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len){
  //assert(fd >= 0 && fd < NR_FILES);
  ssize_t fs_size = fs_filesz(fd);
	if (file_table[fd].open_offset + len > fs_size)
		len = fs_size - file_table[fd].open_offset;
  if(fd < 3 || fd == FD_FB) {
    Log("arg invalid:fd<3");
    return 0;
  }
  else if(fd == FD_EVENTS) {
    return events_read(buf, len);
  }

  else if(fd == FD_DISPINFO){
    dispinfo_read(buf, file_table[fd].open_offset, len);
    //file_table[fd].open_offset += len;
  }
  else {
    ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
//Log("enter fread");
  file_table[fd].open_offset += len;
  return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
  ssize_t fs_size = fs_filesz(fd);

  if(fd == FD_FB){
    fb_write(buf, file_table[fd].open_offset, len);
  }
  else {
    if(file_table[fd].open_offset + len > fs_size)
      len = fs_size - file_table[fd].open_offset;
    ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
//Log("enter fwrite");
  file_table[fd].open_offset += len;
  return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
//Log("enter fseek");
  switch(whence) {
    case SEEK_SET:
      if (offset >= 0 && offset <= file_table[fd].size){
        file_table[fd].open_offset = offset;
        return file_table[fd].open_offset;
      }

    case SEEK_CUR:
      if ((offset + file_table[fd].open_offset >= 0) && (offset + file_table[fd].open_offset <= file_table[fd].size)){
        file_table[fd].open_offset = offset;
        return file_table[fd].open_offset;
      }

    case SEEK_END:
      file_table[fd].open_offset = file_table[fd].size + offset;
      return file_table[fd].open_offset;
    default:
      panic("Unhandled whence ID = %d", whence);
      return -1;
    }
}

int fs_close(int fd) {
  assert(fd >= 0 && fd < NR_FILES);
  file_table[fd].open_offset = 0;
  return 0;
}
