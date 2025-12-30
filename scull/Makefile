# Comment/uncomment the following line to disable/enable debugging
DEBUG = y


# Add your debugging flag (or not) to CFLAGS
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DSCULL_DEBUG # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

ccflags-y += $(DEBFLAGS)
ccflags-y += -I$(LDDINC)

ifneq ($(KERNELRELEASE),)

	# called from kernel build system
	scull-objs := main.o
	obj-m := scull.o

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    	PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

.PHONY: modules modules_install clean

endif

