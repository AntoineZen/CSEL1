Kernel Modules
==============


Kernel module basics
--------------------


1) Generation of a "out of tree" kernel module for the ODROID-XU3
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

We can make a simple module:

.. code-block:: c

    #include <linux/module.h>
    #include <linux/init.h>
    #include <linux/kernel.h>
    
    
    static int __init skeleton_init(void  )
    {
        pr_info("Linux module skeleton loaded\n");
        return 0;
    }
    
    static void __exit skeleton_exit(void  )
    {
        pr_info("Linux module skeletonunloaded\n");
    }
    
    module_init(skeleton_init);
    module_exit(skeleton_exit);
    
    MODULE_AUTHOR("Antoine Zen-Ruffinen <antoine.zen@gmail.com>");
    MODULE_DESCRIPTION("Module skeleton");
    MODULE_LICENSE("GPL");
    
And the appropriate Makefile:

.. code-block:: make
    
    ifneq ($(KERNELRELEASE),)
    # This define by some magic the module name
    obj-m  +=  mymodule.o
    # This define the object needed the make the module. The .C file names will be extrapolated from that.
    mymodule-objs:= skeleton.o
    
    else
    
    # Variable needed for the cross-compilation
    CPU := arm
    KDIR := ~/workspace/xu3/buildroot/output/build/linux-4.3.3/
    TOOLS := ~/workspace/xu3/buildroot/output/host/usr/bin/arm-linux-gnueabihf-
    
    # Source path
    PWD := $(shell pwd)
    
    all:
    	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules
    	       
    clean:
    	$(MAKE) -C $(KDIR) M=$(PWD) clean
    	
    endif


We can compile the kernel module with a simple `make` command:

.. code-block:: console

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
    
    
When this is done, we can copy it to the NFS file system :

.. code-block:: console

    antoine@antoine-vb-64:~/master/CSEL1/02_kernel_modules$ cp mymodule.ko ~/workspace/
    
    
On the odroid, the kernel module is then available:

.. code-block:: console

    # cd /usr/workspace/
    # ls
    RemoteSystemsTempFiles  mymodule.ko             xu3
    #
    
We can install it. We then see the debug message in the kernel log:

.. code-block:: console

    # insmod mymodule.ko 
    # dmesg
    .....
    [    7.619706] random: nonblocking pool is initialized
    [   12.715841] NET: Registered protocol family 10
    [ 3119.198674] Linux module skeleton loaded
    #
    
    
We can check the modules loaded into the kernel in two ways:

.. code-block:: console

    # lsmod
    Module                  Size  Used by    Tainted: G  
    mymodule                1051  0 
    ipv6                  406889 26 [permanent]
    
    # cat /proc/modules 
    mymodule 1051 0 - Live 0xbf07f000 (O)
    ipv6 406889 26 [permanent], Live 0xbf000000
    
The first command show module name, memory size and usage. The second show the same informations plus some address (maybe the module address ?).

We can then remove the module from the kernel:

.. code-block:: console

    # rmmod mymodule
    ...
    [ 3499.366676] Linux module skeletonunloaded
    
    
To make the module available for the `modprobe` command, we need to add a `install` target to the makefile (above the `clean` target):

.. code-block:: make

    MODPATH := /tftpboot/odroidxu3
   
    	
    install:
    	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install


And then we can install it to the filesystem:

.. code-block:: console

    antoine@antoine-vb-64:~/master/CSEL1/02_kernel_modules$ sudo make install
    make -C ~/workspace/xu3/buildroot/output/build/linux-4.3.3/ M=/home/antoine/master/CSEL1/02_kernel_modules INSTALL_MOD_PATH=/tftpboot/odroidxu3 modules_install
    make[1]: Entering directory `/home/antoine/workspace/xu3/buildroot/output/build/linux-4.3.3'
      INSTALL /home/antoine/master/CSEL1/02_kernel_modules/mymodule.ko
      DEPMOD  4.3.3
    make[1]: Leaving directory `/home/antoine/workspace/xu3/buildroot/output/build/linux-4.3.3'
    
