#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	size_t len = 1;
	while(s[len - 1] != '\0') ++len;
	return len - 1;
} 

char *strcpy(char *dst, const char *src) {
	size_t i;
	for(i = 0; src[i] != '\0'; ++i)
		dst[i] = src[i];
	dst[i] = '\0';
	return dst;
} 

char *strncpy(char *dst, const char *src, size_t n) {
	size_t i;
	for(i = 0; src[i] != '\0' && i < n; ++i)
		dst[i] = src[i];
	for(; i < n; ++i)
		dst[i] = '\0';
	return dst;
}

char *strcat(char *dst, const char *src) {
	size_t i;
	size_t dst_len = strlen(dst);
	for(i = 0; src[i] != '\0'; ++i)
		dst[dst_len + i] = src[i];
	dst[dst_len + i] = '\0';
	return dst;
}

int strcmp(const char *s1, const char *s2) {
  for(; *s1 == *s2; ++s1, ++s2){
	if(*s1 == '\0')
		return 0;
  }
  if((uint8_t)(*s1) < (uint8_t)(*s2))
		  return -1;
  else
	return 1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	size_t i;
	for(i = 0; s1[i] == s2[i] && i < n; ++i){
		if(s1[i] == '\0')
			return 0;
	}
	if(i < n)
		return s1[i] - s2[i];
	else
		return 0; // first n bytes are equal
}

void *memset(void *s, int c, size_t n) {
	char *padd = (char *)s;
	for(register int i = 0; i < n; ++i)
		padd[i] = c;
	return s;
} 

void *memmove(void *dst, const void *src, size_t n) {
	char *out = (char *)dst;
	const char *in = (const char *)src;
	char *buf = (char *)malloc(n + 1);
	buf[n] = '\0';
	size_t i;
	for(i = 0; i < n; ++i)
		buf[i] = in[i];
	for(i = 0; i < n; ++i)
		out[i] = buf[i];
	free(buf);
	return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
	char *dst = (char *)out;
	const char *src = (const char *)in;
    size_t i;
//	printf("what\n");
//	printf("%p %p\n", out, in);
	for(i = 0; i < n; ++i)
		dst[i] = src[i];
	return out;	
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const char * c1 = (const char *)s1;
	const char * c2 = (const char *)s2;
	size_t i;
	while(c1[i] == c2[i] && i < n) ++i;
	if(i < n)
		return (int)c1[i] - (int)c2[i];
	else
		return 0;
}

#endif
