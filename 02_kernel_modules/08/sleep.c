#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/delay.h>


// Wait queue for the event
DECLARE_WAIT_QUEUE_HEAD(event_queue);
bool wakeup = false;


// Handle for the thread one
struct task_struct* thread_one_handle = NULL;

/**
 * Thread one implementation (event consumer)
 */
int thread_one(void* data)
{

	pr_info("Thread one started\n");
	
	// Thread loop
	while(!kthread_should_stop())
	{
		// Wait for an event
		wait_event(event_queue, wakeup);
		wakeup = false;
		
		pr_info("Thread one: Got event\n");
	
	}
	
	pr_info("Thread one exiting\n");
	return 0;
}


// Handle for the thread two
struct task_struct* thread_two_handle = NULL;

/**
 * Thread two implementation (event producer)
 */
int thread_two(void* data)
{

	pr_info("Thread two started\n");
	
	// Thread loop
	while(!kthread_should_stop())
	{
		// Debug message
		pr_info("Thread two: Sending event\n");
		
		// Place an event for thread 1
		wakeup = true;
		wake_up(&event_queue);
		
		// Sleep for 5 seconds
		ssleep(5);
	}
	
	pr_info("Thread two exiting\n");
	return 0;
}



static int __init skeleton_init(void  )
{
	// Create the wait queue
	init_waitqueue_head(&event_queue);   
	wakeup = false;
    
    
    // Create & Start both threads
    thread_one_handle = kthread_run(thread_one, NULL, "Thread one");
    thread_two_handle = kthread_run(thread_two, NULL, "Thread two");
    
    pr_info("Linux module sleep loaded\n");
    
    return 0;
}


static void __exit skeleton_exit(void  )
{
	// Stop thread one
	if( thread_one_handle != NULL )
	{
		kthread_stop(thread_one_handle);
	}
	
	// Stop thread two
	if( thread_two_handle != NULL )
	{
		kthread_stop(thread_two_handle);
	}
	
    pr_info("Linux module sleep unloaded\n");
}


module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Antoine Zen-Ruffinen <antoine.zen@gmail.com>");
MODULE_DESCRIPTION("Sleep test Module");
MODULE_LICENSE("GPL");