Then we can try the ``modprob`` command on the target:

.. code-block:: console

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

We can see that the module "mymodule" has been loaded into the kernel.

2) Add some parameter the module
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


We can add some parameter to the module:

.. code-block:: c

    ...
    static char* text = "Some blabla";
    module_param(text, charp, 0);
    
    static int some_val = 0;
    module_param(some_val, int, 0);
    
    static int __init skeleton_init(void  )
    {
        pr_info("Linux module skeleton loaded\n");
        pr_info("mymodule: some_val=%d, text=%s\n", some_val, text);
        return 0;
    }
    ...
    
We can then try it on the Odroid:

.. code-block:: console

    # insmod mymodule.ko 
    # dmesg
    ...
    [ 5776.321720] Linux module skeleton loaded
    [ 5681.759363] mymodule: some_val=0, text=Some blabla
    
    # rmmod mymodule.ko 
    # insmod mymodule.ko some_val=56 text="Hello kernel"
    # dmesg
    ....
    [ 5776.321720] Linux module skeleton loaded
    [ 5776.324895] mymodule: some_val=56, text=Hello


Memory Management, libraries and utility functions
--------------------------------------------------

This code creates dynamic elements in the kernel. Some Arguments can be pass in parameter and use inside the code like “element” and “text” in this example. All of the created elements will receive a unique identifier

Example: with 5 elements and «Salut les copains… » as text parameter passed to the module on load with modprobe:

.. code-block:: console

    # modprobe  mymodule elements=5 text="Salut les copains..."
    
    [13013.483087] Linux module ModuleMemory load
    [13013.485794] text: Salut les copains...
    [13013.485794] elements: 5
    [13013.492355] Element : id = 0, text = Salut les copains...
    [13013.497306] Element : id = 1, text = Salut les copains...
    [13013.502920] Element : id = 2, text = Salut les copains...
    [13013.508074] Element : id = 3, text = Salut les copains...
    [13013.513716] Element : id = 4, text = Salut les copains...

 
When the module is removed, a text is displayed and show all elements destroyed:

.. code-block:: console

    # modprobe -r mymodule
    
    [13265.542569] Deleting elements :
    [13265.544162]  Element : id = 0
    [13265.547349]  Element : id = 1
    [13265.550291]  Element : id = 2
    [13265.553172]  Element : id = 3
    [13265.556166]  Element : id = 4
    [13265.559066] [DONE]
    [13265.561114] Linux module ModuleMemory unload

 
/proc/slabinfo
^^^^^^^^^^^^^^

