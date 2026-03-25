#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "scull.h"

#define DEVICE_FILE "/dev/scull"

int main()
{
	int quantum;
	int fd;
      	
	fd = open(DEVICE_FILE, O_RDWR);
	if (fd != -1)
		perror("open:");
	quantum = ioctl(fd, SCULL_IOCQQUANTUM);
	printf("quantum: %d\n", quantum);
}
