#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t cnt = 0;
  while (s[cnt] != '\0') cnt++;
  return cnt;
}

char *strcpy(char* dst,const char* src) {
  for (char *p = dst; ; p++, src++) {
    *p = *src;
	if (*src == '\0') break;
  }
  return dst;
}

char* strncpy(char* dst, const char* src, size_t n) {
  size_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) 
    dst[i] = src[i];
  for (; i < n; n++) dst[i] = '\0';
  return dst; 
}

char* strcat(char* dst, const char* src) {
  char *p; 
  for (p = dst; *p; p++);
  for ( ; ; p++, src++) {
    *p = *src;
	if (*src == '\0') break;
  }
  return dst;
}

int strcmp(const char* s1, const char* s2) {
/*s1_len > s2_len <-> s1[s2_len] > s2[s2_len]
 *s1_len < s2_len <-> s1[s1_len] < s2[s1_len] 
 */
  for (; *s1 && (*s1 - *s2) == 0; s1++, s2++);
  return (*s1 - *s2);
}

int strncmp(const char* s1, const char* s2, size_t n) {
/*the condition must be n > 1, not n > 0 
 *if n > s1_len, it depends on *s1 and (*s1 - *s2)
 *if n <= s1_len, it depends on n and *(s1 - *s2)
 */
  if (n == 0) return 0;
  for (; n > 1 && *s1 && (*s1 - *s2) == 0; s1++, s2++, n--);
  return (*s1 - *s2);
}

void* memset(void* v,int c,size_t n) {
  char *tmp = (char*)v;
  while (n--) tmp[n] = c; //n = n -1 right after judgement.
  return v;
}

void* memmove(void* dst,const void* src,size_t n) {
  return NULL;
}

void* memcpy(void* out, const void* in, size_t n) {
  for (size_t i = 0; i < n; i++) ((char *)out)[i] = ((char *)in)[i];
  return NULL;
}

int memcmp(const void* s1, const void* s2, size_t n) {
  if (n == 0) return 0;
  unsigned char *s1_ = (unsigned char *)s1;
  unsigned char *s2_ = (unsigned char *)s2;
  for ( ; n > 1 && (*s1_ - *s2_) == 0; s1_++, s2_++, n--);
  return (*s1_ - *s2_); 
}

#endif
