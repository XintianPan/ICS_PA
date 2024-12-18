#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(0);
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
	FILE *fp = fopen(filename, "r");
	fseek(fp, 0, SEEK_END);
	int s = ftell(fp);
	unsigned char * buf = SDL_malloc(s);
	fseek(fp, 0, SEEK_SET);
	fread(buf, 1, s, fp);
	SDL_Surface * suf = STBIMG_LoadFromMemory(buf, s);
	SDL_free(buf);
	fclose(fp);
	return suf;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
