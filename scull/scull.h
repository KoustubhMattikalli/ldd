#ifndef __SCULL_H__
#define __SCULL_H__

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0	/* Dynamic major by default */
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4
#endif

struct scull_dev {
	struct cdev cdev;
};

#endif //__SCULL_H__
