#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/interrupt.h> 
#include <linux/gpio.h>
#include <linux/poll.h>
#include <linux/wait.h>


#define BUTTON_IO_NR 29
// Device structure
static dev_t button_dev;
// Character device structure
static struct cdev button_cdev;


#define BUFFER_SIZE 200

char text[BUFFER_SIZE];

//storage structure accessed by sysfs
struct struct_voiture{
    char marque[100];
    char modele[100];
    int vitesse_max;
};

static struct struct_voiture voiture = {
    .marque="Subaru\n",
    .modele="Impreza\n",
    .vitesse_max = 300
};

// sysfs
static void sysfs_dev_release(struct device * dev) {}

static struct platform_driver sysfs_driver= {
    .driver = {.name = "button",},
};

static struct platform_device sysfs_device = {
    .name= "button",
    .id = -1,
    .dev.release = sysfs_dev_release
};


static ssize_t button_show_marque(
    struct device* dev, 
    struct device_attribute * attr, 
    char * buf
    )
{
    strcpy(buf, voiture.marque);
    return strlen(voiture.marque);
}

static ssize_t button_store_marque(
    struct device * dev, 
    struct device_attribute * attr, 
    const char * buf, 
    size_t count
    )
{
    int len = sizeof(voiture.marque) - 1;
    if (len > count)
        len = count;
    strncpy(voiture.marque, buf, len);
    voiture.marque[len] = 0;
    return len;
}

static ssize_t button_show_modele(
    struct device* dev, 
    struct device_attribute * attr, 
    char * buf
    )
{
    //modele must be converted to cstring
    strcpy(buf, voiture.modele);
    return strlen(voiture.modele);
}

static ssize_t button_store_modele(
    struct device * dev, 
    struct device_attribute * attr, 
    const char * buf, 
    size_t count
    )
{
    int len = sizeof(voiture.modele) - 1;
    if (len > count)
        len = count;
    strncpy(voiture.modele, buf, len);
    voiture.modele[len] = 0;
    return len;
}

static ssize_t button_show_vitesse(
    struct device* dev, 
    struct device_attribute * attr, 
    char * buf
    )
{
    return sprintf(buf, "%u\n",voiture.vitesse_max);
}

static ssize_t button_store_vitesse(
    struct device * dev, 
    struct device_attribute * attr, 
    const char * buf, s
    ize_t count
    )
{
    //buf must be converted to int
    long vitesse;
    if(!kstrtol (buf, 10, &vitesse))
        voiture.vitesse_max = (int)vitesse;
    return strlen(buf);
}

//declare devices attributes structures
DEVICE_ATTR(marque, 0660, button_show_marque, button_store_marque);
DEVICE_ATTR(modele, 0660, button_show_modele, button_store_modele);
DEVICE_ATTR(vitesse, 0660, button_show_vitesse, button_store_vitesse);


// Wait queue for the Button IRQ
DECLARE_WAIT_QUEUE_HEAD(button_event_queue);
bool toggled = false;

int button_value =0;



static int button_open(struct inode* i, struct file* f)
{
    return 0;
}

static int button_release(struct inode* i, struct file* f)
{
    return 0;
}

static unsigned int button_poll(
    struct file* f, 
    struct poll_table_struct* wait
    )
{
    // Wait to have an event in the queue
    pr_info("Before pool_wait()\n");
    poll_wait(f, &button_event_queue, wait);
    pr_info("after pool_wait()\n");
    if (toggled)
    {
        pr_info("pool_wait() return true\n");
        // Note that the event was consumed
        toggled = false;
        // Return that the user-land appliation can read
        return POLLIN | POLLRDNORM;
    }
    return 0;
}

static ssize_t button_read(
    struct file* f, 
    char* data, 
    size_t size, 
    loff_t* offset
    )
{
    char* val;
    
    // Convert to string
    if (button_value)
    {
        val = "1\0";
    }
    else
    {
        val = "0\0";
    }
    
    // Send to user-land application
    if(copy_to_user(data, val, 2) != 0) 
    {
        return -EFAULT;
    }       
    return 2;
}

static ssize_t button_write(
    struct file* f, 
    const char* data, 
    size_t size, 
    loff_t* offset
    )
{
    return size;
}

static irqreturn_t button_irq(int irq, void *dev_id)
{

    if(button_value != gpio_get_value(BUTTON_IO_NR))
    {
    
        // For debug (not good becaus of perf)
        pr_info("button_irq()\n");
        // Store button value
        button_value = gpio_get_value(BUTTON_IO_NR);
    
        // Note that is was toogle since last read
        toggled = true;
    
        // Wakeup the tasks that are using pool()
        wake_up_interruptible(&button_event_queue);
    }
    
    // Tell the kernel that we have handled the interrupt event
    return IRQ_HANDLED;
}

static const struct file_operations button_fops = {
    .owner = THIS_MODULE,
    .read = button_read,
    .write = button_write,
    .poll = button_poll,
    .open = button_open,
    .release = button_release,
};


static int __init button_init(void  )
{

    // 1) Register for a Major device number
    int status = alloc_chrdev_region(&button_dev, 0, 1, "button");
    // If it worked
    if (status == 0)
    {
        // 2) Register the strucutre of call-back function pointers
        cdev_init(&button_cdev, &button_fops);
        button_cdev.owner = THIS_MODULE;
        // 3) Registter the caracter device
        status = cdev_add(&button_cdev, button_dev, 1);
        
    }
    else
    {
        pr_err("Failed to allocate character device");
    }
    
    
    // Create the wait queue
    init_waitqueue_head(&button_event_queue);   
    toggled = false;
    
    //register the IRQ
    gpio_request(BUTTON_IO_NR, "button");
    status = request_irq(
        gpio_to_irq(BUTTON_IO_NR), 
        button_irq, 
        0, 
        "button", 
        NULL
        );
    if (status != 0)
    {
        pr_err("Failed to map button IRQ");
    }

        //sysfs register driver+device and add devices attributes
    if (status == 0)
        status = platform_driver_register(&sysfs_driver);
    if (status == 0)
        status = platform_device_register(&sysfs_device);
    if (status == 0)
    {
        device_create_file(&sysfs_device.dev, &dev_attr_marque);
        device_create_file(&sysfs_device.dev, &dev_attr_modele);
        device_create_file(&sysfs_device.dev, &dev_attr_vitesse);
    }

    // Print that module is being loaded
    pr_info("Module button loaded\n");
    
    return 0;
}


static void __exit button_exit(void  )
{
    //sysfs remove devices attributes and unregister device+driver
    device_remove_file(&sysfs_device.dev, & dev_attr_marque);
    device_remove_file(&sysfs_device.dev, & dev_attr_modele);
    device_create_file(&sysfs_device.dev, &dev_attr_vitesse);
    platform_device_unregister(&sysfs_device);
    platform_driver_unregister(&sysfs_driver);

    // Free the IRQ
    free_irq(gpio_to_irq(BUTTON_IO_NR), NULL);
    // Free the GPIO
    gpio_free(BUTTON_IO_NR);
    // Delete the device driver
    cdev_del(&button_cdev);
    unregister_chrdev_region(button_dev, 1);
    
    // Print that everthing is done
    pr_info("Module button removed\n");
}

module_init(button_init);
module_exit(button_exit);

MODULE_AUTHOR("Antoine Zen-Ruffinen <antoine.zen@gmail.com>");
MODULE_DESCRIPTION("Button driver module");
MODULE_LICENSE("GPL");
