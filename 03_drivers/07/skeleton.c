/*
skeleton.c
*/
#include <linux/module.h>
/* needed by all modules */
#include <linux/init.h>
/* needed for macros */
#include <linux/kernel.h>
/* needed for debugging */
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/platform_device.h>
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

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static dev_t skeleton_dev;
static struct cdev skeleton_cdev;

// sysfs
static void sysfs_dev_release(struct device * dev) {}

static struct platform_driver sysfs_driver= {
    .driver = {.name = "skeleton",},
};

static struct platform_device sysfs_device = {
    .name= "skeleton",
    .id = -1,
    .dev.release = sysfs_dev_release
};

struct file_operations skeleton_fops =
{
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static ssize_t skeleton_show_marque(struct device* dev, struct device_attribute * attr, char * buf)
{
    strcpy(buf, voiture.marque);
    return strlen(voiture.marque);
}

static ssize_t skeleton_store_marque(struct device * dev, struct device_attribute * attr, const char * buf, size_t count)
{
    int len = sizeof(voiture.marque) - 1;
    if (len > count)
        len = count;
    strncpy(voiture.marque, buf, len);
    voiture.marque[len] = 0;
    return len;
}

static ssize_t skeleton_show_modele(struct device* dev, struct device_attribute * attr, char * buf)
{
    //modele must be converted to cstring
    strcpy(buf, voiture.modele);
    return strlen(voiture.modele);
}

static ssize_t skeleton_store_modele(struct device * dev, struct device_attribute * attr, const char * buf, size_t count)
{
    int len = sizeof(voiture.modele) - 1;
    if (len > count)
        len = count;
    strncpy(voiture.modele, buf, len);
    voiture.modele[len] = 0;
    return len;
}

static ssize_t skeleton_show_vitesse(struct device* dev, struct device_attribute * attr, char * buf)
{
    return sprintf(buf, "%u\n",voiture.vitesse_max);
}

static ssize_t skeleton_store_vitesse(struct device * dev, struct device_attribute * attr, const char * buf, size_t count)
{
    //buf must be converted to int
    long vitesse;
    if(!kstrtol (buf, 10, &vitesse))
        voiture.vitesse_max = (int)vitesse;
    return strlen(buf);
}

//declare devices attributes structures
DEVICE_ATTR(marque, 0660, skeleton_show_marque, skeleton_store_marque);
DEVICE_ATTR(modele, 0660, skeleton_show_modele, skeleton_store_modele);
DEVICE_ATTR(vitesse, 0660, skeleton_show_vitesse, skeleton_store_vitesse);

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t device_read(struct file *filp, char *buff, size_t len, loff_t *off)
{
    if(len>BUFFER_SIZE)
        len=BUFFER_SIZE;
    if(len>strlen(text))
        len=strlen(text);
    copy_to_user(buff, text, strlen(text));
    text[0]=0;
    return len;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    pr_info ("Device write\n");
    if(len>BUFFER_SIZE)
        len=BUFFER_SIZE;
    copy_from_user(text, buff, len);
    return len;
}

static int __init skeleton_init(void)
{
    int status = alloc_chrdev_region (&skeleton_dev, 0, 1, "skeleton");
    if(status == 0){
        cdev_init (&skeleton_cdev, &skeleton_fops);
        skeleton_cdev.owner = THIS_MODULE;
        status = cdev_add (&skeleton_cdev, skeleton_dev, 1);
    }
    if (status == 0) 
        pr_info ("Skeleton device driver loaded\n");
    //sysfs register driver+device and add devices attributes
    if (status == 0)
        status = platform_driver_register(&sysfs_driver);
    if (status == 0)
        status = platform_device_register(&sysfs_device);
    if (status == 0)
        device_create_file(&sysfs_device.dev, &dev_attr_marque);
    
    device_create_file(&sysfs_device.dev, &dev_attr_modele);
    device_create_file(&sysfs_device.dev, &dev_attr_vitesse);
    return status;
}
static void __exit skeleton_exit(void)
{
    //sysfs remove devices attributes and unregister device+driver
    device_remove_file(&sysfs_device.dev, & dev_attr_marque);
    device_remove_file(&sysfs_device.dev, & dev_attr_modele);
    device_create_file(&sysfs_device.dev, &dev_attr_vitesse);
    platform_device_unregister(&sysfs_device);
    platform_driver_unregister(&sysfs_driver);
    cdev_del (&skeleton_cdev);
    unregister_chrdev_region(skeleton_dev, 1);
    pr_info("Linux module skeleton unloaded \n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);
MODULE_AUTHOR("Yann Maret");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");

