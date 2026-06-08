#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char *number_to_string(char *buf, int num) {
  char *p = buf;
  char tmp[20]; 
  int i = 0;
  

  if (num < 0) {
    *p++ = '-';
    num = -num;
  }
  

  if (num == 0) {
    tmp[i++] = '0';
  } else {
   
    while (num > 0) {
      tmp[i++] = '0' + (num % 10);
      num /= 10;
    }
  }
  

  while (i > 0) {
    *p++ = tmp[--i];
  }
  
  *p = '\0';
  return p;
}




static char sprint_buf[1024];
int printf(const char *fmt, ...)
{
  va_list args; 
  int n;
  va_start(args, fmt);
  n = vsprintf(sprint_buf, fmt, args);
  va_end(args);
  putstr(sprint_buf);
  return n;
}


int vsprintf(char *out, const char *fmt, va_list ap) {
  char *p = out;
  const char *f = fmt;
  
  while (*f) {
    if (*f != '%') {
      
      *p++ = *f++;
      continue;
    }
    
   
    f++; 
    
    switch (*f) {
      case 's': {
        
        char *str = va_arg(ap, char*);
        while (*str) {
          *p++ = *str++;
        }
        f++;
        break;
      }
      
      case 'd': {
       
        int num = va_arg(ap, int);
        p = number_to_string(p, num);
        f++;
        break;
      }
      
      case '%': {
      
        *p++ = '%';
        f++;
        break;
      }
      
      default: {
       
        *p++ = '%';
        *p++ = *f++;
        break;
      }
    }
  }
  
  *p = '\0'; 
  return p - out; 
}


int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int result = vsprintf(out, fmt, ap);
  va_end(ap);
  return result;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(out, n, fmt, args);
  va_end(args);
  return len;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  if (n == 0) return 0;  // 如果缓冲区大小为0，直接返回
  
  char *p = out;
  const char *f = fmt;
  size_t remaining = n - 1;  // 保留一个字符给结尾的'\0'
  
  while (*f && remaining > 0) {
    if (*f != '%') {
      // 普通字符
      *p++ = *f++;
      remaining--;
      continue;
    }
    
    // 遇到格式符 %
    f++;  // 跳过 '%'
    
    switch (*f) {
      case 's': {
        // 字符串
        char *str = va_arg(ap, char*);
        if (str == NULL) str = "(null)";
        
        while (*str && remaining > 0) {
          *p++ = *str++;
          remaining--;
        }
        f++;
        break;
      }
      
      case 'd': {
        // 整数
        int num = va_arg(ap, int);
        char num_buf[20];  // 临时缓冲区存放数字字符串
        
        
        // 处理负数
        if (num < 0) {
          if (remaining > 0) {
            *p++ = '-';
            remaining--;
          }
          num = -num;
        }
        
        // 转换数字为字符串（逆序）
        int digits = 0;
        if (num == 0) {
          num_buf[digits++] = '0';
        } else {
          while (num > 0 && digits < 19) {
            num_buf[digits++] = '0' + (num % 10);
            num /= 10;
          }
        }
        
        // 将数字字符串正序复制到输出缓冲区
        while (digits > 0 && remaining > 0) {
          *p++ = num_buf[--digits];
          remaining--;
        }
        f++;
        break;
      }
      
      case 'c': {
        // 字符
        if (remaining > 0) {
          char ch = (char)va_arg(ap, int);
          *p++ = ch;
          remaining--;
        }
        f++;
        break;
      }
      
      case 'x': 
      case 'X': {
        // 十六进制（简单实现）
        unsigned int num = va_arg(ap, unsigned int);
        char hex_buf[16];
        int hex_digits = 0;
        
        if (num == 0) {
          hex_buf[hex_digits++] = '0';
        } else {
          while (num > 0 && hex_digits < 15) {
            int digit = num & 0xF;
            hex_buf[hex_digits++] = (digit < 10) ? '0' + digit : 
                                   (*f == 'x' ? 'a' + digit - 10 : 'A' + digit - 10);
            num >>= 4;
          }
        }
        
        // 复制到输出缓冲区
        while (hex_digits > 0 && remaining > 0) {
          *p++ = hex_buf[--hex_digits];
          remaining--;
        }
        f++;
        break;
      }
      
      case '%': {
        // 转义的 %
        if (remaining > 0) {
          *p++ = '%';
          remaining--;
        }
        f++;
        break;
      }
      
      default: {
        // 未知格式符，原样输出
        if (remaining > 1) {
          *p++ = '%';
          *p++ = *f;
          remaining -= 2;
        } else if (remaining == 1) {
          *p++ = '%';
          remaining = 0;
        }
        f++;
        break;
      }
    }
  }
  
  // 确保字符串以 null 结尾
  *p = '\0';
  
  // 返回应该写入的字符数（不包括结尾的null）
  return (p - out);
}

#endif
