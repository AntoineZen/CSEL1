

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
    
    // Open the button device
    button_fd = open("/dev/button", O_RDONLY);

    // Check that the file was oppend
    if (button_fd > 0)
    {
        // Add it to the file desciptor set
        FD_SET(button_fd, &rfds);

        while(counter < 100)
        {
            // Copy the fd_set as it will be modifed by select()
            active_fds = rfds;
            // Wait for the read event to come
            retval = select(button_fd+1, &active_fds, NULL, NULL, NULL);

            // Handle error!
            if (retval == -1)
            {
                perror("select()");
            }
            // Normal case (no error)
            else if (retval)
            {
                counter++;
                printf("Counter is now %d.\n", counter);
            }
            // Handle timeout (should not append!)
            else
            {
                // This will never be reached, but keep for debug.
                printf("Timeout\n");
            }
        }
        // Close the periferal
        close(button_fd);
    }
    else
    {
        perror("Error opening '/dev/button'\n");
    }

    return 0;
}
