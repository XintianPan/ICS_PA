#include <am.h>

Area heap;
#ifndef MAINARGS
#define MAINARGS ""
#endif
void putch(char ch) {
    printf("%c", ch);
}

void halt(int code) {
    return 0;
}
