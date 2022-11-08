#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
//#define AUDIO_READ_ADDR      (AUDIO_ADDR + 0x18)
//#define AUDIO_WRITE_ADDR     (AUDIO_ADDR + 0x1c) // this can avoid interfering with read

static uint32_t w_ptr = 0;
static uint32_t sb_size = 0;

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
	outl(AUDIO_FREQ_ADDR, ctrl->freq);
	outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
	outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
	outb(AUDIO_INIT_ADDR, 1);
	sb_size = inl(AUDIO_SBUF_SIZE_ADDR);
	w_ptr = 0;
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
	uint32_t len = ctl->buf.end - ctl->buf.start;
	while(sb_size - (int)inl(AUDIO_COUNT_ADDR) < len) ;
	uint8_t *s = (uint8_t *)ctl->buf.start;
	int i = 0;
	for( ; i < len; ++i){ 
		outb(AUDIO_SBUF_ADDR + (w_ptr + i) % sb_size, *s);
		++s;
	} 
	w_ptr = (w_ptr + len) % sb_size;
	outl(AUDIO_COUNT_ADDR, inl(AUDIO_COUNT_ADDR) + len);
}
