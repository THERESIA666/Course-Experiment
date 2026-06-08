#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    
    uintptr_t mcause = c->mcause;
    
    if ((mcause >= 0) && (mcause < 20)) {
      ev.event = EVENT_SYSCALL;
      goto goto_finish;
      
    } else { 
      switch (mcause) {
        case -1: 
          ev.event = EVENT_YIELD;
          goto goto_finish;
          
          break;
       
        default: 
          ev.event = EVENT_ERROR; 
          break;
      }
    }
    goto_finish:
    c->mepc +=4;
   
    
    c = user_handler(ev, c);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

#define MSTATUS_MMP  0x1800
#define MSTATUS_MPIE 0x80

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *kctx = (Context *)(kstack.end-sizeof(Context));
  
  
  memset(kctx, 0, sizeof(Context));
  
  
  kctx->mepc=(uintptr_t) entry;
  kctx->mstatus  = MSTATUS_MMP | MSTATUS_MPIE;
  kctx->gpr[2] = (uintptr_t)kstack.end; 
  
  kctx->gpr[10] = (uintptr_t)arg;
  return kctx;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
