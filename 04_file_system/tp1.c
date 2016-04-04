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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

/*
 * pwm0 - gpb2.0 - 203
 */
#define GPIO_EXPORT	"/sys/class/gpio/export"
#define GPIO_UNEXPORT	"/sys/class/gpio/unexport"
#define GPIO_PWM	"/sys/class/gpio/gpio203"
#define PWM		"203"

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

int main(int argc, char* argv[]) 
{
	long duty = 50;
	if (argc >= 2)   duty   = atoi (argv[1]);

	// frequency = 1 kHz
	// compute duty period...
	long p1 = 5000000 / 100 * duty;
	long p2 = 5000000 - p1;

 	int pwm = open_pwm();
	write (pwm, "1", sizeof("1"));

	struct timespec t1;
	clock_gettime (CLOCK_MONOTONIC, &t1);

	int k = 0;
	while(1) {
		struct timespec t2;
		clock_gettime (CLOCK_MONOTONIC, &t2);

		long delta = (t2.tv_sec  - t1.tv_sec) * 1000000000 +
			     (t2.tv_nsec - t1.tv_nsec);

		int toggle = ((k == 0) && (delta >= p1))
			   | ((k == 1) && (delta >= p2));
		if (toggle) {
			t1 = t2;
			k = (k+1)%2;
			if (k == 0) 
				write (pwm, "1", sizeof("1"));
			else
				write (pwm, "0", sizeof("0"));
		}
	}

	return 0;
}


