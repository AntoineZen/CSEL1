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
	
    pr_info("Module iotest loaded\n");
    request_mem_region(0x10000000, 0x100, "iotest");
    id_ptr = ioremap(0x10000000, 0x100);
    
    if(id_ptr != NULL)
    {
    	id = readl(id_ptr);
    	
    	minor_v 	= (id & 0x0000000F);
    	major_v 	= (id & 0x000000F0) >> 4;
    	package_id 	= (id & 0x00000F00) >> 8;
    	product_id  = (id & 0xFFFFF000) >> 12;
    	
    	pr_info("Product=%d, package=%d, major=%d, minor=%d\n", product_id, package_id, major_v, minor_v);
    	
    
    }
    
    return 0;
}

static void __exit skeleton_exit(void  )
{
	iounmap(id_ptr);
	release_mem_region(0x10000000, 0x100);
    pr_info("Module iotest\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Antoine Zen-Ruffinen <antoine.zen@gmail.com>");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");
