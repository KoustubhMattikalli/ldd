#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include "scull.h"

int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);

MODULE_AUTHOR("Koustubh Mattikalli");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("scull module");

struct scull_dev *scull_devices;

int scull_open(struct inode *inode, struct file *filp);
int scull_release(struct inode *inode, struct file *filp);
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t* f_pos);
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t* f_pos);

struct file_operations scull_fops = {
	.owner	 = THIS_MODULE,
	.read	 = scull_read,
	.write	 = scull_write,
	.open	 = scull_open,
	.release = scull_release,
};

static void scull_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(scull_major, scull_minor);

	if (scull_devices) {
		for (i = 0; i < scull_nr_devs; i++) {
			cdev_del(&scull_devices[i].cdev);
		}
		kfree(scull_devices);
	}

	unregister_chrdev_region(devno, scull_nr_devs);
}

int scull_open(struct inode *inode, struct file *filp){ return 0; }
int scull_release(struct inode *inode, struct file *filp){ return 0; }
ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t* f_pos) { return 0; }
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t* f_pos) { return 0; }

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
	int err, devno = MKDEV(scull_major, scull_minor + index);

	cdev_init(&dev->cdev, &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops; /* this line is not required as cdev_init already doing the same */
				     /* we keeping the line to look it similar to code snippet in ldd3 book */
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error: %d adding scull%d\n", err, index);
}

static int scull_init_module(void)
{
	int result, i;
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

	scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
	if (!scull_devices) {
		result = -ENOMEM;
		goto fail;
	}

	for(i = 0; i < scull_nr_devs; i++) {
		scull_setup_cdev(&scull_devices[i], i);
	}

	return 0;

fail:
	scull_cleanup_module();
	return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);
