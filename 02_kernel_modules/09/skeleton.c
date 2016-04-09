#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>

static unsigned int irq;

irqreturn_t short_interrupt()
{
    printk("Hello SW1!\n");
    return IRQ_HANDLED;
}

static int __init skeleton_init(void)
{   
    pr_info ("Linux module skeleton loaded\n");
    int ret;

    gpio_request(29,NULL);
    gpio_direction_input(29);
    irq = gpio_to_irq(29);

    if (irq >= 0){
        ret = request_irq(irq,short_interrupt,IRQF_TRIGGER_FALLING,"sw1",NULL);
        if (ret){
            printk(KERN_INFO "SW1: can't get assigned irq %i\n", irq);
            irq = -1;
        }
    }
    return 0;
}

static void __exit skeleton_exit(void)
{
    pr_info ("Linux module skeleton unloaded\n");
    if (irq != -1){
        free_irq(irq, NULL);
    }
}

module_init (skeleton_init);
module_exit (skeleton_exit);
MODULE_AUTHOR ("Yann Maret");
MODULE_DESCRIPTION ("Module skeleton");
MODULE_LICENSE ("GPL");

