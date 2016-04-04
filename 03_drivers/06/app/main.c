

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <fcntl.h>
       

int main ()
{
	int counter = 0;
	int retval;
	//struct timeval tv;
	// File descriptor for the /dev/button file
	int button_fd;
	// create & init file descriptor set for the select() function
	fd_set active_fds, rfds;
	FD_ZERO(&rfds); 
	
	// Setup a five second time-out, maybe for debug
	/*
    tv.tv_sec = 5;
    tv.tv_usec = 0;
	*/
	
	// Open the memory device
	button_fd = open("/dev/button", O_RDONLY);
	// Add it to the file desciptor set
	FD_SET(button_fd, &rfds);
	
	if (button_fd > 0)
	{
		while(counter < 100)
		{
			active_fds = rfds;
			retval = select(1, &active_fds, NULL, NULL, NULL);
			if (retval == -1)
			{
	        	perror("select()");
        	}
		    else if (retval)
		    {
		    	counter++;
		    	printf("Counter is now %d.\n", counter);
		    }
		    else
		    {
		    	// This will never be reached, but keep for debug.
		    	printf("Timeout\n");
	    	}
	    }

		close(button_fd);
	}
	else
	{
		printf("Error opening '/dev/button'\n");
	}

	return 0;
}
