#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

//heap is decalared in <am.h>
static size_t offset;  
void *malloc(size_t size) {
  char *hbrk, *p; 
  p = (char*)heap.start + offset; 
  size = (size + (8 - 1)) & ~(8 - 1); //8 bytes align
  offset += size;
  hbrk = (char*)heap.start + offset; 
  assert((uintptr_t)heap.start <= (uintptr_t)hbrk && (uintptr_t)hbrk <= (uintptr_t)heap.end);
  //omit the initialization of the memory allocated. 
  return p;
}

void free(void *ptr) {
}

#endif
