#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint32_t am_scancode __attribute((unused))= inl(KBD_ADDR);
	int sig = am_scancode & KEYDOWN_MASK;
    kbd->keydown = sig != 0; //the highest bit 'equivalent' to 1 means keydown
    kbd->keycode = am_scancode ^ sig; 
   //kbd->keycode = AM_KEY_NONE;
   //kbd->keydown = 0;
}
