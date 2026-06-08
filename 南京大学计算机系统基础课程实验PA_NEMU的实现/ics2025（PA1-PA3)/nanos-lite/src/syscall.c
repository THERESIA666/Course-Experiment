#include <common.h>
#include "syscall.h"
#include <proc.h>
#include <sys/types.h> 
int fs_open(const char *pathname, int flags, int mode);
int fs_close(int fd);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
off_t  fs_lseek(int fd, size_t offset, int whence);
void naive_uload(PCB *, const char *);

static char cur_bin[64] = ENTRY_BIN;

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
    case 0: 
       
      Log("ENTRY: %s, cur: %s", ENTRY_BIN, cur_bin);
      if (strcmp(ENTRY_BIN, cur_bin) == 0) {
        halt(0);
      } else {
        naive_uload(NULL, ENTRY_BIN);
      }
      c->GPRx = 0;  
      break;
    case 1:printf("yield!\n");yield();c->GPRx = 0;break;
    case 2:c->GPRx = fs_open((char *)a[1], a[2], a[3]); break;
    case 3:c->GPRx = fs_read(a[1], (void *)a[2], a[3]); break;
    case 4:c->GPRx = fs_write(a[1], (void *)a[2], a[3]); break;
    case 7:c->GPRx = fs_close(a[1]); break;
    case 8:c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
    case 9:
      
      // brk syscall
      // just ignore it
      c->GPRx = 0;
      break;
    case 13:
      strncpy(cur_bin, (const char *)a[1], strlen((const char *)a[1]) + 1);
      naive_uload(NULL, (const char *)a[1]);
      c->GPRx = 0;     
        break;
    case 19:
      uint32_t clock = io_read(AM_TIMER_UPTIME).us;
      struct timeval *tv = (struct timeval *)a[1];
      if (tv == NULL) {
        c->GPRx = -1;
        break;
      }
      tv->tv_sec = clock / 1000000;
      tv->tv_usec = clock % 1000000;
      c->GPRx = 0;
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
