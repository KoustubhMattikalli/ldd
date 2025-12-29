#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>

#include "scull.h"

int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);

MODULE_AUTHOR("Koustubh Mattikalli");
MODULE_LICENSE("Dual BSD/GPL");

static void scull_cleanup_module(void)
{
	dev_t dev = MKDEV(scull_major, scull_minor);

	unregister_chrdev_region(dev, scull_nr_devs);
}

static int scull_init_module(void)
{
	int result;
	dev_t dev;

	if (scull_major) {
		dev = MKDEV(scull_major, scull_minor);
		result = register_chrdev_region(dev, scull_nr_devs, "scull");
	} else {
		result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
		scull_major = MAJOR(dev);
	}

	if (result) {
		printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
		return result;
	}

	return 0;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);
