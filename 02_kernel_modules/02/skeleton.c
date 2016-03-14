#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

static char* text = "Some blabla";
module_param(text, charp, 0);

static int some_val = 0;
module_param(some_val, int, 0);

static int __init skeleton_init(void  )
{
    pr_info("Linux module skeleton loaded\n");
    pr_info("mymodule: some_val=%d, text=%s\n", some_val, text);
    return 0;
}

static void __exit skeleton_exit(void  )
{
    pr_info("Linux module skeletonunloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR("Antoine Zen-Ruffinen <antoine.zen@gmail.com>");
MODULE_DESCRIPTION("Module skeleton");
MODULE_LICENSE("GPL");
