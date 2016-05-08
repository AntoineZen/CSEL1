File Systems
============

In this lab, we write an application that can very the CPU's fan speed based on button inputs. There will be one button to speed-up the fan, one to slow it down, and the last one will reset to a speed of 50%. 

The wohle probram uses GPIO at input and output using the file interface provided in ``/sys``. The programm will use the ``select()`` function to wait on an event. We will use two kind of events:

 - Button press
 - Timer event.

This is done in the code as follow:

.. code-block:: c

	fd_set fds_sw, fds_tim;
	FD_ZERO(&fds_sw);
	FD_ZERO(&fds_tim);
	FD_SET(tim_fd, &fds_tim);
	FD_SET(sw1_fd, &fds_sw);
	FD_SET(sw2_fd, &fds_sw);
	FD_SET(sw3_fd, &fds_sw);

	// Look  if the timer thas overflows
	int ret = select(max_fd+1, &fds_tim, NULL, &fds_sw, NULL);

Note that two different ``fd_set`` structures are used, one for each event types. This is beacause button raises an *"error"* type event when the state have changed. In the above code, the ``tim_fd`` file descitor is a special desciptor pointing to a timer.

To generate the output PWM signal, a timer is used. The timer is readable upon overflow. We will use a periodic timer as follow:

.. code-block:: c

	#define PWM_INC 10
	#define PWM_NS_MULT 100000

	int tim_fd = timerfd_create(CLOCK_REALTIME, 0);
	struct itimerspec tim_dur;
	SET_TIM(tim_dur, 100/PWM_INC * PWM_NS_MULT);
	timerfd_settime(tim_fd, 0, &tim_dur, NULL);

The application will then be blocked and not consuming resources as long no event comes. It will be woken at periodic interval or when a button is pressed in a interput manner.


Source code
-----------

.. literalinclude:: ../04_file_system/tp1.c
   :language: c