Drivers
=======


Memory Oriented Drivers
-----------------------

1) Direct memory read
^^^^^^^^^^^^^^^^^^^^^

The following  program is able to read directly some register from the user space using the ``/dev/mem`` driver:

.. literalinclude:: ../03_drivers/01/main.c
   :language: c
   
   
If we run the program on the Odroid, we get the following output:

.. code-block:: console

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
   
   
The driver can then be compiled as usual and installed on the odroid:

.. code-block:: console

    # insmod mmaptestmod.ko 
    # dmesg
    ....
    [  290.763743] Module mmaptest loaded
    #
    
    
We can read the ``/proc/devices`` file to know which major number was alocated to our driver:

.. code-block:: console

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


So we can see here that the **major number** allocated to our driver is **248**. We can then create the node file:

.. code-block:: console

    # mknod /dev/mmaptest c 248 0
    # 
   

Once this is done we can test our user space driver that uses the new kernel driver:

.. code-block:: console

    # ./mmap_test_driver 
    Product=939042, package=0, major=0, minor=1

We can see in the kernel logs that the ``mmaptest_mmap()`` handler was called:

.. code-block:: console

    # dmesg
    ....
    [  290.763743] Module mmaptest loaded
    [ 1574.025906] Module mmaptest: mmap handler called.

This user space driver (application) is the same as in part 1, but uses ``/dev/mmaptest`` instead of ``/dev/mem``. Its source code is the following:

.. literalinclude:: ../03_drivers/02/app/main.c
   :language: c

Charater Oriented Drivers
-------------------------

The following code is the implementation of a character device. This driver is able to read and write data from the user land. We can use echo and cat command to interact with this module respectively write and read.


.. literalinclude:: ../03_drivers/03/skeleton.c
   :language: c

The device character created using the module code is listed below. A lot of other characters can be show in “/proc/devices”:

.. code-block:: console

    # cat /proc/devices
    Character devices:
    ...
    136 pts
    180 usb
    189 usb_device
    204 ttySAC
    226 drm
    248 skeleton
    ...

Now create an access file:

.. code-block:: console
    
    # mknod /dev/mymodule c 248 0

Test using “echo” and “cat” command on the module node:

.. code-block:: console

    # echo hello > /dev/mymodule
    # cat /dev/mymodule
    hello


The previous module code will be updated for more instance compatibility. With a number given at loading time that’s represent the number of instance. Each instance will create a variable that store a value between user land and kernel land:

.. literalinclude:: ../03_drivers/04/skeleton.c
   :language: c

Create load kernel with the number of node that we will use. And create access file using mknod function provided by linux kernel:

.. code-block:: console
    
    # insmod mymodule.ko count=3
    
    # cat /proc/devices
    Character devices:
    ...
    248 skeleton
    ...
    
    # mknod /dev/mymodule1 c 248 0
    # mknod /dev/mymodule2 c 248 1
    # mknod /dev/mymodule3 c 248 2

Test using the previous created node:

.. code-block:: console

    # echo “hello 1” > /dev/mymodule1
    # echo “hello 2” > /dev/mymodule2
    # echo “hello 3” > /dev/mymodule3
    
    # cat /dev/mymodule1
    hello 1
    # cat /dev/mymodule2
    hello 2
    # cat /dev/mymodule3
    hello 3

 

The following code runs in the user space and read and write data from and to mymodule. The following code take one argument pass to the program and write it to the character device driver previously opened, and read back the data.


.. literalinclude:: ../03_drivers/05/main.c
   :language: c

The result of the program is showed below:

.. code-block:: console

    # ./test “Hello”
    Write: Hello
    Read: Hello
    

Blocking operations
-------------------

For the blocking opeation, the kernel module should implement the ``poll()`` callback function. In this mode, the ``pool()`` function should block on some wait-queue. The wait queue should then be woken by some event like an IRQ. The above examle uses an IRQ from a GPIO to control the pooling:

.. literalinclude:: ../03_drivers/06/drv/button.c
   :language: c

Then the user-land application can use ``select()`` to wait on this event. The ``select()`` syscall will relay on the ``.poll`` call-back function of the caracter driver pointed by the passed file descriptor. The following code does this to count the events:

.. literalinclude:: ../03_drivers/06/app/main.c
   :language: c

We can install the driver:

