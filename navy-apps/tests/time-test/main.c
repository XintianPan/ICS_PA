#include <stdio.h>
#include <time.h>
#include <sys/time.h>

int main(){
	struct timeval tv;
	long pus, ous;
	gettimeofday(&tv, NULL);
	pus = tv.tv_usec;
	while(1){
		gettimeofday(&tv, NULL);
		ous = tv.tv_usec;
		if((ous - pus) / 1000 >= 500){
			printf("Get here Man!\n");
			pus = ous;
		}
	}
	return 0;
}
