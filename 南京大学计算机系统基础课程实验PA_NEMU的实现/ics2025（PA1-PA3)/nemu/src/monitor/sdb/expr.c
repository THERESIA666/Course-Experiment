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
#include <limits.h> 
#include <isa.h>
#include <inttypes.h>
#include <memory/paddr.h>
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NOEQ, TK_NUM, TK_NEG, TK_AND, TK_REG, TK_HEX, TK_DEREF

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},       // spaces
  {"\\+", '+'},            // plus
  {"==", TK_EQ},           // equal
  {"!=", TK_NOEQ},         // unequal
  {"&&",TK_AND},          // and 
  {"\\-", '-'},            // minus
  {"/", '/'},              // division
  {"\\*", '*'},            // multiply
  {"\\b[0-9]+\\b", TK_NUM},// number
  {"0[xX][[:xdigit:]]+", TK_HEX}, // hex
  {"\\$(pc|\\$0|ra|[sgt]p|t[0-6]|a[0-7]|s([0-9]|1[0-1]))", TK_REG}, //register
  {"\\(", '('},            // leftbrace
  {"\\)", ')'},            // rightbrace            
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

#define MAX_TOKENS 1024
static Token tokens[MAX_TOKENS] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;



static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        
        Token token;
        switch (rules[i].token_type) {
          case TK_NOTYPE:
              continue;
          case '*':
              if (nr_token == 0 || 
                  (tokens[nr_token-1].type != ')' && 
                   tokens[nr_token-1].type != TK_NUM &&
                   tokens[nr_token-1].type != TK_HEX &&
                   tokens[nr_token-1].type != TK_REG)) {
                  token.type = TK_DEREF; 
              } else {
                  token.type = '*';
              }
              snprintf(token.str, sizeof(token.str), "%.*s", substr_len, substr_start);
              break;
          case '-':
              if (nr_token == 0 || 
                  tokens[nr_token-1].type == '(' ||
                  tokens[nr_token-1].type == TK_DEREF ||
                  tokens[nr_token-1].type == '+' ||
                  tokens[nr_token-1].type == '-' ||
                  tokens[nr_token-1].type == '*' ||
                  tokens[nr_token-1].type == '/') {
                  token.type = TK_NEG; 
              } else {
                  token.type = '-'; 
              }
              snprintf(token.str, sizeof(token.str), "%.*s", substr_len, substr_start);
              break;
          case TK_AND:
              snprintf(token.str, sizeof(token.str), "%.*s", substr_len, substr_start);
              token.type = TK_AND;
              break;
          default: 
              snprintf(token.str, sizeof(token.str), "%.*s", substr_len, substr_start); 
              token.type = rules[i].token_type;
              break;
        }

        if (token.type != TK_NOTYPE) {
            if (nr_token < sizeof(tokens)/sizeof(Token)) {
                tokens[nr_token] = token;
                nr_token++;
            } else {
                return false;
            }
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static bool check_parentheses(int p, int q) {
    if (tokens[p].type != '(' || tokens[q].type != ')') {
        return false;
    }
    
    int balance = 0;
    for (int i = p; i <= q; i++) {
        if (tokens[i].type == '(') balance++;
        else if (tokens[i].type == ')') balance--;
        
        if (balance == 0 && i < q) return false;
    }
    return balance == 0;
}

static int find_main_op(int p, int q) {
    int op_pos = -1;
    int min_pri = INT_MAX;
    int bal = 0;
    
    const int priority[512] = {
        [TK_AND] = 1,
        [TK_EQ] = 2, [TK_NOEQ] = 2,
        ['+'] = 3, ['-'] = 3,
        ['*'] = 4, ['/'] = 4,
        [TK_NEG] = 5,
        [TK_DEREF] = 5,
        [TK_NUM] = -1, [TK_HEX] = -1, [TK_REG] = -1
    };
    
    
    for (int i = q; i >= p; i--) {
          
    
        if (tokens[i].type == '(') bal++;
        else if (tokens[i].type == ')') bal--;
        else if (bal == 0) {
            int curr_pri = priority[tokens[i].type];
           
            if (curr_pri > 0 && curr_pri < min_pri) {
                min_pri = curr_pri;
                op_pos = i;
               
            }
        }
    }
    return op_pos;
}
static uint32_t token_to_num(int idx, bool *success){
  
    *success=true;
    if (tokens[idx].type == TK_NUM) {
        return (uint32_t)strtoul(tokens[idx].str, NULL, 10);
    } 
    else if (tokens[idx].type == TK_HEX) {
        return (uint32_t)strtoul(tokens[idx].str, NULL, 16);
    }
    else if (tokens[idx].type == TK_REG) {
        if (strcmp(tokens[idx].str, "$pc") == 0) {
            return cpu.pc;
        }
        return isa_reg_str2val(tokens[idx].str, success);
    }
    *success=false;
    return 0;
}

static uint32_t eval(int p, int q, bool *success) {
    
        
    if (p > q) {
        *success = false;
        return 0;
    }
    else if (p == q) {
        if (tokens[p].type == TK_NEG) {
            uint32_t val = eval(p + 1, q, success);
            return *success ? (uint32_t)(-(int32_t)val) : 0;
        }
        else if (tokens[p].type == TK_DEREF) {
            uint32_t addr = eval(p + 1, q, success);
            if (!*success) return 0;
            return paddr_read(addr, 4); 
        }
        return token_to_num(p, success);
    }
    else if (check_parentheses(p, q)) {
        return eval(p + 1, q - 1, success);
    }
    else {
        int op = find_main_op(p, q);
        if (op == -1) {
            printf("Error: No valid operator found in [%d,%d]\n", p, q);
            *success = false;
            return 0;
        }

       
        if (tokens[op].type == TK_NEG || tokens[op].type == TK_DEREF) {
            uint32_t val = eval(op + 1, q, success);
            if (!*success) return 0;
            
            if (tokens[op].type == TK_NEG) {
                return (uint32_t)(-(int32_t)val);
            } else { 
                return paddr_read(val, 4);
            }
        }

       
        bool success1 = true;
        bool success2 = true;
        
        uint32_t val1 = eval(p, op - 1, &success1);
        uint32_t val2 = eval(op + 1, q, &success2);
        
        if (!success1 || !success2) {
            *success = false;
            return 0;
        }
        
                
        switch (tokens[op].type) {
            case '+': return val1 + val2;
            case '-': return val1 - val2;
            case '*': return val1 * val2;
            case '/': 
                if (val2 == 0) {
                    *success = false;
                    return 0;
                }
                return val1 / val2;
            case TK_EQ:  return (val1 == val2) ? 1 : 0;
            case TK_NOEQ: return (val1 != val2) ? 1 : 0;
            case TK_AND:  return (val1 && val2) ? 1 : 0;
            default:
                *success = false;
                return 0;
        }
    }
}



word_t expr(char *e, bool *success) {
    if (!make_token(e)) {
        *success = false;
        return 0;
    }
    if (nr_token == 0) {
        *success = false;
        return 0;
    }
    return (uint32_t)eval(0, nr_token - 1, success);
}
