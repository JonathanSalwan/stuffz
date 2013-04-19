obj-m := your_module.o

KDIR := /your/src/linux
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
