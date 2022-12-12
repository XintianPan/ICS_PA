#include <NDL.h>
#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

uint8_t key_arr[256];

int SDL_PushEvent(SDL_Event *ev) {
	assert(0);
	return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  	char buf[64];
	int i = 0;
	if(NDL_PollEvent(buf, 64) == 0) return 0;
	char *kev;
	char *key_name;
	printf("%s\n", buf);
	kev = strtok(buf, " ");
	printf("cmp rel:%d\n", strcmp(kev, "kd"));
	key_name = strtok(NULL, " ");
	if(strcmp(kev, "kd") == 0)  ev->key.type = SDL_KEYDOWN;
    else 	ev->key.type = SDL_KEYUP;
    int len = sizeof(keyname) / sizeof(keyname[0]);
//	printf("%s\n", key_name);
 	for(; i < len; ++i){ 
//		printf("%s\n", keyname[i]);
		if(strcmp(keyname[i], key_name) == 0) break;
	} 	
	ev->key.keysym.sym = i;
	printf("%d %d\n", i, SDLK_SPACE);
	if(strcmp(kev, "kd") == 0) key_arr[i] = 1, puts("here");
	else key_arr[i] = 0;
	puts("reach here");
	return 1;
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
//	printf("%s\n", key_name);
 	for( ; i < len; ++i){ 
//		printf("%s\n", keyname[i]);
		if(strcmp(keyname[i], key_name) == 0) break;
	}	
	event->key.keysym.sym = i;
	return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
	assert(0);
	return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
//	size_t size = (numkeys == NULL) ? sizeof(keyname) / sizeof(keyname[0]) : *numkeys;
//	uint8_t *arr = (uint8_t *)malloc(sizeof(uint8_t) * size);
//	memset(arr, 0, sizeof(uint8_t) * size);
//	SDL_Event ev;
	puts("use this function");
	printf("%d\n", key_arr[SDLK_SPACE]);
	return key_arr;
//	if(SDL_PollEvent(&ev) == 0)
//		return arr;
 //	else{ 
//		if(ev.key.type = SDL_KEYDOWN)
//		arr[ev.key.keysym.sym] = 1;
//		printf("SDL_Space:%d\n", arr[SDLK_SPACE]);
//		return arr;
//	}
}
