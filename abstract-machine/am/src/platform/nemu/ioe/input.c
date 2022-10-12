#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
	uint32_t input = inl(KBD_ADDR);
	if(kbd->keydown)
		kbd->keycode = input ^ KEYDOWN_MASK;
	else
		kbd->keycode = input;
}
