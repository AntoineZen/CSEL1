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

#define SW_PREFIX	"/sys/class/gpio/gpio"
#define SW1     "29"
#define SW2     "30"
#define SW3     "22"

#define PWM_INC 10

 #define MAX(x, y) ((x>y)?x:y)


#define FIFO_NAME "/tmp/pwm_fifo"


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

 	f = open (buffer, O_RDWR);
	if(f < 0)
	{
		perror("Open for reading");
		exit(-1);
	}

			// Make sure the select will block
	read(f, buffer, 10);
	lseek(f, 0, SEEK_SET);

	return f;
}

int main() 
{
	char dummy[64];

	// Open the syslog
	openlog("PWM control", LOG_PERROR, LOG_USER);

	// Open the buttons
	int sw1_fd = open_switch(SW1);
	int sw2_fd = open_switch(SW2);
	int sw3_fd = open_switch(SW3);

	int duty_set = 50;

	// compute the maximum fd number used
	int max_fd = MAX(sw1_fd, sw2_fd);
	max_fd = MAX(max_fd, sw3_fd);

	fd_set fds_sw;

	int ret = mkfifo(FIFO_NAME, 0666);
	if (ret)
	{
		perror("mkfifo()\n");
	}

	int child_fd = fork();
	if (child_fd > 0)
	{
		int fifo_fd = open(FIFO_NAME, O_WRONLY);
		// parent process, will handle the button, passing the duty to the child
		while(1) 
		{
			FD_ZERO(&fds_sw);
			FD_SET(sw1_fd, &fds_sw);
			FD_SET(sw2_fd, &fds_sw);
			FD_SET(sw3_fd, &fds_sw);

			// Look  if the timer thas overflows
			ret = select(max_fd+1, NULL, NULL, &fds_sw, NULL);
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
				if (FD_ISSET(sw1_fd, &fds_sw))
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

				// Send new dutty to child process
				ret = write(fifo_fd, &duty_set, sizeof(duty_set));
				if(ret < 0)
				{
					perror("write()\n");
				}
			}
		}
		close(fifo_fd);
	}

	else
	{
		// Call the slave process that controls the fan
		ret = execlp("./pwm_slave", "pwm_slave", NULL);
		if(ret > 0)
		{
			perror("execlp()\n");
		}
	}

	close(sw3_fd);
	close(sw2_fd);
	close(sw1_fd);

	closelog();

	return 0;
}


