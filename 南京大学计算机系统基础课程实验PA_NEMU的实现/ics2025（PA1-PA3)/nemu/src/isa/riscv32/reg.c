/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display(void) {
  extern MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state) cpu;

  int reg_count = MUXDEF(CONFIG_RVE, 16, 32);
  printf("General Purpose Registers (x0-x%d):\n", reg_count - 1);
  for (int i = 0; i < reg_count; i++) {
    printf(" %-4s = 0x%08" MUXDEF(CONFIG_RV64, PRIx64, PRIx32) "\n", regs[i], cpu.gpr[i]);
  }

  printf("PC = 0x%08" MUXDEF(CONFIG_RV64, PRIx64, PRIx32) "\n", cpu.pc);
}



word_t isa_reg_str2val(const char *s, bool *success) {
    *success = true;
    extern MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state) cpu;

    if (s[0] == '$') {
        s++; 
    }

    if (strcmp(s, "0") == 0 || strcmp(s, "zero") == 0) {
        return 0;
    }

    int reg_count = MUXDEF(CONFIG_RVE, 16, 32);
    for (int i = 0; i < reg_count; i++) {
        if (strcmp(s, regs[i]) == 0) {
            return cpu.gpr[i];
        }
    }


    if (strcmp(s, "pc") == 0) {
        return cpu.pc;
    }

    *success = false;
    return 0;
}
