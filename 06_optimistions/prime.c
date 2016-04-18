/* Prim1.c: simple program to calculate prime numbers */
#include <stdio.h> 
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

/*Numbers to test */
//#define MIN_NUM 1000000
//#define MAX_NUM 10*MIN_NUM

#define MIN_NUM 1
#define MAX_NUM 320000

int checkNumbers(long long min, long long max, FILE* fd){
	int  num, div;   
    char prime;
    int prime_count = 0;    
	for(num = min; num < max; num++) {
		prime = 1; 
    	for(div = 2; div < num/2; div++) {
	     	if(num%div==0){ 
			    prime=0; 
			    break; 
            }
    	}

		if (prime==1) 
        {
            prime_count++;
            fprintf(fd, "%d\n", num); 
        }
	}
    return prime_count;
}

int main()
{
    struct timespec rt1, rt2;
    long long time1;


    printf("There is %li cores\n", sysconf(_SC_NPROCESSORS_ONLN));

 
    FILE* f = fopen("prime_num.txt", "w");
    clock_gettime(CLOCK_REALTIME, &rt1);  
    checkNumbers(MIN_NUM, MAX_NUM, f); 
    clock_gettime(CLOCK_REALTIME, &rt2);  
    time1 = (long long int)(rt2.tv_sec - rt1.tv_sec)*1000000000 + (rt2.tv_nsec - rt1.tv_nsec);
    printf("%.3f ms\n", (double)(time1)/1000000);

    fclose(f);
}
