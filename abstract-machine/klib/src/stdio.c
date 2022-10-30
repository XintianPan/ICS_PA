#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define NEG_DETER 0x80000000


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
	bool flag = false;
	uintptr_t p;
	uint32_t u;
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
					u = va_arg(ap,unsigned int);
					flag = false;
					if(u & NEG_DETER){
						flag = true;
						u = (~u) + 1;
					}
					j = -1;
					do{
						_std_num_buf[++j] = _std_num_lo[ u % 10];
						u /= 10;
	 				}while(u);
					if(flag) _std_num_buf[++j] = '-';
					for(int k = pad - 1; k > j; --k)
						out[i++] = '0';
					for(int k = j; k >= 0; --k)
						out[i++] = _std_num_buf[k];
					break;
				case 'c':
					c = (char)va_arg(ap, int);
					out[i++] = c;
					break;
				case 'd':
					u = va_arg(ap, unsigned int);
					flag = false;
					if(u & NEG_DETER){
						flag = true;
						u = (~u) + 1;
					}
					j = -1;
					do{
						_std_num_buf[++j] = _std_num_lo[ u % 10];
						u /= 10;
	 				}while (u);
					if(flag) _std_num_buf[++j] = '-';
					for(register int k = j; k >= 0; --k)
						out[i++] = _std_num_buf[k];
					break;
				case 'u':
					u = va_arg(ap, unsigned int);
					j = -1;
					do{
						_std_num_buf[++j] = _std_num_lo[ u % 10];
						u /= 10;
					}while(u);
					for(register int k = j; k >= 0; --k)
						out[i++] = _std_num_buf[k];
					break;
				case 'x':
					u = va_arg(ap, unsigned int);
					j = -1;
					do{
						_std_num_buf[++j] = _std_num_lo[ u % 16];
						u /= 16;
					}while(u);
					for(register int k = j; k >= 0; --k)
						out[i++] = _std_num_buf[k];
					break;
				case 'p':
					p = va_arg(ap, uintptr_t);
					j = -1;
					do{
						_std_num_buf[++j] = _std_num_lo[p % 16];
						p /= 16;
					}while(p);
					_std_num_buf[++j] = 'x';
					_std_num_buf[++j] = '0';
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
  va_list ap;
  int done;
  va_start(ap, fmt);
	done = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return done;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
	char *s;
	char c;
	int d;
	size_t i = 0;
	int pad = 0;
	size_t j;
	while(*fmt && i < n - 1){
		if(*fmt == '%'){
			++fmt;
			switch(*fmt){
				case '0':
					++fmt;
					pad = *fmt - '0';
					++fmt;
					d = va_arg(ap, int);
					j = -1;
					do{
						_std_num_buf[++j] = _std_num_lo[ d % 10];
						d /= 10;
	 				}while(d);
					for(int k = pad - 1; k > j; --k)
						out[i++] = '0';
					for(int k = j; k >= 0; --k)
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

#endif
