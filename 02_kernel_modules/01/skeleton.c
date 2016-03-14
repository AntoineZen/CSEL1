/* skeleton.c*/
#include <linux/module.h>
/* needed by all modules */
#include <linux/init.h>
/* needed for macros */
#include <linux/kernel.h>
/* needed for debugging */
static int __init skeleton_init(void  )
{
    pr_info("Linux module skeleton loaded\n");
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
