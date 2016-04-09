/*
skeleton.c
*/
#include <linux/module.h>/* needed by all modules */
#include <linux/init.h>/* needed for macros */
#include <linux/kernel.h> /* needed for debugging */
#include <linux/kthread.h> /* for use threads */
#include <linux/delay.h> /* for use ssleep */

struct task_struct* k;

int thread(void* data)
{
    while(!kthread_should_stop()){
        ssleep(5);
        pr_info("Hello from thread !\n");
    }
    return 0;
}

static int __init skeleton_init(void)
{
    pr_info("Linux module skeleton loaded\n");
    //Run thread
    k=kthread_run(thread, NULL, "my_thread");
    return 0;
}

static void __exit skeleton_exit(void)
{
    kthread_stop(k);
    pr_info("Linux module skeleton unloaded \n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);
MODULE_AUTHOR("Yann Maret");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");


