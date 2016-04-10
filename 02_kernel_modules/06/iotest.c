#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/io.h>
#include <linux/ioport.h>

static int* id_ptr;

static int __init skeleton_init(void  )
{
	int id;
	int product_id;
	int package_id;
	int major_v;
	int minor_v;
	
	// Print that module is being loaded
    pr_info("Module iotest loaded\n");
    
    // Request the memory region
    request_mem_region(0x10000000, 0x100, "iotest");
    
    // Map it to our local pointer (used to read an later to release
    id_ptr = ioremap(0x10000000, 0x100);
    
    // Check that the memory regison was sucessfully mapped
    if(id_ptr != NULL)
    {
    	// Read the ID word
    	id = readl(id_ptr);
    	
    	// Extract the various bit-fields
    	minor_v 	= (id & 0x0000000F);
    	major_v 	= (id & 0x000000F0) >> 4;
    	package_id 	= (id & 0x00000F00) >> 8;
    	product_id  = (id & 0xFFFFF000) >> 12;
    	
    	// Print the informations readen
    	pr_info(
            "Product=%d, package=%d, major=%d, minor=%d\n", 
            product_id, 
            package_id, 
            major_v, 
            minor_v
            );
    }
    
    return 0;
}

static void __exit skeleton_exit(void  )
{
	// Unmap using the recieved pointer
	iounmap(id_ptr);
	
	// Release the IO memory region that was requested.
	release_mem_region(0x10000000, 0x100);
	   
	// PRint that everthing is done
    pr_info("Module iotest removed\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Antoine Zen-Ruffinen <antoine.zen@gmail.com>");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");