Slab allocation is a memory management mechanism intended for the efficient memory allocation of kernel objects. It eliminates fragmentation caused by allocations and deallocations. The technique is used to retain allocated memory that contains a data object of a certain type for reuse upon subsequent allocations of objects of the same type. It is analogous to an object pool, but only applies to memory, not other resources::

    slabinfo - version: 2.1
    # name            <active_objs> <num_objs> <objsize> <objperslab> <pagesperslab> : tunables <limit> <batchcount> <sharedfactor> : slabdata <active_slabs> <num_slabs> <sharedavail>
    ip6-frags              0      0    200   20    1 : tunables    0    0    0 : slabdata      0      0      0
    RAWv6                 30     30   1088   30    8 : tunables    0    0    0 : slabdata      1      1      0
    UDPLITEv6              0      0   1088   30    8 : tunables    0    0    0 : slabdata      0      0      0
    UDPv6                150    150   1088   30    8 : tunables    0    0    0 : slabdata      5      5      0
    tw_sock_TCPv6          0      0    208   39    2 : tunables    0    0    0 : slabdata      0      0      0
    TCPv6                 45     45   2176   15    8 : tunables    0    0    0 : slabdata      3      3      0
    kcopyd_job             0      0   3384    9    8 : tunables    0    0    0 : slabdata      0      0      0
    dm_io                  0      0     80   51    1 : tunables    0    0    0 : slabdata      0      0      0
    cfq_queue              0      0    152   26    1 : tunables    0    0    0 : slabdata      0      0      0
    bsg_cmd                0      0    288   28    2 : tunables    0    0    0 : slabdata      0      0      0
    mqueue_inode_cache     36     36    896   36    8 : tunables    0    0    0 : slabdata      1      1      0
    fuse_inode             0      0    768   21    4 : tunables    0    0    0 : slabdata      0      0      0
    ntfs_big_inode_cache      0      0   1088   30    8 : tunables    0    0    0 : slabdata      0      0      0
    ntfs_inode_cache       0      0    480   34    4 : tunables    0    0    0 : slabdata      0      0      0
    nfs_direct_cache       0      0    288   28    2 : tunables    0    0    0 : slabdata      0      0      0
    nfs_commit_data       36     36    448   36    4 : tunables    0    0    0 : slabdata      1      1      0
    nfs_inode_cache      384    384   1000   32    8 : tunables    0    0    0 : slabdata     12     12      0
    fat_inode_cache        0      0    792   20    4 : tunables    0    0    0 : slabdata      0      0      0
    fat_cache              0      0     24  170    1 : tunables    0    0    0 : slabdata      0      0      0
    jbd2_transaction_s      0      0    192   21    1 : tunables    0    0    0 : slabdata      0      0      0
    jbd2_journal_handle      0      0     56   73    1 : tunables    0    0    0 : slabdata      0      0      0
    jbd2_journal_head      0      0     64   64    1 : tunables    0    0    0 : slabdata      0      0      0
    jbd2_revoke_table_s      0      0     16  256    1 : tunables    0    0    0 : slabdata      0      0      0
    ext4_inode_cache       0      0   1256   26    8 : tunables    0    0    0 : slabdata      0      0      0
    ext4_prealloc_space      0      0    104   39    1 : tunables    0    0    0 : slabdata      0      0      0
    ext4_io_end            0      0     40  102    1 : tunables    0    0    0 : slabdata      0      0      0
    ext4_extent_status      0      0     32  128    1 : tunables    0    0    0 : slabdata      0      0      0
    ext2_inode_cache       0      0    928   35    8 : tunables    0    0    0 : slabdata      0      0      0
    dquot                  0      0    320   25    2 : tunables    0    0    0 : slabdata      0      0      0
    kioctx               550    550    640   25    4 : tunables    0    0    0 : slabdata     22     22      0
    rpc_inode_cache        0      0    704   23    4 : tunables    0    0    0 : slabdata      0      0      0
    ip4-frags             22     22    184   22    1 : tunables    0    0    0 : slabdata      1      1      0
    UDP-Lite               0      0    960   34    8 : tunables    0    0    0 : slabdata      0      0      0
    RAW                   34     34    960   34    8 : tunables    0    0    0 : slabdata      1      1      0
    UDP                  170    170    960   34    8 : tunables    0    0    0 : slabdata      5      5      0
    tw_sock_TCP           39     39    208   39    2 : tunables    0    0    0 : slabdata      1      1      0
    request_sock_TCP      34     34    240   34    2 : tunables    0    0    0 : slabdata      1      1      0
    TCP                   80     80   2048   16    8 : tunables    0    0    0 : slabdata      5      5      0
    blkdev_queue          51     51   1848   17    8 : tunables    0    0    0 : slabdata      3      3      0
    blkdev_requests       39     39    208   39    2 : tunables    0    0    0 : slabdata      1      1      0
    biovec-256            60     60   3072   10    8 : tunables    0    0    0 : slabdata      6      6      0
    biovec-128             0      0   1536   21    8 : tunables    0    0    0 : slabdata      0      0      0
    biovec-64              0      0    768   21    4 : tunables    0    0    0 : slabdata      0      0      0
    sock_inode_cache     225    225    640   25    4 : tunables    0    0    0 : slabdata      9      9      0
    skbuff_fclone_cache    168    168    384   21    2 : tunables    0    0    0 : slabdata      8      8      0
    configfs_dir_cache      0      0     56   73    1 : tunables    0    0    0 : slabdata      0      0      0
    file_lock_cache       75     75    160   25    1 : tunables    0    0    0 : slabdata      3      3      0
    net_namespace          0      0   2816   11    8 : tunables    0    0    0 : slabdata      0      0      0
    shmem_inode_cache    805    805    712   23    4 : tunables    0    0    0 : slabdata     35     35      0
    taskstats              0      0    328   24    2 : tunables    0    0    0 : slabdata      0      0      0
    proc_inode_cache     675    675    632   25    4 : tunables    0    0    0 : slabdata     27     27      0
    sigqueue             224    224    144   28    1 : tunables    0    0    0 : slabdata      8      8      0
    bdev_cache           108    108    896   36    8 : tunables    0    0    0 : slabdata      3      3      0
    kernfs_node_cache  26989  27027    104   39    1 : tunables    0    0    0 : slabdata    693    693      0
    inode_cache        18792  18792    600   27    4 : tunables    0    0    0 : slabdata    696    696      0
    dentry             20260  20260    200   20    1 : tunables    0    0    0 : slabdata   1013   1013      0
    iint_cache             0      0     48   85    1 : tunables    0    0    0 : slabdata      0      0      0
    buffer_head          128    128     64   64    1 : tunables    0    0    0 : slabdata      2      2      0
    nsproxy              170    170     24  170    1 : tunables    0    0    0 : slabdata      1      1      0
    vm_area_struct      1242   1242     88   46    1 : tunables    0    0    0 : slabdata     27     27      0
    mm_struct            224    224    576   28    4 : tunables    0    0    0 : slabdata      8      8      0
    files_cache          600    600    320   25    2 : tunables    0    0    0 : slabdata     24     24      0
    signal_cache         576    576    896   36    8 : tunables    0    0    0 : slabdata     16     16      0
    sighand_cache        276    276   1408   23    8 : tunables    0    0    0 : slabdata     12     12      0
    task_struct          299    312   3968    8    8 : tunables    0    0    0 : slabdata     39     39      0
    anon_vma             504    504    112   36    1 : tunables    0    0    0 : slabdata     14     14      0
    radix_tree_node      260    260    312   26    2 : tunables    0    0    0 : slabdata     10     10      0
    trace_event_file    1275   1275     48   85    1 : tunables    0    0    0 : slabdata     15     15      0
    ftrace_event_field   3840   3840     32  128    1 : tunables    0    0    0 : slabdata     30     30      0
    idr_layer_cache      450    450   1072   30    8 : tunables    0    0    0 : slabdata     15     15      0
    page->ptl            816    816     40  102    1 : tunables    0    0    0 : slabdata      8      8      0
    kmalloc-8192          36     36   8192    4    8 : tunables    0    0    0 : slabdata      9      9      0
    kmalloc-4096          80     80   4096    8    8 : tunables    0    0    0 : slabdata     10     10      0
    kmalloc-2048         288    288   2048   16    8 : tunables    0    0    0 : slabdata     18     18      0
    kmalloc-1024        1312   1312   1024   32    8 : tunables    0    0    0 : slabdata     41     41      0
    kmalloc-512          512    512    512   32    4 : tunables    0    0    0 : slabdata     16     16      0
    kmalloc-256          512    512    256   32    2 : tunables    0    0    0 : slabdata     16     16      0
    kmalloc-192         1155   1155    192   21    1 : tunables    0    0    0 : slabdata     55     55      0
    kmalloc-128         3968   3968    128   32    1 : tunables    0    0    0 : slabdata    124    124      0
    kmalloc-64         13760  13760     64   64    1 : tunables    0    0    0 : slabdata    215    215      0
    kmem_cache_node      160    160    128   32    1 : tunables    0    0    0 : slabdata      5      5      0
    kmem_cache           126    126    192   21    1 : tunables    0    0    0 : slabdata      6      6      0



