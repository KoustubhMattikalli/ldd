#ifndef __SCULL_H__
#define __SCULL_H__

#include <linux/ioctl.h>

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


/* ioctl definations */

#define SCULL_IOC_MAGIC 'k'
#define SCULL_IOCRESET _IO(SCULL_IOC_MAGIC, 0)

/*
 * S means "set": through a ptr
 * T means "Tell": directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomatically
 * H means "sHift": switch T and Q automatically
 */

#define SCULL_IOCSQUANTUM _IOW(SCULL_IOC_MAGIC, 1, int)
#define SCULL_IOCSQSET	  _IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_IOCTQUANTUM _IO(SCULL_IOC_MAGIC,  3)
#define SCULL_IOCTQSET    _IO(SCULL_IOC_MAGIC,  4)
#define SCULL_IOCGQUANTUM _IOR(SCULL_IOC_MAGIC, 5, int)
#define SCULL_IOCGQSET    _IOR(SCULL_IOC_MAGIC, 6, int)
#define SCULL_IOCQQUANTUM _IO(SCULL_IOC_MAGIC,  7)
#define SCULL_IOCQQSET	  _IO(SCULL_IOC_MAGIC,  8)
#define SCULL_IOCXQUANTUM _IOR(SCULL_IOC_MAGIC, 9, int)
#define SCULL_IOCXQSET    _IOR(SCULL_IOC_MAGIC,10, int)
#define SCULL_IOCHQUANTUM _IO(SCULL_IOC_MAGIC, 11)
#define SCULL_IOCHQSET    _IO(SCULL_IOC_MAGIC, 12)

/* TODO: ioctl for pipe */

#define SCULL_IOC_MAXNR 14

#endif //__SCULL_H__
