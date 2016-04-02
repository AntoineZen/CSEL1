#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/mm.h>


#define REG_ADDR 0x10000000
#define REG_SIZE 0x100

static dev_t mmaptest_dev;
static struct cdev mmaptest_cdev;



int mmaptest_open(struct inode* i, struct file* f)
{
	return 0;
}


int mmaptest_release(struct inode* i, struct file* f)
{
	return 0;
}


int mmaptest_mmap(struct file* f, struct vm_area_struct* vma)
{

	pr_info("Module mmaptest: mmap handler called.\n");
	return remap_pfn_range(vma, vma->vm_start, REG_ADDR >> PAGE_SHIFT, REG_SIZE,  vma->vm_page_prot);
}


static struct file_operations mmaptest_fops = {
	.owner = THIS_MODULE,
	.open = mmaptest_open,
	.mmap = mmaptest_mmap,
	.release = mmaptest_release,
};

static int __init mmaptest_init(void  )
{

	// 1) Register for a Major device number
	int status = alloc_chrdev_region(&mmaptest_dev, 0, 1, "mmaptest");
	// If it worked
	if (status == 0)
	{
		// 2) Register the strucutre of call-back function pointers
		cdev_init(&mmaptest_cdev, &mmaptest_fops);
		mmaptest_cdev.owner = THIS_MODULE;
		// 3) Registter the caracter device
		status = cdev_add(&mmaptest_cdev, mmaptest_dev, 1);
		
	}
	else
	{
		pr_err("Failed to alocate character device");
	}
	

	// Print that module is being loaded
    pr_info("Module mmaptest loaded\n");
    
    return 0;
}

static void __exit mmaptest_exit(void  )
{
	cdev_del(&mmaptest_cdev);
	unregister_chrdev_region(mmaptest_dev, 1);
	
	// Print that everthing is done
    pr_info("Module mmaptest removed\n");
}

module_init(mmaptest_init);
module_exit(mmaptest_exit);

MODULE_AUTHOR("Antoine Zen-Ruffinen <antoine.zen@gmail.com>");
MODULE_DESCRIPTION("mmap test module");
MODULE_LICENSE("GPL");
