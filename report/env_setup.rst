Environement setup
==================


We need to configure the U-Boot bootloader to download the kernel & root-fs via NFS. This is done by checking the Ethernet connection works and then setup the kernel argument by setting some U-Boot environment variables:

.. code-block:: console

    ODROID-XU3 #
    ODROID-XU3 # usb start
    starting USB...
    USB0:   USB EHCI 1.00
    USB1:   Register 2000140 NbrPorts 2
    Starting the controller
    USB XHCI 1.00
    USB2:   Register 2000140 NbrPorts 2
    Starting the controller
    USB XHCI 1.00
    scanning bus 0 for devices... 1 USB Device(s) found
    scanning bus 1 for devices... 1 USB Device(s) found
    scanning bus 2 for devices... 1 USB Device(s) found
           scanning usb for ethernet devices... 0 Ethernet Device(s) found
    ODROID-XU3 # usb reset
    resetting USB...
    USB0:   USB EHCI 1.00
    USB1:   Register 2000140 NbrPorts 2
    Starting the controller
    USB XHCI 1.00
    USB2:   Register 2000140 NbrPorts 2
    Starting the controller
    USB XHCI 1.00
    scanning bus 0 for devices... 4 USB Device(s) found
    scanning bus 1 for devices... 1 USB Device(s) found
    scanning bus 2 for devices... 1 USB Device(s) found
           scanning usb for ethernet devices... 1 Ethernet Device(s) found
    ODROID-XU3 # ping 192.168.0.4
    Waiting for Ethernet connection... done.
    Using sms0 device
    host 192.168.0.4 is alive
    ODROID-XU3 # setenv bootcmd run nfsboot
    ODROID-XU3 # saveenv
    Saving Environment to MMC...
    Writing to MMC(0)... done
    ODROID-XU3 # 
    
    
We can then start the kernel:

.. code-block:: console

    ODROID-XU3 # run nfsboot
