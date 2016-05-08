Optimizations
=============

In this lab, we get the source code of a program that compute the list of all prime numbers in a given range (start and stop number). This program is sub-obtimal, the goal it to apply the techniques learned in the lecture to optimize the program. For each optimization, we give the description of the change the execution time of the program and the gain in percent.

The goal of the lab is to make a program that compute the prime number (by probing) between 1'000'000 and 10'000'000. On the un-modified verison, this take houres on the odroid. So to speed-up the testing and development, it was first chosen to make this computation between 1 and 320'000. The initial range, will be tested only on the final alogrithm.

No modified version
------------------- 

This is just the program as given, but modified for the range from 1 to 320'000. When run, this program give the following output:

.. code-block:: console

	# /usr/workspace/prime_opt > prime_list.txt
	# tail prime_list.txt
	p: 319901
	p: 319919
	p: 319927
	p: 319931
	p: 319937
	p: 319967
	p: 319973
	p: 319981
	p: 319993
	25579.335 ms
	#
	# wc -l prime_list.txt
	27611 prime_list.txt

This version takes 25.579 seconds to run. It has found 27610 primve number between 1 and 320'000. (last line is taken by execution time print).


Modified to write to file
-------------------------

This version simply writes to a file instead of the the STDOUT. It is just about opening the file in the begin of the ``main()`` function and colsing it at the end of it, like this:

.. code-block:: c

	int main()
	{
		...

	 
	    FILE* f = fopen("prime_num.txt", "w");
	    ...
	    fclose(f);
	    return 0;
	}

Then in the checkNumbers() function, we remplace the printf() call by fprintf(). The file descriptor is added to parameter list:

.. code-block:: c

	int checkNumbers(long long min, long long max, FILE* fd)
	{
		...
		if (prime==1) 
	    {
	        prime_count++;
	        fprintf(fd, "%d\n", num); 
	    }
	...
	}

When run, this program give the following output:

.. code-block:: console

	# /usr/workspace/prime_opt 
	There is 8 cores
	25270.439 ms
	# 
	# wc -l prime_num.txt 
	27610 prime_num.txt


This version takes 25.27 seconds to run, this is gain of xx% over the unmodified version. It has found 27610 primve number between 1 and 320'000.

Modified to remplace fprintf() by itoa()
----------------------------------------

This version removes the ``fprintf()`` call by ``itoa()`` as it is more efficient. The ``printf()`` call is remplaced by the following code:

.. code-block:: c
    itoa(num, buffer);
    int len = strlen(buffer);
    buffer[len] = '\n';
    buffer[len+1] = '\0';
    fwrite(buffer, 1,  len+1, fd);

When run, this program give the following output:

.. code-block:: console

	# /usr/workspace/prime_opt 
	There is 8 cores
	85865.354 ms
	# 
	# wc -l prime_num.txt 
	27610 prime_num.txt


This version takes 27.61 seconds to run, this is gain of xx% over the unmodified version. It has found 27610 primve number between 1 and 320'000.

Threaded Version
----------------

This version uses thread in order to use the whole computation power of the system. It creates as many threads as they are cores on the system. On the Odroid, there is 8 cores, so 8 threads are created (this number changes dynamicaly). The work is then splited between the theads, each thread having a slice of the range to compute. At the end, all threads are joined.

A run of this version gives:

.. code-block:: console

	# /usr/workspace/prime_opt 
	There is 8 cores
	This is thread 0 computing from 1 to 39999
	This is thread 1 computing from 40000 to 79998
	This is thread 3 computing from 119998 to 159996
	This is thread 4 computing from 159997 to 199995
	This is thread 2 computing from 79999 to 119997
	This is thread 5 computing from 199996 to 239994
	This is thread 7 computing from 279994 to 320000
	This is thread 6 computing from 239995 to 279993
	26044.550 ms
	# 
	# wc -l prime_num.txt 
	27610 prime_num.txt


This version takes 26.044 seconds to run, this is gain of xx% over the unmodified version.

Changed Prime calculation algorithm
-----------------------------------

This version have a new optimized alogrithm to check if a number is a prime number or not. This alogrithm was found at https://en.wikipedia.org/wiki/Primality_test#Pseudocode and transposed to C. The code is then the following:

..code-block:: c

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

When run, this program give the following output:

..code-block:: console

	# /usr/workspace/prime_opt 
	There is 8 cores
	This is thread 0 computing from 1 to 40000
	This is thread 1 computing from 40001 to 80000
	This is thread 3 computing from 120001 to 160000
	This is thread 2 computing from 80001 to 120000
	This is thread 4 computing from 160001 to 200000
	This is thread 5 computing from 200001 to 240000
	This is thread 6 computing from 240001 to 280000
	This is thread 7 computing from 280001 to 320000
	There is 27607 prime number between 1 and 320000.
	23.866 ms


This version takes 0.023 seconds to run, this is gain of xx% over the unmodified version.


Summary
-------

The following table give an overview of the different optimization and the gain they brings:

+---------------------------------+----------------+-------+
| Version                         | Execution time | Gain  |
+=================================+================+=======+
| No modified                     | 25.579         | 0%    |
+---------------------------------+----------------+-------+
| Write to file                   | 25.270         | 1.2%  |
+---------------------------------+----------------+-------+
| Remplaced fprintf() by itoa()   | 27.610         | -7.9% |
+---------------------------------+----------------+-------+
| Threaded Version                | 26.044         | -1.8% |
+---------------------------------+----------------+-------+
| New Prime calculation algorithm | 0.0023         | 99.9% | 
+---------------------------------+----------------+-------+


Test on final algorithm
-----------------------

This is the program run with the initial range:

..code-block:: console

	# /usr/workspace/prime_opt 
	There is 8 cores
	This is thread 0 computing from 1000000 to 2124999
	This is thread 1 computing from 2125000 to 3249999
	This is thread 2 computing from 3250000 to 4374999
	This is thread 4 computing from 5500000 to 6624999
	This is thread 5 computing from 6625000 to 7749999
	This is thread 6 computing from 7750000 to 8874999
	This is thread 7 computing from 8875000 to 10000000
	This is thread 3 computing from 4375000 to 5499999
	There is 586081 prime number between 1000000 and 10000000.
	1963.207 ms



On the initial range, the optimized version takes 1.963 seconds to run!