.. code-block:: console

    # insmod button.ko   
    # dmesg
    .....
    [  456.158645] Module button loaded
    # cat /proc/devices 
    Character devices:
      1 mem
      2 pty
    ...
    248 button
    ...
    # mknod /dev/button c 248 0


And test the application:

.. code-block:: console

    # ./button_test &
    # dmesg
    ...
    [  673.581700] Before pool_wait()
    [  673.583421] after pool_wait()
    # fg
    ./button_test
    Counter is now 1.
    Counter is now 2.
    Counter is now 3.
    Counter is now 4.
    Counter is now 5.
    Counter is now 6.
    Counter is now 7.
    Counter is now 8.
    Counter is now 9.
    ...
    ^C
    
We can see that the counter is incremented every time we press on the button. The kernel logs gives some detail on what appends:

.. code-block:: console

    # dmesg
    [  727.092662] pool_wait() return true
    [  727.096203] Before pool_wait()
    [  727.098918] after pool_wait()
    [  727.210328] button_irq()
    [  727.211629] Before pool_wait()
    [  727.214424] after pool_wait()
    [  727.217613] pool_wait() return true
    [  727.221161] Before pool_wait()
    [  727.223873] after pool_wait()
    [  727.312790] button_irq()
    [  727.314086] Before pool_wait()
    [  727.316933] after pool_wait()
    [  727.319828] pool_wait() return true
    [  727.323883] Before pool_wait()
    [  727.326381] after pool_wait()
    [  767.894888] Before pool_wait()
    [  767.896515] after pool_wait()

Sysfs
-----

This code implements a character driver on a sysfs. 3 members can be access “marque”, “model” and “vitesse”. Only “echo” and “cat” are used for the purpose.


.. literalinclude:: ../03_drivers/07/skeleton.c
   :language: c


Search of the skeleton module, the module isn’t mounted yet:

.. code-block:: console

    # ls /sys/bus/devices/platform/
    ...
    hello 3
    alarmtimer
    amba
    arm-pmu
    exynos-cpufreq
    exynos-drm
    gpioleds
    power
    pwmleds
    pwrseq
    reg-dummy
    serial8250
    sound
    uevent
    ...


After a module load, the peripheric is now visible:

.. code-block:: console
 
    # ls /sys/bus/devices/platform/
    ...
    hello 3
    alarmtimer
    amba
    arm-pmu
    exynos-cpufreq
    exynos-drm
    gpioleds
    power
    pwmleds
    pwrseq
    reg-dummy
    serial8250
    skeleton
    sound
    uevent
    ...

Inside the module skeleton, we find “marque”, “model” and “vitesse”:

.. code-block:: console

    # ls /sys/bus/devices/platform/skeleton/
    driver	modalias	power		uevent
    marque	modele	subsystem	vitesse

 
Using all 3 attributes with echo and cat, let’s try:

.. code-block:: console
 
    # cd /sys/bus/devices/platform/skeleton/
    # cat marque
    Subaru
    # cat modele
    Impreza
    # cat vitesse
    # 300
    # echo Ferrari > marque
    # echo California > modele
    # echo 330 > marque
    # cat marque
    Ferrari
    # cat modele
    California
    # cat vitesse
    # 330

Device Manager
--------------

In this part we will use ``mdev`` to create the node file for us. The kernel module for this will take the example of *Blocking device* part but adding the *Sysfs* part to make ``mdev`` able to detect it. The user space application will be the same than for the *Blocking device* part. The code of the merged kernel module is given hereafter:

.. literalinclude:: ../03_drivers/10/drv/button_sysfs.c
   :language: c


We can test the installation of the kernel driver:

.. code-block:: console

    # insmod button_sysfs.ko 
    # dmesg
    ...
    [ 5178.208953] Module button loaded
    
    # ls /sys/devices/platform/skeleton/
    driver           modalias         subsystem
    driver_override  modele           uevent
    marque           power            vitesse
    
    # cat /proc/devices 
    Character devices:
    ...
    248 button
    ...
    # ls /sys/devices/platform/button/
    driver           modalias         subsystem
    driver_override  modele           uevent
    marque           power            vitesse

The ``mdev`` utility will detect the folder in ``/sys/devices/platform`` and create the node file ``/dev/button`` using the following rule::

    button          root:root 666

The rule should be added to ``/etc/mdev.conf``. Then if we run ``mdev -s`` the node file will be created:

.. code-block:: console

    # mdev -s
    # ls /dev/button
    button
    
    







