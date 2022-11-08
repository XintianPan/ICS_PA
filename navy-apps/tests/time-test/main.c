#include <stdio.h>
#include <stdint.h>
#include <NDL.h>
#include <time.h>

int main(){
	uint32_t pms, oms;
	int i = 0;
	printf("%u\n", sizeof(struct timeval));
	NDL_Init(0);
	pms = NDL_GetTicks();
	while(1){ 
		oms = NDL_GetTicks();
	 	if((oms - pms)>= 500){
			printf("Get here Man: %d!\n", i);
			++i;
			pms = oms;
		}
	}
	NDL_Quit();
	return 0;
}
