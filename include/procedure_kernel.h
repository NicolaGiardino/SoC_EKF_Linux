/****************************************************************************************
* This file is part of The SoC_EKF_Linux Project.                                       *
*                                                                                       *
* Copyright � 2020-2021 By Nicola di Gruttola Giardino. All rights reserved.           * 
* @mail: nicoladgg@protonmail.com                                                       *
*                                                                                       *
* SoC_EKF_Linux is free software: you can redistribute it and/or modify                 *
* it under the terms of the GNU General Public License as published by                  *
* the Free Software Foundation, either version 3 of the License, or                     *
* (at your option) any later version.                                                   *
*                                                                                       *
* SoC_EKF_Linux is distributed in the hope that it will be useful,                      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                         *
* GNU General Public License for more details.                                          *
*                                                                                       *
* You should have received a copy of the GNU General Public License                     *
* along with The SoC_EKF_Linux Project.  If not, see <https://www.gnu.org/licenses/>.   *
*                                                                                       *
* In case of use of this project, I ask you to mention me, to whom it may concern.      *
*****************************************************************************************/
#ifndef PROCEDURE_KERNEL_H
#define PROCEDURE_KERNEL_H

/* THIS IS JUST A FOUND PROTOTYPE OF KERNEL THREADS, TO BE MODIFIED AFTER HAVING TESTED AND DEBUGGED USER LEVEL */

/* Include Global Parameters */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <linux/module.h>	    /* included for all kernel modules */
#include <linux/kernel.h>		    /* included for KERN_INFO */
#include <linux/init.h>      /* included for __init and __exit macros */
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>					 /* kmalloc() */
#include <linux/uaccess.h>			/* copy_to/from_user() */
#include <linux/kthread.h>			  /* kernel threads */
#include <linux/sched.h>			   /* task_struct */
#include <linux/delay.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("di Gruttola Giardino Nicola");
MODULE_DESCRIPTION("HuskyBMS Master");
MODULE_VERSION("0.0.1");            /* Alpha Version */

dev_t dev = 0;
static struct class* dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

static struct task_struct* etx_thread;

/*
** Function Prototypes
*/
/*************** Driver Fuctions **********************/
static int etx_open(struct inode* inode, struct file* file);
static int etx_release(struct inode* inode, struct file* file);
static ssize_t etx_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off);
static ssize_t etx_write(struct file* filp,
    const char* buf, size_t len, loff_t* off);
/******************************************************/

int thread_function(void* pv);

/*
** Thread
*/
int thread_function(void* pv)
{
    int i = 0;
    while (!kthread_should_stop()) {
        printk(KERN_INFO "In EmbeTronicX Thread Function %d\n", i++);
        msleep(1000);
    }
    return 0;
}

/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner = THIS_MODULE,
        .read = etx_read,
        .write = etx_write,
        .open = etx_open,
        .release = etx_release,
};

/*
** This fuction will be called when we open the Device file
*/
static int etx_open(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}

/*
** This fuction will be called when we close the Device file
*/
static int etx_release(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "Device File Closed...!!!\n");
    return 0;
}

/*
** This fuction will be called when we read the Device file
*/
static ssize_t etx_read(struct file* filp,
    char __user* buf, size_t len, loff_t* off)
{
    printk(KERN_INFO "Read function\n");

    return 0;
}

/*
** This fuction will be called when we write the Device file
*/
static ssize_t etx_write(struct file* filp,
    const char __user* buf, size_t len, loff_t* off)
{
    printk(KERN_INFO "Write Function\n");
    return len;
}

/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
    /*Allocating Major number*/
    if ((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) < 0) {
        printk(KERN_INFO "Cannot allocate major number\n");
        return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    /*Creating cdev structure*/
    cdev_init(&etx_cdev, &fops);

    /*Adding character device to the system*/
    if ((cdev_add(&etx_cdev, dev, 1)) < 0) {
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }

    /*Creating struct class*/
    if ((dev_class = class_create(THIS_MODULE, "etx_class")) == NULL) {
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }

    /*Creating device*/
    if ((device_create(dev_class, NULL, dev, NULL, "etx_device")) == NULL) {
        printk(KERN_INFO "Cannot create the Device \n");
        goto r_device;
    }

    etx_thread = kthread_create(thread_function, NULL, "eTx Thread");
    if (etx_thread) {
        wake_up_process(etx_thread);
    }
    else {
        printk(KERN_ERR "Cannot create kthread\n");
        goto r_device;
    }
    printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;


r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    cdev_del(&etx_cdev);
    return -1;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
    kthread_stop(etx_thread);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device Driver Remove...Done!!\n");
}


static int __init hello_init(void)
{
    printk(KERN_INFO "Hello world!\n");
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

static void __exit hello_cleanup(void)
{
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);











#endif