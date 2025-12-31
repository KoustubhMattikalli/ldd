#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

#include "scull.h"

int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS;
int scull_quantum = SCULL_QUANTUM;
int scull_qset = SCULL_QSET;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

MODULE_AUTHOR("Koustubh Mattikalli");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("scull module");

struct scull_dev *scull_devices;

int scull_trim(struct scull_dev *dev);
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

int scull_trim(struct scull_dev *dev)
{
	struct scull_qset *next, *dptr;
	int qset = dev->qset;
	int i;

	for (dptr = dev->data; dptr; dptr = next) {
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = scull_quantum;
	dev->qset = scull_qset;
	dev->data = NULL;
	return 0;
}

#ifdef SCULL_DEBUG /* use proc only if debugging */

#if 0 /* Depricated */
int scull_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int i, j, len = 0;
	int limit = count - 80; /*Don't print more then this */

	for (i = 0; i < scull_nr_devs && len <= limit; i++) {
		struct scull_dev *d = &scull_devices[i];
		struct scull_qset *qs = d->data;
		/* todo: semmaphore */
		len += sprintf(buf+len, "\nDevice %i, qset %i, q %i, sz %li\n",
				i, d->qset, d->quantum, d->size);
		for (; qs && len <= limit; qs = qs->next) {
			len += sprintf(buf+len, "  item at %p, qset at %p\n", qs, qs->data);
			if (qs->data && !qs->next) { /* dump only last item */
				for (j = 0; j < d->qset; j++) {
					if (qs->data[j])
						len += sprintf(buf+len, 
								"    % 4i; %8p\n",
								j, qs->data[j]);
				}
			}
		}
		/* todo: semaphore */
	}
	*eof = 1;
	return len;
}
#endif /* Depricated */

static void scull_create_proc(void)
{
#if 0 /* Depricated */
	create_proc_read_entry("scullmem", 0 /* default mode*/,
			NULL /* parent dir */, scull_read_procmem,
			NULL /* client data */);
#endif /* Depricated */
}

static void scull_remove_proc(void)
{
#if 0 /* Depricated */
	remove_proc_entry("scullmem", NULL /* parent dir */);
#endif /* Depricated */
}

#endif //SCULL_DEBUG

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

#ifdef SCULL_DEBUG
	scull_remove_proc();
#endif

	unregister_chrdev_region(devno, scull_nr_devs);
}

int scull_open(struct inode *inode, struct file *filp)
{
	struct scull_dev *dev;

	dev = container_of(inode->i_cdev, struct scull_dev, cdev);
	filp->private_data = dev;

	if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
		/* todo: semaphore */
		scull_trim(dev);
	}
	return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{
	struct scull_qset *qs = dev->data;

	if (!qs) {
		qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
		if (qs == NULL)
			return NULL;
		memset(qs, 0, sizeof(struct scull_qset));
	}

	while(n--) {
		if (!qs->next) {
			qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;
			memset(qs->next, 0, sizeof(struct scull_qset));

		}
		qs = qs->next;
		continue;
	}
	return qs;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t* f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr = NULL;
	int quantum = dev->quantum;
	int qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	/* todo: semphore */

	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	dptr = scull_follow(dev, item);
	if (dptr == NULL || ! dptr->data || ! dptr->data[s_pos]) {
		goto out;
	}

	/* read only upto end of this quantum */
	if (count > quantum - q_pos)
		count  = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;

out:
	/* todo: semphore */
	return retval;

}
ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t* f_pos)
{
	struct scull_dev *dev = filp->private_data;
	struct scull_qset *dptr;
	int quantum = dev->quantum;
	int qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM;

	/* todo: semphore */

	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	dptr = scull_follow(dev, item);
	if (dptr == NULL)
		goto out;

	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char*), GFP_KERNEL);
		if (!dptr->data) {
			goto out;
		}
		memset(dptr->data, 0, qset * sizeof(char*));
	}
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}

	/* write only upto end of this quantum */
	if (count > quantum - q_pos)
		count  = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	*f_pos += count;
	retval = count;

	if (dev->size < *f_pos)
		dev->size = *f_pos;
out:
	/* todo: semaphore */
	return retval;
}

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
		/* todo: Semophore init */
		scull_setup_cdev(&scull_devices[i], i);
	}

#ifdef SCULL_DEBUG
	scull_create_proc();
#endif
	return 0;

fail:
	scull_cleanup_module();
	return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);
