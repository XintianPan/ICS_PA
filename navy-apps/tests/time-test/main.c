#include <stdio.h>
#include <stdint.h>
#include <NDL.h>

int main(){
	uint32_t pms, oms;
	int i = 0;
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
