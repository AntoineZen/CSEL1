Kernel Modules
==============

1) Generation of a "out of tree" kernel module for the ODROID-XU3
-----------------------------------------------------------------

We can compile the kernel module with a simple `make` command::

    antoine@antoine-vb-64:~/master/CSEL1/02_kernel_modules$ make
    make -C ~/workspace/xu3/buildroot/output/build/linux-4.3.3/ M=/home/antoine/master/CSEL1/02_kernel_modules ARCH=arm CROSS_COMPILE=~/workspace/xu3/buildroot/output/host/usr/bin/arm-linux-gnueabihf- modules
    make[1]: Entering directory `/home/antoine/workspace/xu3/buildroot/output/build/linux-4.3.3'
      CC [M]  /home/antoine/master/CSEL1/02_kernel_modules/skeleton.o
      LD [M]  /home/antoine/master/CSEL1/02_kernel_modules/mymodule.o
      Building modules, stage 2.
      MODPOST 1 modules
      CC      /home/antoine/master/CSEL1/02_kernel_modules/mymodule.mod.o
      LD [M]  /home/antoine/master/CSEL1/02_kernel_modules/mymodule.ko
    make[1]: Leaving directory `/home/antoine/workspace/xu3/buildroot/output/build/linux-4.3.3'
    antoine@antoine-vb-64:~/master/CSEL1/02_kernel_modules$ 
    
    
When this is done, we can copy it to the NFS file system ::


    antoine@antoine-vb-64:~/master/CSEL1/02_kernel_modules$ cp mymodule.ko ~/workspace/
    
    
On the odroid, the kernel module is then available::


    # cd /usr/workspace/
    # ls
    RemoteSystemsTempFiles  mymodule.ko             xu3
    #
    
We can install it. We then see the debug message in the kernel log::

    # insmod mymodule.ko 
    # dmesg
    .....
    [    7.619706] random: nonblocking pool is initialized
    [   12.715841] NET: Registered protocol family 10
    [ 3119.198674] Linux module skeleton loaded
    #
    
    
We can check the modules loaded into the kernel in two ways::

    # lsmod
    Module                  Size  Used by    Tainted: G  
    mymodule                1051  0 
    ipv6                  406889 26 [permanent]
    
    # cat /proc/modules 
    mymodule 1051 0 - Live 0xbf07f000 (O)
    ipv6 406889 26 [permanent], Live 0xbf000000
    
The first command show module name, memory size and usage. The second show the same informations plus some address (maybe the module address ?).

We can then remove the module from the kernel::

    # rmmod mymodule
    ...
    [ 3499.366676] Linux module skeletonunloaded
    
    
To make the module available for the `modprobe` command, we need to add a `install` target to the makefile and then we can install it to the filesystem::

    antoine@antoine-vb-64:~/master/CSEL1/02_kernel_modules$ sudo make install
    make -C ~/workspace/xu3/buildroot/output/build/linux-4.3.3/ M=/home/antoine/master/CSEL1/02_kernel_modules INSTALL_MOD_PATH=/tftpboot/odroidxu3 modules_install
    make[1]: Entering directory `/home/antoine/workspace/xu3/buildroot/output/build/linux-4.3.3'
      INSTALL /home/antoine/master/CSEL1/02_kernel_modules/mymodule.ko
      DEPMOD  4.3.3
    make[1]: Leaving directory `/home/antoine/workspace/xu3/buildroot/output/build/linux-4.3.3'
    
Then we can try the `modprob`command on the target::

    # modprobe mymodule
    # 
    # dmesg
    ...
    [ 1169.627580] Linux module skeleton loaded
    # lsmod
    Module                  Size  Used by    Tainted: G  
    mymodule                1051  0 
    ipv6                  406996 26 [permanent]
    # 






    








