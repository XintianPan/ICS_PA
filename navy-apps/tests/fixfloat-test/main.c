#include <stdio.h>
#include <assert.h>
#include <fixedptc.h>
int main(){
	fixedpt a = fixedpt_rconst(1.5);
	fixedpt b = fixedpt_rconst(-1.5);
	fixedpt c = fixedpt_rconst(0);
	fixedpt d = fixedpt_rconst(1);
	fixedpt e = fixedpt_rconst(-1);
	fixedpt f = fixedpt_rconst(100.590234);
	fixedpt g = fixedpt_rconst(-1002.32423599);
	assert(fixedpt_floor(a) == fixedpt_rconst(1));
	assert(fixedpt_floor(c) == fixedpt_rconst(0));
	assert(fixedpt_floor(d) == fixedpt_rconst(1));
	assert(fixedpt_floor(e) == fixedpt_rconst(-1));
	assert(fixedpt_floor(f) == fixedpt_rconst(100));
	assert(fixedpt_floor(g) == fixedpt_rconst(-1003));
//	assert(fixedpt_ceil(a) == fixedpt_rconst(2));
//	assert(fixedpt_ceil(b) == fixedpt_rconst(-1));
	assert(fixedpt_floor(b) == fixedpt_rconst(-2));
	assert(fixedpt_ceil(a) == fixedpt_rconst(2));
	assert(fixedpt_ceil(c) == fixedpt_rconst(0));
	assert(fixedpt_ceil(d) == fixedpt_rconst(1));
	assert(fixedpt_ceil(e) == fixedpt_rconst(-1));
	assert(fixedpt_ceil(f) == fixedpt_rconst(101));
	assert(fixedpt_ceil(g) == fixedpt_rconst(-1001));
	return 0;
}
