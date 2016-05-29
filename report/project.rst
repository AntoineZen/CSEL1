Mini-Project
============

In this project we have the task to create a system made of 3 software:

 - A kernel driver
 - A daemon application that uses the driver
 - A client that controls the daemon

The kernel driver enable to read the CPU temperature and to drive the CPU's fan using PWM.
It has also two mode. One is manual mode, where an user-land application can control the fan speed. In the seconde *"automatic"* mode, the kernel module drives the fan speed according the readen temperature.

The dameon application uses the interface provided by the kernel driver to manage it. In addition, the dameon appicion can be controller in two way: by an application that comunicate with it using IPC or directly by the hardware of the Odroid. The button and led can be used to change the mode and to set the fan speed in manual mode.

The client application can do exacly the same task that the button but can also read the system status.

Kernel Driver
-------------

The kernel driver uses sysfs as a interface as it is easyer to read & write some simple parameter as we will have in this application. We are not making heavy data-exchange that whould requires a stream or memory interface. 

It will run a thread to handle the temperature read-out and to implement the automatic mode.
It will also drives the PWM.

The driver will expose 3 variables to the user application:

	:duty: Current duty cycle (read/write, read only in auto mode)
	:mode: Current working mode, auto or manual
	:temp: Current CPU termperature.

The kerenel module will uses the following data structre to handle its internal state:

.. code-block:: c

	struct fan_ctrl_t{
	    char mode[100];
	    int duty;
	    int temp;
	    struct pwm_device* pwm;
	};

	static struct fan_ctrl_t fan_ctrl_var = {
	    .mode=MODE_AUTO,
	    .duty =  20,
	};


Sysfs inteface
^^^^^^^^^^^^^^

The driver paramters will be materialized in the sysfs structure by the following files:

 - duty: ``/sys/devices/platform/fan-ctrl/duty``
 - mode: ``/sys/devices/platform/fan-ctrl/mode``
 - temp: ``/sys/devices/platform/fan-ctrl/temp``


This is realized in the code by declaring them as a device attributes:

.. code-block:: c

	DEVICE_ATTR(mode, 0660, fan_ctrl_show_mode, fan_ctrl_store_mode);
	DEVICE_ATTR(duty, 0660, fan_ctrl_show_duty, fan_ctrl_store_duty);
	DEVICE_ATTR(temp, 0440, fan_ctrl_show_temp, NULL);

And then in the module initialization function, a sysfs driver is declared and the access files are created:

.. code-block:: c

	static void sysfs_dev_release(struct device * dev) {}

	static struct platform_driver sysfs_driver= {
	    .driver = {.name = DRIVER_NAME,},
	};

	static struct platform_device sysfs_device = {
	    .name= DRIVER_NAME,
	    .id = -1,
	    .dev.release = sysfs_dev_release
	};

	static int __init skeleton_init(void)
	{
	    int status = 0;

	    //sysfs register driver+device and add devices attributes
	    if (status == 0)
	        status = platform_driver_register(&sysfs_driver);
	    if (status == 0)
	        status = platform_device_register(&sysfs_device);
	    if (status == 0)
	    {
	        device_create_file(&sysfs_device.dev, &dev_attr_mode);
	        device_create_file(&sysfs_device.dev, &dev_attr_duty);
	        device_create_file(&sysfs_device.dev, &dev_attr_temp);
	    }

	    // ....

	}

For each attribute there is a read & write function. Those are not detailed here, as they are prety straitforaward. They just made type convertion and value checking.

Thread
^^^^^^

The thread is created from the module initialization fuction:

.. code-block:: c

	static int __init skeleton_init(void)
	{
		// ...
	    fan_ctrl_task = kthread_run(fan_ctrl_thread, &fan_ctrl_var, "fan-ctrl thread");
	    if(fan_ctrl_task == NULL)
	    {
	        pr_err("Unable to start fan-ctrl thread.\n");
	        return 1;
	    }

	    // ...
	}

The thread initizaile the PWM output before entering the main loop:

.. code-block:: c

	int fan_ctrl_thread(void* data)
	{
	    int i;
	    int temp_work;
	    int ret;
	    int duty_ns;

	    // get the passed parameters
	    struct fan_ctrl_t* state = (struct fan_ctrl_t*)data;

	    // Enable PWM
	    ret = pwm_enable(state->pwm);
	    if(ret)
	    {
	        pr_err("Error enabling PWM output\n");
	    }

	    // Initial seting of pwm
	    duty_ns = (PERIOD_NS * state->duty) / 100;
	    ret = pwm_config(state->pwm, duty_ns, PERIOD_NS);
	    if(ret)
	    {
	        pr_err("Error setting PWM output\n");
	    }

	    while(!kthread_should_stop())
	    {
	        // ...
	    }	
	}


In the main loop makes tree tasks:

 - Read the CPU temperature
 - Compute the required fan speed, if in auto mode
 - Update PWM output

When this sequence is done, it sleeps for half a second and then restarts.

The CPU has many "Thermal zone", we need to agredate the global CPU temerature simply by selecting the maxiumum temperature:

.. code-block:: c

    // get the maxiumum temperature of the CPU
    state->temp = -50000;
    for(i = 0; i< sizeof(th_zones); i++)
    {
        thermal_zone_get_temp(thermal_zone_get_zone_by_name(th_zones[i]), &temp_work);
        state->temp = MAX(state->temp, temp_work);
    }


The management of the fan speed, for the **auto** mode is done as following:

