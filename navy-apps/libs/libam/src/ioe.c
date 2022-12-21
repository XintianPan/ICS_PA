#include <am.h>

void __am_timer_init();
void __am_gpu_init();
void __am_audio_init();
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);
void __am_timer_rtc(AM_TIMER_RTC_T *);
void __am_timer_uptime(AM_TIMER_UPTIME_T *);
void __am_gpu_config(AM_GPU_CONFIG_T *);
void __am_gpu_status(AM_GPU_STATUS_T *);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);
void __am_audio_config(AM_AUDIO_CONFIG_T *);
void __am_audio_ctrl(AM_AUDIO_CTRL_T *);
void __am_audio_status(AM_AUDIO_STATUS_T *);
void __am_audio_play(AM_AUDIO_PLAY_T *);
void __am_disk_config(AM_DISK_CONFIG_T *cfg);
void __am_disk_status(AM_DISK_STATUS_T *stat);
void __am_disk_blkio(AM_DISK_BLKIO_T *io);

static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) {
    cfg->present = true;
    cfg->has_rtc = true;
}
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = true; }
static void __am_uart_config(AM_UART_CONFIG_T *cfg) { cfg->present = false; }
static void __am_net_config(AM_NET_CONFIG_T *cfg) { cfg->present = false; }

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
    [AM_TIMER_CONFIG] = __am_timer_config,
    [AM_TIMER_RTC] = __am_timer_rtc,
    [AM_TIMER_UPTIME] = __am_timer_uptime,
    [AM_INPUT_CONFIG] = __am_input_config,
    [AM_INPUT_KEYBRD] = __am_input_keybrd,
    [AM_GPU_CONFIG] = __am_gpu_config,
    [AM_GPU_FBDRAW] = __am_gpu_fbdraw,
    [AM_GPU_STATUS] = __am_gpu_status,
    [AM_UART_CONFIG] = __am_uart_config,
    [AM_AUDIO_CONFIG] = __am_audio_config,
    [AM_AUDIO_CTRL] = __am_audio_ctrl,
    [AM_AUDIO_STATUS] = __am_audio_status,
    [AM_AUDIO_PLAY] = __am_audio_play,
    [AM_DISK_CONFIG] = __am_disk_config,
    [AM_DISK_STATUS] = __am_disk_status,
    [AM_DISK_BLKIO] = __am_disk_blkio,
    [AM_NET_CONFIG] = __am_net_config,
};

static void fail(void *buf) { panic("access nonexist register"); }

bool ioe_init() {
    for (int i = 0; i < LENGTH(lut); i++)
        if (!lut[i]) lut[i] = fail;
    __am_gpu_init();
    __am_timer_init();
    __am_audio_init();
    return true;
}

void ioe_read(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }

void __am_disk_config(AM_DISK_CONFIG_T *cfg) {
    cfg->present = false;
}

void __am_disk_status(AM_DISK_STATUS_T *stat) {
}

void __am_disk_blkio(AM_DISK_BLKIO_T *io) {
}

#define AUDIO_FREQ_ADDR (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR (AUDIO_ADDR + 0x14)
// #define AUDIO_READ_ADDR      (AUDIO_ADDR + 0x18)
// #define AUDIO_WRITE_ADDR     (AUDIO_ADDR + 0x1c) // this can avoid interfering with read

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
    while (sb_size - (int)inl(AUDIO_COUNT_ADDR) < len)
        ;
    uint8_t *s = (uint8_t *)ctl->buf.start;
    int i = 0;
    for (; i < len; ++i) {
        outb(AUDIO_SBUF_ADDR + (w_ptr + i) % sb_size, *s);
        ++s;
    }
    w_ptr = (w_ptr + len) % sb_size;
    outl(AUDIO_COUNT_ADDR, inl(AUDIO_COUNT_ADDR) + len);
}

#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
    *cfg = (AM_GPU_CONFIG_T){
        .present = true, .has_accel = false, .width = 400, .height = 300, .vmemsz = 0};
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
    if (ctl->pixels != NULL) {
        uint32_t *pix = (uint32_t *)(ctl->pixels);
        uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
        int x, y, w, h;
        x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
        for (int i = 0; i < h; ++i) {
            for (int j = 0; j < w; ++j) {
                fb[x + j + 400 * (y + i)] = pix[j + w * i];
            }
        }
    }
    if (ctl->sync) {
        outl(SYNC_ADDR, 1);
    }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
    status->ready = true;
}

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    uint32_t k = inl(KBD_ADDR);
    if (k & KEYDOWN_MASK) {
        kbd->keydown = true;
        kbd->keycode = k ^ KEYDOWN_MASK;
    } else {
        kbd->keydown = false;
        kbd->keycode = k;
    }
}

void __am_timer_init() {
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
    uint64_t upt = (uint64_t)(inl(RTC_ADDR + 4));
    upt <<= 4;
    uptime->us = (uint64_t)inl(RTC_ADDR) + upt;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
    rtc->second = 0;
    rtc->minute = 0;
    rtc->hour = 0;
    rtc->day = 0;
    rtc->month = 0;
    rtc->year = 1900;
}