#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static uint32_t rect;
void __am_gpu_init() {
  rect = inl(VGACTL_ADDR);
  int i;
  int w = rect >> 16; 
  int h = rect & 0x0000ffff; 
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i++) fb[i] = i;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = rect >> 16, .height = rect & 0x0000ffff,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
  if (ctl->pixels != NULL) {
    int screen_w = rect >> 16; 
    //int screen_h = rect & 0x0000ffff; 
	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
	int i = 0;
	for (int y = ctl->y; y < ctl->y + ctl->h; y++)
	  for (int x = ctl->x; x < ctl->x + ctl->w; x++)
	    fb[y * screen_w + x] = ((uint32_t *)ctl->pixels)[i++];
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
