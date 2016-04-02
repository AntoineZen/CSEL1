Drivers
=======


Memory Oriented Drivers
-----------------------

1) Direct memory read
^^^^^^^^^^^^^^^^^^^^^

The following  program is able to read directly some register from the user space using the ``/dev/mem`` driver:

.. literalinclude:: ../03_drivers/01/main.c
   :language: c
   
   
If we run the program on the Odroid, we get the following output::

    # cd /usr/workspace/
    # ls
    RemoteSystemsTempFiles  mmap_test               sleep.ko
    iotest.ko               mymodule.ko             xu3
    # ./mmap_test 
    Product=939042, package=0, major=0, minor=1
    # 
    
This is the same output we had from the kernel module doing the same job. This means the user space drivers works.

2) mmap with custom driver
^^^^^^^^^^^^^^^^^^^^^^^^^^

To have a driver (in kernel space) that implement the call-back for the mmap() function, the kernel module should register like a character device driver. For this the following operation are required in the module initialization callback:

 1) Allocate a driver major number, using the ``alloc_chredev_region()`` function.
 2) register the ``file_operation`` structure where the ``mmap`` pointer should be pointing to the mmap callback function, usint ``cdev_init()`` and ``cdev_add()`` functions.
 3) register the driver minior number using the ``cdev_add()`` function (and get the minor number).
 
 
The following code implement this kernel driver:

.. literalinclude:: ../03_drivers/02/drv/mmaptestmod.c
   :language: c
   
   
The driver can then be compiled as usual and installed on the odroid::

    # insmod mmaptestmod.ko 
    # dmesg
    ....
    [  290.763743] Module mmaptest loaded
    #
    
    
We can read the ``/proc/devices`` file to know which major number was alocated to our driver::

    # cat /proc/devices 
    Character devices:
      1 mem
      2 pty
      3 ttyp
      4 /dev/vc/0
      4 tty
      4 ttyS
      5 /dev/tty
      5 /dev/console
      5 /dev/ptmx
      7 vcs
     10 misc
     13 input
     21 sg
     29 fb
    128 ptm
    136 pts
    180 usb
    189 usb_device
    204 ttySAC
    226 drm
    248 mmaptest
    249 cros_ec
    250 bsg
    251 watchdog
    252 iio
    253 rtc
    254 tpm
    
    Block devices:
      1 ramdisk
    259 blkext
      7 loop
      8 sd
     65 sd
     66 sd
     67 sd
     68 sd
     69 sd
     70 sd
     71 sd
    128 sd
    129 sd
    130 sd
    131 sd
    132 sd
    133 sd
    134 sd
    135 sd
    179 mmc
    254 device-mapper
    # 
    # 


So we can see here that the **major number** allocated to our driver is **248**. We can then create the node file::

    # mknod /dev/mmaptest c 248 0
    # 
   

Once this is done we can test our user space driver that uses the new kernel driver::

    # ./mmap_test_driver 
    Product=939042, package=0, major=0, minor=1

We can see in the kernel logs that the ``mmaptest_mmap()`` handler was called::

    # dmesg
    ....
    [  290.763743] Module mmaptest loaded
    [ 1574.025906] Module mmaptest: mmap handler called.

This user space driver (application) is the same as in part 1, but uses ``/dev/mmaptest`` instead of ``/dev/mem``. Its source code is the following:

.. literalinclude:: ../03_drivers/02/app/main.c
   :language: c

Charater Oriented Drivers
-------------------------

TBD Yann

Blocking operations
-------------------

TBD Antoine


Sysfs
-----

TBD Yann


Device Manager
--------------

TBD Antoine