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
#include <linux/thermal.h>
#include <linux/pwm.h>
#include <linux/kthread.h> /* for use threads */
#include <linux/delay.h> /* for use ssleep */


#define BUFFER_SIZE 200
#define PERIOD_NS 20972

#define DRIVER_NAME "fan-ctrl"
#define MODE_AUTO "auto\n"
#define MODE_MAN "manual\n"

#define MAX(x, y) ((x>y)?x:y)       

char text[BUFFER_SIZE];

//storage structure accessed by sysfs
struct fan_ctrl_t{
    char mode[100];
    int duty;
    int temp;
    struct pwm_device* pwm;
};

static struct fan_ctrl_t fan_ctrl_var = {
    .mode=MODE_AUTO,
    .duty =  20,
};



// sysfs
static void sysfs_dev_release(struct device * dev) {}

static struct platform_driver sysfs_driver= {
    .driver = {.name = DRIVER_NAME,},
};

static struct platform_device sysfs_device = {
    .name= DRIVER_NAME,
    .id = -1,
    .dev.release = sysfs_dev_release
};



static ssize_t fan_ctrl_show_mode(
    struct device* dev, 
    struct device_attribute * attr, 
    char * buf
    )
{
    strcpy(buf, fan_ctrl_var.mode);
    return strlen(fan_ctrl_var.mode);
}

static ssize_t fan_ctrl_store_mode(
    struct device * dev, 
    struct device_attribute * attr, 
    const char * buf, size_t count
    )
{
    int len = sizeof(fan_ctrl_var.mode) - 1;
    if (len > count)
        len = count;
    strncpy(fan_ctrl_var.mode, buf, len);
    fan_ctrl_var.mode[len] = 0;
    pr_info("Mode changed to %s", fan_ctrl_var.mode);
    return len;
}


static ssize_t fan_ctrl_show_duty(
    struct device* dev, 
    struct device_attribute * attr, 
    char * buf
    )
{
    return sprintf(buf, "%u\n", fan_ctrl_var.duty);
}

static ssize_t fan_ctrl_store_duty(
    struct device * dev, 
    struct device_attribute * attr, 
    const char * buf, 
    size_t count
    )
{
    //buf must be converted to int
    long duty;
    if(!kstrtol (buf, 10, &duty))
    {
        fan_ctrl_var.duty= (int)duty;
    }

    // Avoid overflow
    if( fan_ctrl_var.duty > 100 )
        fan_ctrl_var.duty = 100;

    // Avoid undeflow
    if( fan_ctrl_var.duty < 0)
        fan_ctrl_var.duty = 0;
    pr_info("Duty changed to %d\n", fan_ctrl_var.duty);
    return strlen(buf);
}

static ssize_t fan_ctrl_show_temp(
    struct device* dev, 
    struct device_attribute * attr, 
    char * buf
    )
{
    return sprintf(buf, "%d\n", fan_ctrl_var.temp);
}


//declare devices attributes structures
DEVICE_ATTR(mode, 0660, fan_ctrl_show_mode, fan_ctrl_store_mode);
DEVICE_ATTR(duty, 0660, fan_ctrl_show_duty, fan_ctrl_store_duty);
DEVICE_ATTR(temp, 0440, fan_ctrl_show_temp, NULL);

struct task_struct* fan_ctrl_task;

static const char th_zones[][15] = 
{
    "cpu0-thermal",
    "cpu1-thermal",
    "cpu2-thermal",
    "cpu3-thermal",
    "gpu-thermal",
};


int fan_ctrl_thread(void* data)
{
    int i;
    int temp_work;
    int ret;
    int duty_ns;

    // get the passed parameters
    struct fan_ctrl_t* state = (struct fan_ctrl_t*)data;

    // Enable PWM
    ret = pwm_enable(state->pwm);
    if(ret)
    {
        pr_err("Error enabling PWM output\n");
    }

    // Initial seting of pwm
    duty_ns = (PERIOD_NS * state->duty) / 100;
    ret = pwm_config(state->pwm, duty_ns, PERIOD_NS);
    if(ret)
    {
        pr_err("Error setting PWM output\n");
    }

    while(!kthread_should_stop())
    {
        // get the maxiumum temperature of the CPU
        state->temp = -50000;
        for(i = 0; i< sizeof(th_zones); i++)
        {
            thermal_zone_get_temp(thermal_zone_get_zone_by_name(th_zones[i]), &temp_work);
            state->temp = MAX(state->temp, temp_work);
        }

        // Do we are in auto mode ?
        if(strcmp(state->mode, MODE_AUTO) == 0)
        { 
            // Change the duty accoring the temperature
            if( state->temp < 57000 )
                state->duty = 0;
            else if( state->temp < 63000 )
                state->duty = 20;
            else if( state->temp < 68000 )
                state->duty = 50;
            else 
                state->duty = 100;
        }

        // Update PWM output
        duty_ns = (PERIOD_NS * state->duty) / 100;
        ret = pwm_config(state->pwm, duty_ns, PERIOD_NS);
        if(ret)
        {
            pr_err("Error setting PWM output\n");
        }

        // Sleep for half a second
        msleep(500);
    }

    pwm_disable(state->pwm);
    return 0;
}


static int __init skeleton_init(void)
{
    int status = 0;

    //sysfs register driver+device and add devices attributes
    if (status == 0)
        status = platform_driver_register(&sysfs_driver);
    if (status == 0)
        status = platform_device_register(&sysfs_device);
    if (status == 0)
    {
        device_create_file(&sysfs_device.dev, &dev_attr_mode);
        device_create_file(&sysfs_device.dev, &dev_attr_duty);
        device_create_file(&sysfs_device.dev, &dev_attr_temp);
    }

    fan_ctrl_var.pwm = pwm_request(0, DRIVER_NAME);

    fan_ctrl_task = kthread_run(fan_ctrl_thread, &fan_ctrl_var, "fan-ctrl thread");
    if(fan_ctrl_task == NULL)
    {
        pr_err("Unable to start fan-ctrl thread.\n");
        return 1;
    }
    if (status == 0)
        pr_info ("fan-ctrl device driver loaded\n");
    return status;
}
static void __exit skeleton_exit(void)
{
    kthread_stop(fan_ctrl_task);

    pwm_free(fan_ctrl_var.pwm);

    //sysfs remove devices attributes and unregister device+driver
    device_remove_file(&sysfs_device.dev, &dev_attr_mode);
    device_remove_file(&sysfs_device.dev, &dev_attr_duty);
    device_remove_file(&sysfs_device.dev, &dev_attr_temp);

    platform_device_unregister(&sysfs_device);
    platform_driver_unregister(&sysfs_driver);

    pr_info("Linux module fan-ctrl unloaded \n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);
MODULE_AUTHOR("Antoine Zen-Ruffinena");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");

