#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char _std_num_buf[20];
static char _std_out_buf[1000000 + 5];
static char *_std_num_lo = "0123456789abcdef";
//static char *_std_num_up = "0123456789ABCDEF";


int printf(const char *fmt, ...) {
	va_list ap;
	int done;
	va_start(ap, fmt);
		done = vsprintf(_std_out_buf, fmt, ap);
	va_end(ap);
    putstr(_std_out_buf);
	return done;
} 

int vsprintf(char *out, const char *fmt, va_list ap) {
 	char *s;
	int d;
	char c;
	size_t i = 0;
	int pad = 0;
	size_t j;
	while(*fmt){
		if(*fmt == '%'){
			++fmt;
			switch(*fmt){
				case '0':
					++fmt;
					pad = *fmt - '0';
					++fmt;
					putch('h');
					putch('\n');
					d = va_arg(ap, int);
					j = -1;
					do{
						_std_num_buf[++j] = _std_num_lo[ d % 10];
						d /= 10;
					}while(d);
					putch('o');
					putch('\n');
					for(int k = pad - 1; k > j; --k)
						out[i++] = '0';
					for(int k = j; j >= 0; --k)
						out[i++] = _std_num_buf[k];
					break;
				case 'c':
					c = (char)va_arg(ap, int);
					out[i++] = c;
					break;
				case 'd':
					d = va_arg(ap, int);
					j = -1;
					do{
						_std_num_buf[++j] = _std_num_lo[ d % 10];
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

int sprintf(char *out, const char *fmt, ...) {
	va_list ap;
	int done;
	va_start(ap, fmt);
		done = vsprintf(out, fmt, ap);
	va_end(ap);
	return done;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
