#include <proc.h>
#include <elf.h>
#include <sys/types.h> 
#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

int fs_open(const char *pathname, int flags, int mode);
int fs_close(int fd);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
off_t  fs_lseek(int fd, size_t offset, int whence);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
static uintptr_t loader(PCB *pcb, const char *filename) {
  printf("start loader!\n");
  int fd = fs_open(filename, 0, 0);
  Log("1. File opened, fd = %d", fd);
  if (fd < 0) {
    panic("should not reach here");
  }
  Elf_Ehdr elf;
 
  assert(fs_read(fd, &elf, sizeof(elf)) == sizeof(elf));
  Log("2. ELF header read successfully");
  // 检查魔数
  assert(*(uint32_t *)elf.e_ident == 0x464c457f);
  Log("3. ELF magic number verified");
  
  Elf_Phdr phdr;
  for (int i = 0; i < elf.e_phnum; i++) {
    Log("4. Processing segment %d/%d", i, elf.e_phnum);
    uint32_t base = elf.e_phoff + i * elf.e_phentsize;
 
    fs_lseek(fd, base, 0);
    assert(fs_read(fd, &phdr, elf.e_phentsize) == elf.e_phentsize);
    Log("5. Segment header read: type=%d, vaddr=0x%p", phdr.p_type, phdr.p_vaddr);
    // 需要装载的段
    if (phdr.p_type == PT_LOAD) {
      Log("6. Loading segment to 0x%p", phdr.p_vaddr);
      char * buf_malloc = (char *)malloc(phdr.p_filesz);

      Log("7. Buffer allocated: %p, size=%d", buf_malloc, phdr.p_filesz);
      
      fs_lseek(fd, phdr.p_offset, 0);
      
      
      
      assert(fs_read(fd, buf_malloc, phdr.p_filesz) == phdr.p_filesz);//报错行1
     
      Log("8. Segment data read into buffer");
      
      memcpy((void*)phdr.p_vaddr, buf_malloc, phdr.p_filesz);//报错行2
      Log("9. memcpy completed");
      memset((void*)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
      Log("10. memset completed");
      free(buf_malloc);
      Log("11. Buffer freed");
    }
  }
 
  assert(fs_close(fd) == 0);
  Log("12. File closed");
  Log("Loading segment: vaddr=0x%p, filesz=%d, memsz=%d", 
    phdr.p_vaddr, phdr.p_filesz, phdr.p_memsz);
  return elf.e_entry;
}


void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  // Log("pcb = %p", pcb);

  // 通过 pcb->stack 来提供栈区域
  Area kstack = {
    .start = pcb->stack,
    .end   = pcb->stack + STACK_SIZE,
  };

  // 调用 kcontext() 在这片栈区里创建上下文
  Context *context = kcontext(kstack, entry, arg);

  // 记录到 pcb->cp 里
  pcb->cp = context;
}


#define UNSPECIFIED_MEMORY 0
#define MEMORY_SPACE sizeof(void *)

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  protect(&pcb->as);

  Area kstack = {
    .start = pcb->stack,
    .end   = pcb->stack + STACK_SIZE,
  };

  uintptr_t entry = loader(pcb, filename);

  if (!entry) {
    Log("Loaded entry is NULL, doing nothing...");
    return;
  }

  int num_pages = STACK_SIZE / PGSIZE;  

  void *user_stack_va_base = (char *)pcb->as.area.end - STACK_SIZE;
  void *user_stack_base    = new_page(num_pages);

  for (int i = 0; i < num_pages; i++) {
    void *page_pa = (char *)user_stack_base    + i * PGSIZE;
    void *page_va = (char *)user_stack_va_base + i * PGSIZE;

    map(&pcb->as, page_va, page_pa, 0b111);
  }

  uintptr_t sp   = (uintptr_t)user_stack_base    + STACK_SIZE - 1;  // 物理地址栈顶
  uintptr_t vasp = (uintptr_t)user_stack_va_base + STACK_SIZE - 1;  // 虚拟地址栈顶

  sp   -= UNSPECIFIED_MEMORY;
  vasp -= UNSPECIFIED_MEMORY;

  Log("Current user stack: sp = %p, vasp = %p", sp, vasp);

  int argc, envc;

  if (!argv) { argc = 0; }
  if (!envp) { envc = 0; }

  for (argc = 0; argv && argv[argc] != NULL; argc++) ;
  for (envc = 0; envp && envp[envc] != NULL; envc++) ;

  char **tmp_argv = malloc(argc * sizeof(char *));
  char **tmp_envp = malloc(envc * sizeof(char *));

  for (int i = 0; i < argc; i++) {
    int len = strlen(argv[i]) + 1;
    strncpy((char *)sp, argv[i], len);
    tmp_argv[i] = (char *)sp;

    sp   -= len;
    vasp -= len;
  }

  for (int i = 0; i < envc; i++) {
    int len = strlen(envp[i]) + 1;
    strncpy((char *)sp, envp[i], len);
    tmp_envp[i] = (char *)sp;

    sp   -= len;
    vasp -= len;
  }

  typedef char ** space;

  sp -= UNSPECIFIED_MEMORY;
  sp -= MEMORY_SPACE;

  vasp -= UNSPECIFIED_MEMORY;
  vasp -= MEMORY_SPACE;

  *(space)sp = NULL;

  for (int i = envc - 1; i >= 0; i--) {
    sp   -= MEMORY_SPACE;
    vasp -= MEMORY_SPACE;
    *(space)sp = tmp_envp[i];
  }
  sp   -= MEMORY_SPACE;
  vasp -= MEMORY_SPACE;
  *(space)sp = NULL;

  for (int i = argc - 1; i >= 0; i--) {
    sp   -= MEMORY_SPACE;
    vasp -= MEMORY_SPACE;
    *(space)sp = tmp_argv[i];
  }
  vasp -= MEMORY_SPACE;
  sp   -= MEMORY_SPACE;
  *(int *)sp = argc;

  Context *context = ucontext(&pcb->as, kstack, (void *)entry);

  pcb->cp = context;
  pcb->cp->GPRx = sp;

  Log("mepc: %p", pcb->cp->mepc);
}
