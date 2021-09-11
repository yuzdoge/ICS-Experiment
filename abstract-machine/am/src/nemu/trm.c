#include <am.h>
#include <nemu.h>

extern char _heap_start;
int main(const char *args);

Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
  outb(SERIAL_PORT, ch);
}

void halt(int code) {
  nemu_trap(code);

  // should not reach here
  while (1);
}

void _trm_init() {
//if ((uintptr_t)&_pmem_start == 0x80000000) { putch('y'); putch('\n'); } //the output is y with the ARCH=riscv32-nemu
  int ret = main(mainargs);
  halt(ret);
}
