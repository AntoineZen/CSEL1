Drivers
=======


Memory Oriented Drivers
-----------------------

The following  program is able to read direcly some register from the user space using the `/dev/mem` driver:

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