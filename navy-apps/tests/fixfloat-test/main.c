#include <stdio.h>
#include <assert.h>
#include <fixedptc.h>
int main(){
	fixedpt a = fixedpt_rconst(1.5);
	fixedpt b = fixedpt_rconst(-1.5);
	assert(fixedpt_floor(a) == fixedpt_rconst(1));
	assert(fixedpt_ceil(a) == fixedpt_rconst(2));
	assert(fixedpt_ceil(b) == fixedpt_rconst(-1));
	assert(fixedpt_floor(b) == fixedpt_rconst(-2));
	return 0;
}
