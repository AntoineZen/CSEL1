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
#define SW_PREFIX	"/sys/class/gpio/gpio"
#define SW1     "29"
#define SW2     "30"
#define SW3     "22"

#define PWM_INC 10
#define PWM_NS_MULT 100000
#define MAX(x, y) ((x>y)?x:y)
#define SET_TIM(x, y) {\
 	x.it_interval.tv_sec=0;\
  	x.it_interval.tv_nsec=y;\
  	x.it_value.tv_sec=0;\
   	x.it_value.tv_nsec=y;}

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

static int open_switch(char* pin)
{
	char buffer[32];
		// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	if(f < 0)
	{
		perror("unexport");
		exit(-1);
	}
	write (f, pin, strlen(pin));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	if(f < 0)
	{
		perror("export");
		exit(-1);
	}
	write (f, pin, strlen(pin));
	close (f);	

	// config pin as output
	sprintf(buffer, "%s%s%s", SW_PREFIX, pin, "/direction");
	//printf("writing to %s\n", buffer);
	f = open (buffer, O_WRONLY);
	if(f < 0)
	{
		perror("Setting pin as output");
		exit(-1);
	}
	write (f, "in", strlen("in"));
	close (f);

	// Config pin for rising edge event
	sprintf(buffer, "%s%s%s", SW_PREFIX, pin, "/edge");
	//printf("writing to %s\n", buffer);
	f = open (buffer, O_WRONLY);
	write (f, "rising", strlen("rising"));
	if(f < 0)
	{
		perror("event");
		exit(-1);
	}
	close (f);

	// open gpio value attribute
	sprintf(buffer, "%s%s%s", SW_PREFIX, pin, "/value");
	//printf("Opening %s\n", buffer);
	if(f < 0)
	{
		perror("Open for reading");
		exit(-1);
	}
 	f = open (buffer, O_RDWR);
	return f;
}

int main() 
{
	char dummy[64];

	// Open the syslog
	openlog("PWM control", LOG_PERROR, LOG_USER);
	int sw1_fd = open_switch(SW1);
	int sw2_fd = open_switch(SW2);
	int sw3_fd = open_switch(SW3);

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

	int max_fd = MAX(sw1_fd, sw2_fd);
	max_fd = MAX(max_fd, sw3_fd);
	max_fd = MAX(max_fd, tim_fd);

	while(1) {
		fd_set fds_sw, fds_tim;
		FD_ZERO(&fds_sw);
		FD_ZERO(&fds_tim);
		FD_SET(tim_fd, &fds_tim);
		FD_SET(sw1_fd, &fds_sw);
		FD_SET(sw2_fd, &fds_sw);
		FD_SET(sw3_fd, &fds_sw);

		// Look  if the timer thas overflows
		int ret = select(tim_fd+1, &fds_tim, NULL, &fds_sw, NULL);
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
			else if (FD_ISSET(sw1_fd, &fds_sw))
			{
				// Make sure the select will block
				read(sw1_fd, dummy, 10);
				lseek(sw1_fd, 0, SEEK_SET);

				// Manage duty cycle
				if (duty_set < 100)
					duty_set += PWM_INC;
				printf("Duty cyle is now %d\n", duty_set);
			}
			else if (FD_ISSET(sw2_fd, &fds_sw))
			{
				// Make sure the select will block
				read(sw2_fd, dummy, 10);
				lseek(sw2_fd, 0, SEEK_SET);
				
				// Manage duty cycle
				duty_set = 50;
				printf("Duty cyle is now %d\n", duty_set);
			}
			else if (FD_ISSET(sw3_fd, &fds_sw))
			{
				// Make sure the select will block
				read(sw3_fd, dummy, 10);
				lseek(sw3_fd, 0, SEEK_SET);
				
				// Manage duty cycle
				if (duty_set > 0)
					duty_set -= PWM_INC;
				printf("Duty cyle is now %d\n", duty_set);
			}
			else
			{
				printf("Unkown fd set in select\n");
			}
		}


	}

	close(tim_fd);
	close(pwm);
	close(sw3_fd);
	close(sw2_fd);
	close(sw1_fd);

	closelog();

	return 0;
}