Input/Output access
-------------------
 
The following code access the IO memory space to read the device identifier form CPU register:

.. literalinclude:: ../02_kernel_modules/06/iotest.c
   :language: c

It read one word at address 0x10000000 and decode it.

We can then test the module by installing it:

.. code-block:: console

    # insmod iotest.ko 
    # dmesg
    ...
    [ 7578.946034] Module iotest loaded
    [ 7578.947962] Product=939042, package=0, major=0, minor=1
    # cat /proc/iomem 
    ...
    10000000-100000ff : iotest
    ...
    # rmmod iotest.ko 
    # 
    
    
So we have read the product id =  939042, package id is 0 and version is 0.1. We can see that the region was reserved for the `iotest` module by reading `/proc/iomem`.
 
    

Kernel Threads
--------------

This code instantiate a thread inside the kernel. Each 5 seconds a message will be plotted in the console “Hello from thread !”. ``Ssleep()`` is used to pause the thread for 5 seconds.

.. literalinclude:: ../02_kernel_modules/07/skeleton.c
   :language: c

When the module is loaded inside the kernel, the module plot each 5 seconds the « Hello from thread ! » message and stop when the module is removed from the kernel:

.. code-block:: console

    # insmod mymodule.ko
    
    [17736.188563] Linux module skeleton loaded
    # [17741.194918] Hello from thread !
    [17746.199929] Hello from thread !
    [17751.204932] Hello from thread !
    [17756.209915] Hello from thread !
    [17761.214929] Hello from thread !
    rmmod mymodule.ko
    
    [17766.219913] Hello from thread !
    [17766.221689] Linux module skeleton unloaded

