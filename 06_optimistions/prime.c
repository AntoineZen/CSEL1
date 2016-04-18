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

void itoa(long value, char* result) { 

    char* ptr = result, *ptr1 = result, tmp_char; 
    int tmp_value; 
    do 
    { 
        tmp_value = value; 
        value /= 10; 
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz"[35 + (tmp_value - value * 10)]; 
    } 
    while ( value );     

    if (tmp_value < 0) *ptr++ = '-';   // Apply negative sign 
    *ptr-- = '\0'; 

    while(ptr1 < ptr) 
    { 
        tmp_char = *ptr; 
        *ptr--= *ptr1; 
        *ptr1++ = tmp_char; 
    } 
} 

int checkNumbers(long long min, long long max, FILE* fd){
	int  num, div;   
    char prime;
    int prime_count = 0;    

    char buffer[32];

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
            //fprintf(fd, "%d\n", num); 
            itoa(num, buffer);
            int len = strlen(buffer);
            buffer[len] = '\n';
            buffer[len+1] = '\0';
            fwrite(buffer, 1,  len+1, fd);
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
