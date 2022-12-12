#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
//  puts("bilt here");
  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
//  uint32_t * srcpix = (uint32_t *)src->pixels;
//  uint32_t * dstpix = (uint32_t *)dst->pixels;
  int sx = (srcrect == NULL ? 0 : srcrect->x);
  int sy = (srcrect == NULL ? 0 : srcrect->y);
  int dx = (dstrect == NULL ? 0 : dstrect->x);
  int dy = (dstrect == NULL ? 0 : dstrect->x);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);
  if(sx < 0) { w += sx; dx -= sx; sx = 0;}
  if(sy < 0) { h += sy; dy -= sy; sy = 0;}
  if(dx < 0) { w += dx; sx -= dx; dx = 0;}
  if(dy < 0) { h += dy; sy -= dy; dy = 0;}
  if(sx >= src->w) return;
  if(sy >= src->h) return;
  if(dx >= dst->w) return;
  if(dy >= dst->h) return;  
  if(src->w - sx < w) { w = src->w - sx;}
  if(src->h - sy < h) { h = src->h - sy;}
  if(dst->w - dx < w) { w = dst->w - dx;}
  if(dst->h - dy < h) { h = dst->h - dy;}
  if(dstrect != NULL){
	dstrect->w = w;
	dstrect->h = h;
  }
  if(dst->format->BitsPerPixel == 32){
    uint32_t * srcpix = (uint32_t *)src->pixels;
    uint32_t * dstpix = (uint32_t *)dst->pixels;
    for(int i = 0; i < h; ++i){ 
	  for(int j = 0; j < w; ++j) {
	    dstpix[dx + j + dst->w * (dy + i)] = srcpix[sx + j + src->w * (sy + i)];
	  }
	}
  }else if(dst->format->BitsPerPixel == 8){
//    uint32_t src_color;
//	uint8_t dst_color;
//	int k;
/*
  	  for(int i = 0; i < src->format->palette->ncolors; ++i){
      printf("src color: %d 0x%02x 0x%02x 0x%02x\n", i, src->format->palette->colors[i].r, src->format->palette->colors[i].g, src->format->palette->colors[i].b);
	} 
	for(int i = 0; i < dst->format->palette->ncolors; ++i){
    printf("dst color: %d 0x%02x 0x%02x 0x%02x\n", i, dst->format->palette->colors[i].r, dst->format->palette->colors[i].g, dst->format->palette->colors[i].b);
	}
*/	
	printf("dst & src:%d %d\n", dst->w, src->w);
	printf("%d %d\n", dst->w * dst->h, w * h);
	printf("dst: %d %d src: %d %d\n", dx, dy, sx, sy);
	uint8_t *srcp = (uint8_t *)src->pixels;
	uint8_t *dstp = (uint8_t *)dst->pixels;
	for(int i = 0; i < h; ++i){
	  for(int j = 0; j < w; ++j){
/*	    src_color = src->format->palette->colors[srcp[sx + j + src->w * (sy + i)]].val;
		k = 0;
		for(; k < dst->format->palette->ncolors; ++k){ 
		  if(src_color == dst->format->palette->colors[k].val)
		    break;	
		}
		assert(k < dst->format->palette->ncolors);
 	    dst_color = (uint8_t)k; */
		dstp[dx + j + dst->w * (dy + i)] = srcp[sx + j + src->w * (sy + i)];
//		printf("%d %d\n", dx + j + dst->w * (dy + i), sx + j + src->w * (sy + i));
 	  }
 	}
   }
//  puts("end blit");
}

void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {
	puts("fill here");
	assert(dst != NULL);
	int w = (dstrect == NULL ? dst->w : dstrect->w);
	int h = (dstrect == NULL ? dst->h : dstrect->h);
	int dx = (dstrect == NULL ? 0 : dstrect->x);
	int dy = (dstrect == NULL ? 0 : dstrect->y);
	if(dx < 0) { w += dx; dx = 0; }
	if(dy < 0) { h += dy; dy = 0; }
	if(dst->w - dx < w) { w = dst->w - dx; }
	if(dst->h - dy < h) { h = dst->h - dy; }
	if(dst->format->BitsPerPixel == 32){
		uint32_t *dstpix = (uint32_t *)dst->pixels;
		for(int i = 0; i  < h; ++i){
			for(int j = 0;  j < w; ++j){
				dstpix[dx + j + dst->w * (dy + i)] = color;
			}
		}
	}else if(dst->format->BitsPerPixel == 8){
		uint8_t rel;
		int i = 0;
		uint8_t *dstp = (uint8_t *)dst->pixels;
		assert(dst->format->palette->ncolors <= 256);
		uint32_t temp = 0;
		uint32_t test = 0;
		for(; i  < dst->format->palette->ncolors; ++i){
			test = 0;
			temp = (uint32_t)dst->format->palette->colors[i].r;
			temp <<= 16;
			test |= temp;
			temp = (uint32_t)dst->format->palette->colors[i].g;
			temp <<= 8;
			test |= temp;
			temp = (uint32_t)dst->format->palette->colors[i].b;
			test |= temp;
			printf("%d %d\n", color, test);
			if(color == test)
				break;
		} 
		if(i >= dst->format->palette->ncolors) return;
		rel = (uint8_t)i;
		for(i = 0; i < h; ++i){
			for(int j = 0; j < w; ++j){
				dstp[dx + j + dst->w * (dy + i)] = rel;
		  	}
		}  
	}
//	puts("end fill");	
}

