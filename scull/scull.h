#ifndef __SCULL_H__
#define __SCULL_H__

/*
 * Micros to help debugging
 */

#undef PDEBUG
#ifdef SCULL_DEBUG
#	ifdef __KERNEL__	/* kernel space debugging */
#		define PDEBUG(fmt, args...) printk( KERN_DEBUG "scull: " fmt, ## args)
#	else			/* user space debugging */
#		define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#	endif
#else
#	define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0	/* Dynamic major by default */
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4
#endif

#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET 1000
#endif

struct scull_qset {
	void **data;
	struct scull_qset *next;
};

struct scull_dev {
	struct scull_qset *data;/* pointer to first quantum set */
	int quantum;		/* the current quantum size */
	int qset;		/* the current array size */
	unsigned long size;	/* amount of data stored */
	struct cdev cdev;	/* char device structure */
};

#endif //__SCULL_H__
