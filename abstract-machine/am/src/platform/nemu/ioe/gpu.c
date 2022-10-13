#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = 400, .height = 300,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
	if(ctl->pixels != NULL){
		uint32_t *pix = (uint32_t *)(ctl->pixels);
		uint32_t *fb = (uint32_t *)(uintptr_t) FB_ADDR;
		int x , y, w, h;
		x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
		for(int i = 0; i < h; ++i){
			for(int j = 0; j < w; ++j){
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
