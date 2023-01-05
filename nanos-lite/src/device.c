#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static char events[64];

static char *dispinfo_mes = "WIDTH : 400\n""HEIGHT : 300\n";

static int w = 0;

static int h = 0;

uint32_t fb_buf[400 * 300];

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
}; 

size_t serial_write(const void *buf, size_t offset, size_t len) {
//	Log("here");
	yield();
	size_t i = 0;
	const char *serial_wbuf = (const char *)buf;
	for(; i < len; ++i){
		putch(*(serial_wbuf + i));
	}
	return i;
}

size_t events_read(void *buf, size_t offset, size_t len) {
//	Log("this comes");
//	yield();
	AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);	
	memset(buf, 0, len);
	if(ev.keycode == AM_KEY_NONE)
		return 0;
	else{
		memset(events, 0, sizeof(events));
		if(ev.keydown){
//			Log("ok here");
			sprintf((char *)events, "kd %s", keyname[ev.keycode]);
		}else{
			sprintf((char *)events, "ku %s", keyname[ev.keycode]);
		}
		int i = 0;
		char *w_buf = (char *)buf; 
		for(; i < len && events[i]; ++i)
			w_buf[i] = events[i];
		return 1;
	}
} 

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
	size_t slen = strlen(dispinfo_mes);	
	size_t i = 0;
	char *wbuf = (char *)buf;
	for(; i < slen && i < len; ++i)
		*(wbuf + i) = *(dispinfo_mes + i);
	for(; i < len; ++i)
		*(wbuf + i) = '\0';
	return i;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
//	Log("what");
//	yield();
	//    printf("fb address: %p\n", fb_buf);
	if(len == 0){ // use len = 0 to specify the input of w and h from canvas 
		int *arr = (int *)buf;
		w = arr[0];
		h = arr[1];
//		printf("%d %d\n", w, h);
		return 0;
	}else{
		offset /= sizeof(uint32_t);
		int x = offset % 400;
		int y = offset / 400;
//		Log("%d %d", x, y);
		const uint32_t* draw_buf = (const uint32_t* )buf;
//		Log("%d", draw_buf[0]);
 		for(int i = 0; i < h; ++i){
 			for(int j = 0; j < w; ++j){
				fb_buf[x + j + 400 * (y + i)] = draw_buf[j + w * i];
 			}
 		}
		io_write(AM_GPU_FBDRAW, 0, 0, fb_buf, 400, 300, true);
		return len;
 	}
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
