/* Prim1.c: simple program to calculate prime numbers */
#include <time.h>

#include <math.h>
#include <pthread.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*Numbers to test */
#define MIN_NUM 1000000
#define MAX_NUM 10*MIN_NUM

//#define MIN_NUM 1
//#define MAX_NUM 320000

#define BUFFER_SIZE 4096

void itoa(int value, char* result) { 

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

inline int is_prime(int n)
{
    register int i = 5;
    if (n <= 1)
    {
        return 0;
    }
    else if (n <= 3)
    {
        return 1;
    }
    else if( n%2 == 0 || n%3 == 0)
    {
        return 0;
    }
    while (i*i <= n)
    {
        if (n%i == 0 || n%(i+2) == 0)
            return 0;
        i += 6;
    }
    return 1;
}


int checkNumbers(int min, int max, FILE* fd){
	register int  num;   
    register int prime_count = 0;    

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    char* buffer_ptr = buffer;

    // if min is even, skip it
    if( (min & 0x01) == 0)
    {
        min +=1;
    }

	for(num = min; num < max; num+=2) {
		if (is_prime(num)) 
        {
            prime_count++;

            //printf("%d\n", num); 
            itoa(num, buffer_ptr);
            int len = strlen(buffer_ptr);
            buffer_ptr[len] = '\n';
            buffer_ptr[len+1] = '\0';
            buffer_ptr += len+1;
            //fwrite(buffer,  1, len+1, fd);
            if (buffer_ptr > buffer + BUFFER_SIZE - 32)
            {
                fwrite(buffer, 1, buffer_ptr - buffer, fd);
                memset(buffer, 0, BUFFER_SIZE);
                buffer_ptr = buffer;
            }
        }

	}

    fwrite(buffer, 1, buffer_ptr - buffer, fd);

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
    if( f == NULL)
    {
        printf("Unable to open \"prime_num.txt\" !\n");
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &rt1); 

    int slice = (MAX_NUM+1 - MIN_NUM) / n_cpu; 

    // Start the threads
    for(int i=0; i < n_cpu; i++)
    {
        task_args[i].n = i;
        task_args[i].min = MIN_NUM + i * slice;
        if(i == n_cpu-1)
            task_args[i].max = MAX_NUM;
        else
            task_args[i].max = MIN_NUM + (i+1) * slice -1;
        task_args[i].fd = f;
        pthread_create(threads+i, NULL, prime_task, &task_args[i]);
    }
    int prime_count = 0;
    // Wait for the threads to end
    for(int i=0; i < n_cpu; i++)
    {
        pthread_join(threads[i], NULL);
        prime_count +=task_args[i].ret;
    }
    clock_gettime(CLOCK_REALTIME, &rt2);  

    printf("There is %d prime number between %d and %d.\n", prime_count, MIN_NUM, MAX_NUM);
    time1 = (long long int)(rt2.tv_sec - rt1.tv_sec)*1000000000 + (rt2.tv_nsec - rt1.tv_nsec);
    printf("%.3f ms\n", (double)(time1)/1000000);

    fclose(f);
}
