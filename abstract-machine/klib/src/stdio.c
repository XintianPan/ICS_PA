#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char _std_num_buf[20];

int printf(const char *fmt, ...) {
  panic("Not implemented");
} 

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	char *s;
	int d;
	size_t j = -1;
	size_t i = 0;
	va_start(ap, fmt);
	while(*fmt){
		if(*fmt == '%'){
			++fmt;
			switch(*fmt){
				case 'd':
					d = va_arg(ap, int);
					j = -1;
					do{
						_std_num_buf[++j] = d % 10 + '0';
						d /= 10;
					}while(d);
					for(register int k = j; k >= 0; --k)
						out[i++] = _std_num_buf[k];
					break;
				case 's':
					s = va_arg(ap, char *);
					while(*s){
						out[i++] = *s;
						++s;
					}
					break;
				case '%':
					out[i++] = *fmt;
					break;
				default:
					break;
			}
		}else{
			out[i++] = *fmt; 
		}
		++fmt;	
	}
	out[i] = '\0';
	return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
