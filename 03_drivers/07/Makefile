
ifneq ($(KERNELRELEASE),)
# This define by some magic the module name
obj-m  +=  skeleton.o
# This define the object needed the make the module. The .C file names will be extrapolated from that.
mymodule-objs:= skeleton.o

else

# Variable needed for the cross-compilation
CPU := arm
KDIR := ~/workspace/xu3/buildroot/output/build/linux-4.3.3/
TOOLS := ~/workspace/xu3/buildroot/output/host/usr/bin/arm-linux-gnueabihf-

# Source path
PWD := $(shell pwd)
MODPATH := /tftpboot/odroidxu3

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules
	
install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install
    
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	
endif

