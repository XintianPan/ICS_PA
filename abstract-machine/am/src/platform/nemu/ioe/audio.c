#include <am.h>
#include <nemu.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)
#define AUDIO_READ_ADDR      (AUDIO_ADDR + 0x18)
#define AUDIO_WRITE_ADDR     (AUDIO_ADDR + 0x1c)

void __am_audio_init() {
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
	outl(AUDIO_FREQ_ADDR, ctrl->freq);
	outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
	outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
	outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
	int sb_size = inl(AUDIO_SBUF_SIZE_ADDR);
	int len = ctl->buf.end - ctl->buf.start;
	int w = inl(AUDIO_WRITE_ADDR);
	while(sb_size - (int)inl(AUDIO_COUNT_ADDR) < len) ;
	uint8_t *s = (uint8_t *)ctl->buf.start;
	int i = 0;
	for(; i < len; ++i){ 
		outb(AUDIO_SBUF_ADDR + (w + i) % sb_size, *s);
		++s;
	}
	outl(AUDIO_WRITE_ADDR, w + len);
	outl(AUDIO_COUNT_ADDR, inl(AUDIO_COUNT_ADDR) + len);
}
