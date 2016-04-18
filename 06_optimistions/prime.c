/* Prim1.c: simple program to calculate prime numbers */
#include <time.h>

#include <math.h>
#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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


typedef struct 
{
    int n;
   int min;
   int max;
   FILE* fd;
   int ret;
}
prime_task_args;

void* prime_task(void* param)
{

    prime_task_args* args = (prime_task_args*)param;

    printf("This is thread %d computing from %i to %i\n", args->n, args->min, args->max);
    args->ret =  checkNumbers(args->min, args->max, args->fd);
    return param;
}

int main()
{
    pthread_t threads[32];
    prime_task_args task_args[32];


    struct timespec rt1, rt2;
    long long time1;

    int n_cpu = sysconf(_SC_NPROCESSORS_ONLN);


    printf("There is %i cores\n", n_cpu);

 
    FILE* f = fopen("prime_num.txt", "w");
    clock_gettime(CLOCK_REALTIME, &rt1); 

    int slice = (MAX_NUM - MIN_NUM) / n_cpu; 

    for(int i=0; i < n_cpu; i++)
    {
        task_args[i].n = i;
        task_args[i].min = MIN_NUM + i * slice;
        task_args[i].max = MIN_NUM + (i+1) * slice -1;
        task_args[i].fd = f;
        pthread_create(threads+i, NULL, prime_task, &task_args[i]);
    }
    checkNumbers(MIN_NUM, MAX_NUM, f); 
    clock_gettime(CLOCK_REALTIME, &rt2);  
    time1 = (long long int)(rt2.tv_sec - rt1.tv_sec)*1000000000 + (rt2.tv_nsec - rt1.tv_nsec);
    printf("%.3f ms\n", (double)(time1)/1000000);

    fclose(f);
}
