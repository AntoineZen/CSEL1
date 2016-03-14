# Part executed when called from kernel build system
ifneq ($(KERNELRELEASE),)
obj-m  +=  mymodule.o
## nameof the generated module 
mymodule-objs:= skeleton.o
## listof objets neededby thatmodule
# Part executed when called from standard make in the module source directory
else
CPU := arm
KDIR := ~/workspace/xu3/buildroot/output/build/linux-4.3.3/
TOOLS := ~/workspace/xu3/buildroot/output/host/usr/bin/arm-linux-gnueabihf-
PWD := $(shell pwd)
MODPATH := /tftpboot/odroidxu3

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules
	
install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install
    
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif

