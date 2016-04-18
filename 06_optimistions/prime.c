/* Prim1.c: simple program to calculate prime numbers */
#include <stdio.h> 
#include <time.h>
#include <stdlib.h>
#include <math.h>

/*Numbers to test */
#define MIN_NUM 1
#define MAX_NUM 40000

int checkNumbers(min,max){
	int  num, div;   
	char prime; 
	for(num = min; num < max; num++) {
		prime = 1; 
        	for(div = 2; div < num/2; div++) {
       		     	if(num%div==0){ 
				prime=0; 
				break; 
			} 
        	}
		//if (prime==1) printf("p: %d\n", num); 
	}
}

int main()
{
    int  i;
    struct timespec rt1, rt2;
    long long time1;
   
    clock_gettime(CLOCK_REALTIME, &rt1);  
    checkNumbers(MIN_NUM, MAX_NUM); 
    clock_gettime(CLOCK_REALTIME, &rt2);  
    time1 = (long long int)(rt2.tv_sec - rt1.tv_sec)*1000000000 + (rt2.tv_nsec - rt1.tv_nsec);
    printf("%.3f ms\n", (double)(time1)/1000000);
    return 0;
}
