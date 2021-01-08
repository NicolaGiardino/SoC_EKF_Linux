#include "include/procedure_kernel.h"

/*
** Module Init function
*/
static int __init SoC_init(void)
{
   
    etx_thread = kthread_create(thread_function, NULL, "SoC Thread");
    if (etx_thread)
    {
        wake_up_process(etx_thread);
    }
    else
    {
        printk(KERN_ERR "Cannot create kthread\n");
        return -1;
    }
    printk(KERN_INFO "Done\n");

    return 0;

}

/*
** Module exit function
*/
static void __exit SoC_exit(void)
{
    kthread_stop(etx_thread);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "SoC ended\n");
}

module_init(SoC_init);
module_exit(SoC_exit);