Sleep
-----

In this task, we have two threads. The first one is waiting for an event of the second. The second just send the event every five seconds. For this, we use an event queue. Note that the waiting of an event works together with a C condition. So the producer of the event need to set some condition to **true** before to wake-up the threads waiting for the event/condition. The consumer thread will turn this condition to **false**. In our example the C condition will be as simple boolean global variable (*wakeup* in the code).

Here is the output of the module test:

.. code-block:: console

    # insmod sleep.ko
    # sleep 30
    # dmesg
    ....
    [  133.689821] Thread one started
    [  133.690097] Linux module sleep loaded
    [  133.690153] Thread two started
    [  133.690159] Thread two: Sending event
    [  133.701950] Thread one: Got event
    [  138.691556] Thread two: Sending event
    [  138.693856] Thread one: Got event
    [  143.696559] Thread two: Sending event
    [  143.698843] Thread one: Got event
    [  148.701548] Thread two: Sending event
    [  148.703829] Thread one: Got event
    [  153.706556] Thread two: Sending event
    [  153.708839] Thread one: Got event
    # rmmod sleep.ko 
    # 


Interrupts
----------

This code handle by interrupt the SW1 button press state. And when the button is pressed a message like “Hello SW1” is plotted.

.. literalinclude:: ../02_kernel_modules/09/skeleton.c
   :language: c

Each time the SW1 is pressed a message is displayed on the screen:

.. code-block:: console

    # insmod mymodule.ko
    
    [20036.188563] Linux module skeleton loaded
    # [20041.194918] Hello SW1 !
    [20046.199929] Hello SW1 !
    [20051.204932] Hello SW1 !
    [20056.209915] Hello SW1 !
    [20061.214929] Hello SW1 !
    [20071.214569] Hello SW1 !
    [20072.214349] Hello SW1 !
    
    rmmod mymodule.ko
    
    [17766.221689] Linux module skeleton unloaded


    