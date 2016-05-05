Optimizations
=============

In this lab, we get the source code of a program that compute the list of all prime numbers in a given range (start and stop number). This program is sub-obtimal, the goal it to apply the techniques learned in the lecture to optimize the program. For each optimization, we give the description of the change the execution time of the program and the gain in percent.

No modified version
------------------- 

This is just the program as given. When run, the program takes about ... 

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

This version takes xx seconds to run, this is gain of xx% over the unmodified version.

Modified to remplace fprintf() by itoa()
----------------------------------------

This version takes xx seconds to run, this is gain of xx% over the unmodified version.

Threaded Version
----------------


This version takes xx seconds to run, this is gain of xx% over the unmodified version.

Changed Prime calculation algorithm
-----------------------------------

This version takes xx seconds to run, this is gain of xx% over the unmodified version.

Summary
-------

The following table give an overview of the different optimization and the gain they brings:

+---------------------------------+----------------+------+
| Version                         | Execution time | Gain |
+=================================+================+======+
| No modified                     |                | 0%   |
+---------------------------------+----------------+------+
| Write to file                   |                |      |
+---------------------------------+----------------+------+
| Remplaced fprintf() by itoa()   |                |      |
+---------------------------------+----------------+------+
| Threaded Version                |                |      |
+---------------------------------+----------------+------+
| New Prime calculation algorithm |                |      |
+---------------------------------+----------------+------+
