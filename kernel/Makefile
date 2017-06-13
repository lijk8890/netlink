TARGET = test
obj-m := ${TARGET}.o

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