.. code-block:: c

    // Do we are in auto mode ?
    if(strcmp(state->mode, MODE_AUTO) == 0)
    { 
        // Change the duty accoring the temperature
        if( state->temp < 57000 )
            state->duty = 0;
        else if( state->temp < 63000 )
            state->duty = 20;
        else if( state->temp < 68000 )
            state->duty = 50;
        else 
            state->duty = 100;
    }


The PWM duty-cycle is then update:

.. code-block:: c

    // Update PWM output
    duty_ns = (PERIOD_NS * state->duty) / 100;
    ret = pwm_config(state->pwm, duty_ns, PERIOD_NS);
    if(ret)
    {
        pr_err("Error setting PWM output\n");
    }


Finaly, the threads sleep for a second.

.. code-block:: c

    // Sleep for half a second
    msleep(500);

PWM ouput
^^^^^^^^^

The PWM output drives the CPU's fan speed. To use the PWM output, it first need to be requested from the module initialization function:

.. code-block:: c

	static int __init skeleton_init(void)
	{
	    // ...
	    fan_ctrl_var.pwm = pwm_request(0, DRIVER_NAME);
	    // ...
	}


The rest of the PWM initilsation is done in the beginning of the thread code:

.. code-block:: c

    // Enable PWM
    ret = pwm_enable(state->pwm);
    if(ret)
    {
        pr_err("Error enabling PWM output\n");
    }

    // Initial seting of pwm
    duty_ns = (PERIOD_NS * state->duty) / 100;
    ret = pwm_config(state->pwm, duty_ns, PERIOD_NS);
    if(ret)
    {
        pr_err("Error setting PWM output\n");
    }


It should also be released when the module is unloaded:

.. code-block:: c

	static void __exit skeleton_exit(void)
	{
	    kthread_stop(fan_ctrl_task);

	    pwm_free(fan_ctrl_var.pwm);

	    //...
	}	

Daemon
------

Introduction
This project is to realize a Daemon that communicate with the driver and the application. For the communication with the driver, the sysfs communication will be used and for the application, we decided to use an ip linux socket as IPC.


Overview
The program communicate with the driver, it can set the duty cycle (0 to 100) value and change the mode (auto or manual). The temperature can be read (RO) from the driver.

The button on the board change the mode, and the fan speed. An application communicate through socket to interact with the daemon is implemented. All datas are passed to the driver through the daemon application, as shown in the following image.

.. image:: 07_img/block.png

Board interaction
^^^^^^^^^^^^^^^^^

The user can user 3 switch on the extension board to change the mode and control the speed of the fan.

 - The first button SW1 switch the mode between auto and manual.
 - The second button SW2 increase 10% the fan speed on manual mode otherwise in auto mode do nothing
 - The button SW3 does the same as SW2 but decrease 10% the speed fan.

Socket interaction
^^^^^^^^^^^^^^^^^^

The fan speed can be change and set from a remote socket connected to this daemon. The protocol implemented is the following:

	:mode=[auto,manual]:	to change the mode in the mode specified by a string end line terminated.

	:duty=[0-100]:	to change the duty cycle, can be a value from 0 to 100

	:show:	to display the current settings in this format mode[],duty[],temp[]

	:help:	show this help
	
	
	
Explication
^^^^^^^^^^^

The daemon program is divided in five parts as follow:

 1. Initialize the daemon
 2. Initialize the communication with GPIO (LED,SWITCH), the driver, the server(port=8080), timer
 3. Set the default states (mode=auto, duty=50)
 4. Create EPOLL loop
 5. Start forever in the event loop

All the following part are synchronized in the EPOLL loop.

Daemon
^^^^^^

The initialize of the daemon the process is the following

 - fork off the parent process
 - create new session
 - fork again to get rid of session leading process
 - capture all required signals
 - update file mode creation mask
 - change working directory to appropriate place
 - close all open file descriptors
 - redirect stdin, stdout and stderr to /dev/null
 - option: open syslog for message logging
 - option: get effective user and group id for appropriate's one
 - option: change root directory
 - option: change effective user and group id for appropriate's one
 - launch the body function

The creation of the daemon is done in the main function then it calls the body function that run the program.

GPIO
^^^^

The following function open a gpio in which direction is passed in parameter. Then for the led the direction should be in output and for the button in input:

.. code-block:: c

	static int open_gpio(char* pin, char* direction)

Driver
^^^^^^

The communication with the driver is by SYFS, then the only things to do is to open the following filedescriptor in the program and the communication is bidirectional, thanks to the last parameter.

.. code-block:: c

	int fmode = open("/sys/devices/platform/fan-ctrl/mode",O_RDWR);
	int fduty = open("/sys/devices/platform/fan-ctrl/duty",O_RDWR);
	int ftemp = open("/sys/devices/platform/fan-ctrl/temp",O_RDONLY);


Server
^^^^^^

The server is listening on port 8080, by these two function the server listen and accept connection in a non-blocking state:

.. code-block:: c

	int make_socket_non_blocking (int sfd)
	int create_and_bind (char *port)

The following part of code create the server and make the server socket non-blocking. In this mode the program can continue to run even if no client is connected.

.. code-block:: c

    int sfd = create_and_bind ("8080");
    if (sfd == -1)
        exit(-1);
    ret = make_socket_non_blocking (sfd);
    if (ret == -1)
        exit(-1);
    ret = listen (sfd, SOMAXCONN);
    if (ret == -1){
        perror ("listen");
        exit(-1);
    }


Timer
^^^^^

The timer is started when a user button is pressed and cleared after a while. Then reset the led state. Its reset if the button is pressed while the downcount.

.. code-block:: c

    timfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 400000000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;


Each 400ms the leds are shutdown, when they light up.



Application
-----------

