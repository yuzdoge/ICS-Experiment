#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
//  kbd->keycode = AM_KEY_NONE;
//  kbd->keydown = 0;
  uint32_t am_scancode = inl(KBD_ADDR);
  kbd->keycode = am_scancode ^ KEYDOWN_MASK; 
  kbd->keydown = ((am_scancode & KEYDOWN_MASK) != 0); //the highest bit 'equivalent' to 1 means keydown
}
