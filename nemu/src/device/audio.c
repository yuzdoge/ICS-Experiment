#include <common.h>

#ifdef HAS_IOE

#include <device/map.h>
#include <SDL2/SDL.h>

#define AUDIO_PORT 0x200 // Note that this is not the standard
#define AUDIO_MMIO 0xa1000200
#define STREAM_BUF 0xa0800000
#define STREAM_BUF_MAX_SIZE 65536

enum {
  reg_freq,
  reg_channels,
  reg_samples,
  reg_sbuf_size,
  reg_init,
  reg_count,
  nr_reg
};

static uint8_t *sbuf = NULL;
static uint32_t *audio_base = NULL;

static volatile int count = 0; 
static int qhead = 0;

static inline void audio_play(void *userdata, uint8_t *stream, int len) {
  int nread = count < len ? count : len; 
  for (int i = 0; i < nread; i++) {
    stream[i] = sbuf[qhead];
    qhead = (qhead + 1) % STREAM_BUF_MAX_SIZE;
  }

  count -= nread;
  if (len > nread) memset(stream + nread, 0, len - nread); //if audio data is not enough, fill with silence data.
}

static SDL_AudioSpec s = {.format = AUDIO_S16SYS, .userdata = NULL, .callback = audio_play};
#define reg_off(reg_name) (reg_name * sizeof(uint32_t))

static void audio_io_handler(uint32_t offset, int len, bool is_write) {
  assert(len == 4);
  switch(offset) {
    case reg_off(reg_freq): case reg_off(reg_channels): case reg_off(reg_samples): 
	  if (!is_write) panic("don't support read"); break;
	case reg_off(reg_init):
	  if (!is_write) panic("don't support read");
	  else {
	    if (audio_base[reg_init]) {
		  //initialize audio device
		  s.freq = audio_base[reg_freq];
		  s.channels = audio_base[reg_channels];
		  s.samples = audio_base[reg_samples];
		  Assert(SDL_OpenAudio(&s, NULL) >= 0, "can't open audio");
          SDL_PauseAudio(0);
		}
		audio_base[reg_init] = 0;
	  }
	  break;
	case reg_off(reg_sbuf_size): case reg_off(reg_count):
	  if (is_write) panic("don't support write"); 
	  else if (offset == reg_off(reg_count)) {
	    audio_base[reg_count] = count;
	  }
	  break;
	default: panic("don't support offset = %d", offset);
  }
}

static void audio_sbuf_handler(uint32_t offset, int len, bool is_write) {
  assert(is_write && offset + len <= STREAM_BUF_MAX_SIZE);
  count += len;
}

void init_audio() {
  uint32_t space_size = sizeof(uint32_t) * nr_reg;
  audio_base = (void *)new_space(space_size);
  audio_base[reg_sbuf_size] = STREAM_BUF_MAX_SIZE; //stream buffer size is unvariable
  
  add_pio_map("audio", AUDIO_PORT, (void *)audio_base, space_size, audio_io_handler);
  add_mmio_map("audio", AUDIO_MMIO, (void *)audio_base, space_size, audio_io_handler);

  sbuf = (void *)new_space(STREAM_BUF_MAX_SIZE);
  add_mmio_map("audio-sbuf", STREAM_BUF, (void *)sbuf, STREAM_BUF_MAX_SIZE, audio_sbuf_handler);

  Assert(SDL_InitSubSystem(SDL_INIT_AUDIO) == 0, "fail to initialize audio");
}
#endif	/* HAS_IOE */
