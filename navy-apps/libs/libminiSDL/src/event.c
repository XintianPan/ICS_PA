#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <stdio.h>
#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
	char buf[64];
	int i = 0;
	while(NDL_PollEvent(buf, 64) == 0) ++i;
	char *kev;
	char *key_name;
	kev = strtok(buf, " ");
	key_name = strtok(NULL, " ");
	if(strcmp(kev, "kd") == 0)  event->key.type = SDL_KEYDOWN;
    else 	event->key.type = SDL_KEYUP;
    int len = sizeof(keyname) / sizeof(keyname[0]);
	i = 0;
	printf("%s\n", key_name);
	for(; i < len; ++i){
		if(strcmp(keyname[i], key_name) == 0) break;
	}	
	event->key.keysym.sym = i;
	return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
