

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
       
// Include for the mmap() function
#include <sys/mman.h>

int main ()
{
        // File descriptor for the /dev/mem file
        int mem_fd;
        
        // Pointer to the ID register (will be set by mmap())
        uint32_t* id;
        
        // Fileds of the ID 	
        int product_id;
        int package_id;
        int major_v;
        int minor_v;
        
        // Open the memory device
        mem_fd = open("/dev/mmaptest", O_RDONLY);
        
        if (mem_fd > 0)
        {
                // Map the ID register to our virtual address space
                id = mmap(NULL, 0x100,  PROT_READ, MAP_PRIVATE, mem_fd, 0x10000000);
        
                // Extract the various bit-fields
        minor_v 	= (*id & 0x0000000F);
        major_v 	= (*id & 0x000000F0) >> 4;
        package_id 	= (*id & 0x00000F00) >> 8;
        product_id  = (*id & 0xFFFFF000) >> 12;
        
        // Print the informations readen
        printf(
                "Product=%d, package=%d, major=%d, minor=%d\n", 
                product_id, 
                package_id, 
                major_v, 
                minor_v
                );
        
                // Unmap the ID register
                munmap(id, 0x100);
                // Close /dev/mem
                close(mem_fd);
        }
        else
        {
                printf("Error oppening '/dev/mmaptest'\n");
        }
        

        return 0;
}
