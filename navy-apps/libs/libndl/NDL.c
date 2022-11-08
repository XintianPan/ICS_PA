#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
int wh_mes[2];

uint32_t NDL_GetTicks() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	uint32_t msec = (uint32_t)((uint32_t)tv.tv_usec / 1000);
	return msec;
}

int NDL_PollEvent(char *buf, int len) {
  return read(evtdev, buf, len);
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_ APP")) { 
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) { 
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }else{
//    printf("cur size:%d %d\n", *w, *h);
	if(*w == 0 && *h == 0){
		*w = screen_w, *h = screen_h;
//		printf("reshpe size:%d %d\n", *w, *h);
	}
	wh_mes[0] = *w;
	wh_mes[0] = *h;
	write(fbdev, wh_mes, 0);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
}

void NDL_OpenAudio(int freq, int channels, int samples) {
} 

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
	if (getenv("NWM_APP")) {
		evtdev = 3;
	}else{
		evtdev = open("/dev/events", 0, 0);
		fbdev = open("/dev/fb", 0, 0);
		char buf[64];
		int fd = open("/proc/dispinfo", 0, 0);
		read(fd, buf, sizeof(buf));
		strtok(buf, ":");
		char *wid = strtok(NULL, "\n");
		sscanf(wid, "%d", &screen_w);
		strtok(NULL, ":");
		char *hei = strtok(NULL, "\n");
		sscanf(hei, "%d", &screen_h);
		close(fd);
	}
	return 0;
}

void NDL_Quit() {
	close(evtdev);
	evtdev = -1;
	close(fbdev);
	fbdev = -1;
}