void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
//	puts("update here");
	if(s->format->BitsPerPixel == 32)
		NDL_DrawRect((uint32_t *)s->pixels, x, y, w, h);
   	else if(s->format->BitsPerPixel == 8){
//		printf("x:%d y:%d w:%d h:%d\n", x, y, w, h);
//	    printf("%d %d %d %d\n", s->format->Rshift, s->format->Gshift, s->format->Bshift, s->format->Ashift);
		if(x == 0 && y == 0 && w == 0 && h == 0) w = 400, h = 300;	
		uint32_t * pix = (uint32_t *)malloc(sizeof(uint32_t) * w * h);
		memset(pix, 0, sizeof(uint32_t) * w * h);
//		printf("pitch:%d\n", s->pitch);
		uint8_t * index = (uint8_t *)s->pixels;
//		printf("pixels:%p\n", s->pixels);
		uint32_t color = 0;
		uint32_t temp = 0;
//		printf("para:%d %d %d %d\n", x, y, w, h);
 		for(int i = 0; i < w * h; ++i){
			color = 0;
			temp = (uint32_t)s->format->palette->colors[index[i]].r;
			temp <<= 16;
			color |= temp;
			temp = (uint32_t)s->format->palette->colors[index[i]].g;
			temp <<= 8;
			color |= temp;
			temp = (uint32_t)s->format->palette->colors[index[i]].b;
			color |= temp;
			pix[i] = color;
//			printf("pix:%d\n", pix[i]);
 		}
		NDL_DrawRect(pix, x, y, w, h);
		free(pix);
	}
//	puts("end update");
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask) {
  switch (mask) {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    case 0x00000000: return 24; // hack
    default: assert(0);
   }
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  assert(depth == 8 || depth == 32);
//  printf("here :%d %d %d %d\n", Rmask, Gmask, Bmask, Amask);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  if (depth == 8) {
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    s->format->palette->ncolors = 256;
  } else {
    s->format->palette = NULL;
    s->format->Rmask = Rmask; s->format->Rshift = maskToShift(Rmask); s->format->Rloss = 0;
    s->format->Gmask = Gmask; s->format->Gshift = maskToShift(Gmask); s->format->Gloss = 0;
    s->format->Bmask = Bmask; s->format->Bshift = maskToShift(Bmask); s->format->Bloss = 0;
    s->format->Amask = Amask; s->format->Ashift = maskToShift(Amask); s->format->Aloss = 0;
   }

  s->format->BitsPerPixel = depth;
  s->format->BytesPerPixel = depth / 8;

  s->w = width;
  s->h = height;
  s->pitch = width * depth / 8;
  assert(s->pitch == width * s->format->BytesPerPixel);

  if (!(flags & SDL_PREALLOC)) {
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  return s;
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
      Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
  if (s != NULL) {
    if (s->format != NULL) {
      if (s->format->palette != NULL) {
        if (s->format->palette->colors != NULL) free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
    free(s);
  }
}

SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
  if (flags & SDL_HWSURFACE) NDL_OpenCanvas(&width, &height);
  return SDL_CreateRGBSurface(flags, width, height, bpp,
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
  if(w == dstrect->w && h == dstrect->h) {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  }
  else {
    assert(0);
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(firstcolor == 0);

  s->format->palette->ncolors = ncolors;
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  if(s->flags & SDL_HWSURFACE) {
    assert(ncolors == 256);
    for (int i = 0; i < ncolors; i ++) {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    SDL_UpdateRect(s, 0, 0, 0, 0);
  } 
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
  int i;
  uint8_t (*pdst)[4] = dst;
  uint8_t (*psrc)[4] = src;
  union {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16) {
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];

    macro(i + 0); macro(i + 1); macro(i + 2); macro(i + 3);
    macro(i + 4); macro(i + 5); macro(i + 6); macro(i + 7);
    macro(i + 8); macro(i + 9); macro(i +10); macro(i +11);
    macro(i +12); macro(i +13); macro(i +14); macro(i +15);
  }

  for (; i < len; i ++) {
    macro(i);
  }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
  assert(src->format->BitsPerPixel == 32);
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

  SDL_Surface* ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  assert(fmt->Gmask == src->format->Gmask);
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
}
