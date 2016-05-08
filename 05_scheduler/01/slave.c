/**
 * Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Project:	HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: 	System programming -  file system
 *
 * Purpose:	ODROID-XU3 Lite silly fan control system
 *
 * Author:	Wolfram Luithardt
 * Date:	30.03.2016
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

/*
 * pwm0 - gpb2.0 - 203
 */
#define GPIO_EXPORT	"/sys/class/gpio/export"
#define GPIO_UNEXPORT	"/sys/class/gpio/unexport"
#define GPIO_PWM	"/sys/class/gpio/gpio203"
#define PWM		"203"


#define PWM_INC 10
#define PWM_NS_MULT 100000
#define MAX(x, y) ((x>y)?x:y)
#define SET_TIM(x, y) {\
 	x.it_interval.tv_sec=0;\
  	x.it_interval.tv_nsec=y;\
  	x.it_value.tv_sec=0;\
   	x.it_value.tv_nsec=y;}

#define FIFO_NAME "/tmp/pwm_fifo"

static int open_pwm()
{
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	write (f, PWM, strlen(PWM));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	write (f, PWM, strlen(PWM));
	close (f);	

	// config pin
	f = open (GPIO_PWM "/direction", O_WRONLY);
	write (f, "out", 3);
	close (f);

	// open gpio value attribute
 	f = open (GPIO_PWM "/value", O_RDWR);
	return f;
}


int main() 
{
	char dummy[64];

	// Open the syslog
	openlog("PWM control", LOG_PERROR, LOG_USER);

	printf("Slave starting\n");


	int duty_set = 50;
	int duty_current = 0;

	// Set the driver

	// Open PWM output
 	int pwm = open_pwm();
	write(pwm, "1", sizeof("1"));

	// Create the timer
	int tim_fd = timerfd_create(CLOCK_REALTIME, 0);
	struct itimerspec tim_dur;
	SET_TIM(tim_dur, 100/PWM_INC * PWM_NS_MULT);
	timerfd_settime(tim_fd, 0, &tim_dur, NULL);

	fd_set fds_tim;

	int ret = mkfifo(FIFO_NAME, 0666);
	if (ret)
	{
		perror("mkfifo()\n");
	}

	
	int fifo_fd = open(FIFO_NAME, O_RDONLY);
	// child process, will handle the fan
	while(1) 
	{

		FD_ZERO(&fds_tim);
		FD_SET(tim_fd, &fds_tim);
		FD_SET(fifo_fd, &fds_tim);

		// Look  if the timer thas overflows
		ret = select(MAX(tim_fd, fifo_fd)+1, &fds_tim, NULL, NULL, NULL);
		// Manage errors
		if (ret == -1)
		{
			perror("Selec()");
			exit(-1);
		}
		else if (ret == 0)
		{
			perror("Timeout should not occure!");
			exit(-1);
		}
		// Normal case
		else
		{
			// Handle timer
			if (FD_ISSET(tim_fd, &fds_tim))
			{
				read(tim_fd, dummy, 10);
				//printf("Timer!\n");

				duty_current = (duty_current + PWM_INC) % 100;

				// Toggle pin
				if (duty_current < duty_set)
					write(pwm, "1", sizeof("1"));
				else
					write(pwm, "0", sizeof("0"));

			}
			else if (FD_ISSET(fifo_fd, &fds_tim))
			{
				ret = read(fifo_fd, &duty_set, sizeof(duty_set));
				printf("Slave:Got new duty %d\n", duty_set);
				if(ret < 0)
				{
					perror("read()\n");
				}
			}
		}
	}
	close(fifo_fd);
	close(tim_fd);
	close(pwm);
	closelog();

	return 0;
}


