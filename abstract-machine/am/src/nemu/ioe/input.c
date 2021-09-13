#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keydown = 0;
  kbd->keycode = AM_KEY_NONE;
  if (KBD_ADDR == 0xa1000060) putch('y');
  uint32_t am_scancode = inl(KBD_ADDR);
  kbd->keydown = (am_scancode & KEYDOWN_MASK) != 0; //the highest bit 'equivalent' to 1 means keydown
  kbd->keycode = am_scancode ^ KEYDOWN_MASK; 
}
