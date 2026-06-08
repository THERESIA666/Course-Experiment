#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)


size_t strlen(const char *s) {
   if (s == NULL) {
    return 0;
  }
  size_t n = 0;
  while(s[n] != '\0') {
    ++n;
  }
  return n;
 
}


char *strcpy(char *dst, const char *src) {
  char *d = dst;
  const char *s = src;
  
  while ((*d++ = *s++) != '\0') {
  }
  return dst;
}




char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for(i = 0; src[i] != '\0'; i++){
    dst[i] = src[i];
  }
	  
  dst[i] = '\0';
  return dst;
 
}


char *strcat(char *dst, const char *src) {
  char *d = dst;
  
  
  while (*d != '\0') {
    d++;
  }
  

  while ((*d++ = *src++) != '\0') {
  }
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


int strncmp(const char *s1, const char *s2, size_t n) {
  if (n == 0) return 0;
  
  while (n-- && *s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  
  if (n == (size_t)-1) {  
    return 0;
  }
  return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void *memset(void *s, int c, size_t n) {
  unsigned char *p = (unsigned char *)s;
  unsigned char value = (unsigned char)c;
  
  for (size_t i = 0; i < n; i++) {
    p[i] = value;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *d = (unsigned char *)dst;
  const unsigned char *s = (const unsigned char *)src;
  
  if (d == s) {
    return dst; 
  }
  
  if (d < s || d >= s + n) {
 
    for (size_t i = 0; i < n; i++) {
      d[i] = s[i];
    }
  } else {

    for (size_t i = n; i > 0; i--) {
      d[i - 1] = s[i - 1];
    }
  }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *dst = (unsigned char *)out;
  const unsigned char *src = (const unsigned char *)in;
  
  for (size_t i = 0; i < n; i++) {
    dst[i] = src[i];
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  
  for (size_t i = 0; i < n; i++) {
    if (p1[i] != p2[i]) {
      return p1[i] - p2[i];
    }
  }
  return 0;
}

#endif
