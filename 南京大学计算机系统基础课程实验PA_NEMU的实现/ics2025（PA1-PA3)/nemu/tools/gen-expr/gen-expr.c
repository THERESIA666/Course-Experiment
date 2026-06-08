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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int buf_pos = 0;

static uint32_t choose(uint32_t n) {
    return rand() % n;
}

static void gen_rand_op() {
    switch (choose(4)) {
        case 0: sprintf(buf + buf_pos, " + "); buf_pos += 3; break;
        case 1: sprintf(buf + buf_pos, " - "); buf_pos += 3; break;
        case 2: sprintf(buf + buf_pos, " * "); buf_pos += 3; break;
        case 3: sprintf(buf + buf_pos, " / "); buf_pos += 3; break;
    }
}

static void gen_num() {
    uint32_t num = rand() % 500 + 1; 
    buf_pos += sprintf(buf + buf_pos, "%u", num);
}


static bool has_division_by_zero(const char *expr) {
    const char *p = expr;
    while (*p) {
        if (*p == '/') {
            const char *after_slash = p + 1;
            
     
            while (*after_slash == ' ') after_slash++;
            
     
            if (*after_slash == '0') {
           
                char next_char = after_slash[1];
                if (next_char == '\0' || next_char == ' ' || 
                    next_char == ')' || next_char == '+' || 
                    next_char == '-' || next_char == '*' || 
                    next_char == '/') {
                    return true;
                }
            }
            
          
            if (*after_slash == '(') {
                const char *close_paren = strchr(after_slash + 1, ')');
                if (close_paren) {
              
                    char inner[32] = {0};
                    strncpy(inner, after_slash + 1, close_paren - after_slash - 1);
                    if (strcmp(inner, "0") == 0) {
                        return true;
                    }
                }
            }
        }
        p++;
    }
    return false;
}

static void gen_rand_expr(int depth) {
   
    if (buf_pos > 9998 || depth > 12) return;
    
    switch (choose(10)) {
        case 0: case 1: case 2:  case 3:  
            if (buf_pos == 0 || buf[buf_pos - 1] != ')') {
                gen_num(); 
            } else {
                gen_rand_expr(depth + 1);
            }
            break;
        case 4: case 5: 
            if (buf_pos > 10000) return;
            sprintf(buf + buf_pos, "("); buf_pos++;
            gen_rand_expr(depth + 1);
            if (buf_pos > 10001) return;
            sprintf(buf + buf_pos, ")"); buf_pos++;
            break;
        case 6: case 7: case 8: 
            if (buf_pos > 10001) return;
            if (choose(2) == 0) {

                sprintf(buf + buf_pos, "-"); buf_pos++;
            } else {
              
                sprintf(buf + buf_pos, "*"); buf_pos++;
            }
            gen_rand_expr(depth + 1);
            break;
        case 9:  
            gen_rand_expr(depth + 1);
            gen_rand_op();
            gen_rand_expr(depth + 1);
            break;
    }
}

int main(int argc, char *argv[]) {
    int seed = time(0);
    srand(seed);
    int loop = 1;
    if (argc > 1) {
        sscanf(argv[1], "%d", &loop);
    }
    system("rm -f /tmp/.code.c /tmp/.expr");
    
    int i;
    for (i = 0; i < loop; i ++) {
        buf_pos = 0;
        
        memset(buf, 0, 10005);
        
        gen_rand_expr(0); 
        
        buf[buf_pos] = '\0';
        
   
        if (strlen(buf) < 2) {
            i--;
            continue;
        }
        
      
        if (has_division_by_zero(buf)) {
            i--;
            continue;
        }
        
        sprintf(code_buf, code_format, buf);

        FILE *fp = fopen("/tmp/.code.c", "w");
        assert(fp != NULL);
        fputs(code_buf, fp);
        fclose(fp);

        int ret = system("gcc /tmp/.code.c -o /tmp/.expr -w 2>/dev/null");
        if (ret != 0) {
            unlink("/tmp/.code.c"); 
            continue;
        }

        fp = popen("/tmp/.expr", "r");
        if(fp == NULL){
            unlink("/tmp/.code.c");
            unlink("/tmp/.expr");
            i--;
            continue;
        }

        unsigned result;
        ret = fscanf(fp, "%u", &result);
        pclose(fp);
        
        unlink("/tmp/.code.c");
        unlink("/tmp/.expr");
        
        if(ret != 1){
            i--;
            continue;
        }
        printf("%u %s\n", result, buf);
        

        if (i % 100 == 0) {
            system("rm -f /tmp/.code.c /tmp/.expr");
        }
    }
    system("rm -f /tmp/.code.c /tmp/.expr");
    return 0;
}
