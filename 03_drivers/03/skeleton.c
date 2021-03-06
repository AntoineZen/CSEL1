/*
skeleton.c
*/
#include <linux/module.h> /* needed by all modules */
#include <linux/init.h> /* needed for macros */
#include <linux/kernel.h> /* needed for debugging */
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/string.h>

#define BUFFER_SIZE 200

char text[BUFFER_SIZE];

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static dev_t skeleton_dev;
static struct cdev skeleton_cdev;

struct file_operations skeleton_fops =
{
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static int num_instance=1;
module_param(num_instance,int,0);

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t device_read(
    struct file *filp,
    char *buff, 
    size_t len, 
    loff_t *off
    )
{
    if(len>BUFFER_SIZE)
        len=BUFFER_SIZE;
    if(len>strlen(text))
        len=strlen(text);
    
    copy_to_user(buff, text, strlen(text));
    text[0]=0;
    
    return len;
}

static ssize_t device_write(
    struct file *filp, 
    const char *buff, 
    size_t len, 
    loff_t *off
    )
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
    return status;
}

static void __exit skeleton_exit(void)
{
    cdev_del (&skeleton_cdev);
    unregister_chrdev_region(skeleton_dev, 1);
    pr_info("Linux module skeleton unloaded \n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);
MODULE_AUTHOR("Yann Maret");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");


