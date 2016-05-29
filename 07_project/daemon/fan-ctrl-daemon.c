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
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include <grp.h>
#include <pwd.h>
#include <signal.h>

#include <sys/socket.h>
#include <netdb.h>

#define GPIO_EXPORT	"/sys/class/gpio/export"
#define GPIO_UNEXPORT	"/sys/class/gpio/unexport"
#define GPIO_PWM	"/sys/class/gpio/gpio203"
#define SW_PREFIX	"/sys/class/gpio/gpio"

#define SW1     "29"
#define SW2     "30"
#define SW3     "22"
#define LED1	"31"
#define LED2	"28"
#define LED3	"19"

#define SOCKETU32 0x10

#define AUTOMODE "auto\n"
#define MANUALMODE "manual\n"

struct ProtocolCmd{
	int index;
	char* cmd;
};

struct ProtocolCmd protocolcmd[] = {
	{1,"show\n"},
	{2,"mode"},
	{3,"duty"},
	{4,"help\n"},
	{-1,NULL}
};

static int signal_catched = 0;

static void catch_signal (int signal)
{
	syslog (LOG_INFO, "signal=%d catched\n", signal);
	signal_catched++;
}

static void fork_process()
{
	pid_t pid = fork();
	switch (pid) {
	case  0: break; // child process has been created
	case -1: syslog (LOG_ERR, "ERROR while forking"); exit (1); break;
	default: exit(0);  // exit parent process with success
	}
}

//set some flag on the socket to be non-blocking
int make_socket_non_blocking (int sfd)
{
	int flags, s;

	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1){
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1){
		perror ("fcntl");
		return -1;
	}

	return 0;
}

//create a socket server and bind to that port
int create_and_bind (char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;     // Return IPv4 and IPv6 choices
	hints.ai_socktype = SOCK_STREAM; // We want a TCP socket
	hints.ai_flags = AI_PASSIVE;     // All interfaces

	s = getaddrinfo (NULL, port, &hints, &result);
	if (s != 0){
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next){
		sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)// We managed to bind successfully!
			break;
		
		close (sfd);
	}

	if (rp == NULL){
		fprintf (stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo (result);

	return sfd;
}

static int open_gpio(char* pin, char* direction)
{
	char buffer[32];
	// unexport pin out of sysfs (reinitialization)
	int f = open (GPIO_UNEXPORT, O_WRONLY);
	if(f < 0){
		perror("unexport");
		exit(-1);
	}
	write (f, pin, strlen(pin));
	close (f);

	// export pin to sysfs
	f = open (GPIO_EXPORT, O_WRONLY);
	if(f < 0){
		perror("export");
		exit(-1);
	}
	write (f, pin, strlen(pin));
	close (f);	

	if(direction != "in" && direction != "out"){
		perror("direction");
		exit(-1);	
	}

	// config pin as input
	sprintf(buffer, "%s%s%s", SW_PREFIX, pin, "/direction");
	f = open (buffer, O_WRONLY);
	if(f < 0){
		perror("Setting pin as input");
		exit(-1);
	}
	write (f, direction, strlen(direction));
	close (f);

	if(direction == "in"){
		// Config pin for rising edge event
		sprintf(buffer, "%s%s%s", SW_PREFIX, pin, "/edge");
		//printf("writing to %s\n", buffer);
		f = open (buffer, O_WRONLY);
		write (f, "rising", strlen("rising"));
		if(f < 0){
			perror("event");
			exit(-1);
		}
		close (f);

	}

	// open gpio value attribute
	sprintf(buffer, "%s%s%s", SW_PREFIX, pin, "/value");
	//printf("Opening %s\n", buffer);

	f = open (buffer, O_RDWR);
	if(f < 0){
		perror("Open for reading");
		exit(-1);
	}

	// Make sure the select will block
	read(f, buffer, 10);
	lseek(f, 0, SEEK_SET);
	
	return f;
}

void body()
{
	//variable to communicate with the driver
	int ret=0;	//return state value
	int duty=0; 	//duty-cycle
	int mode=0;	//auto-manual
	int temp=0;	//temperature

	//open gpio, led and switch
	syslog (LOG_INFO, "Open GPIO in-out");
	int sw1_fd = open_gpio(SW1,"in");
	int sw2_fd = open_gpio(SW2,"in");
	int sw3_fd = open_gpio(SW3,"in");
	int led1_fd = open_gpio(LED1,"out");
	int led2_fd = open_gpio(LED2,"out");
	int led3_fd = open_gpio(LED3,"out");

	//open the fand-ctrl file
	syslog (LOG_INFO, "Open driver");
	int fmode = open("/sys/devices/platform/fan-ctrl/mode",O_RDWR); //auto-manual
	int fduty = open("/sys/devices/platform/fan-ctrl/duty",O_RDWR); //0-100
	int ftemp = open("/sys/devices/platform/fan-ctrl/temp",O_RDONLY); //read temperature milli
	
	//configure driver
	char buf[20];

	/*ret = read(fmode,buf,1);
	ret = read(fduty,buf,6);
	duty = atoi(buf);
	ret = read(ftemp,buf,1);
	temp = atoi(buf);*/

	//force current configuration
	ret = write(fmode,"auto",strlen("auto"));
	mode = 1; //next mode is manual so
	ret = write(fduty,"50",strlen("50"));
	duty = 50;

	//configure user interface LED
	write (led1_fd, "0", strlen("0")); //set mode auto
	write (led2_fd, "0", strlen("0"));
	write (led3_fd, "0", strlen("0"));

	//open a socket ip ipc
	syslog (LOG_INFO, "Create socket for remote connection");
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
	
	//timer configuration
	int timfd, temfd;
	struct itimerspec its, tts;

	syslog (LOG_INFO, "Create timer for LED clear");
	timfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);

	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 400000000;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	syslog (LOG_INFO, "Create timer for temperature polling");
	//temfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);

	tts.it_value.tv_sec = 2;
	tts.it_value.tv_nsec = 0;
	tts.it_interval.tv_sec = 2;
	tts.it_interval.tv_nsec = 0;

	//creat epoll and link with to events
	int epfd;
	struct epoll_event ev, events;

	syslog (LOG_INFO, "Create epoll");
	epfd = epoll_create1(EPOLL_CLOEXEC);

	ev.events = EPOLLIN;	//timer events link
	ev.data.fd = timfd;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, timfd, &ev);
	/*ev.events = EPOLLIN;
	ev.data.fd = temfd;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, temfd, &ev);*/
	
	ev.events = EPOLLPRI;	//switch events link
	ev.data.fd = sw1_fd;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sw1_fd, &ev);
	ev.events = EPOLLPRI;
	ev.data.fd = sw2_fd;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sw2_fd, &ev);
	ev.events = EPOLLPRI;
	ev.data.fd = sw3_fd;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sw3_fd, &ev);

	ev.events = EPOLLIN | EPOLLET;	//socket events link
	ev.data.fd = sfd;
	ret = epoll_ctl (epfd, EPOLL_CTL_ADD, sfd, &ev);
	
	//initialize the switch by reading first
	int dummy[8];

	read(sw1_fd,&dummy,sizeof(dummy));
	lseek(sw1_fd, 0, SEEK_SET);
	read(sw2_fd,&dummy,sizeof(dummy));
	lseek(sw2_fd, 0, SEEK_SET);
	read(sw3_fd,&dummy,sizeof(dummy));
	lseek(sw3_fd, 0, SEEK_SET);

	//launch the time to poll the temperature from the driver
	//timerfd_settime(temfd, 0, &tts, NULL);

	while(1){
		ret = epoll_wait(epfd, &events, 10, -1);
		if(events.data.fd == sw1_fd){
			// flush the event
			read(sw1_fd,&dummy,sizeof(dummy));
			lseek(sw1_fd, 0, SEEK_SET);

			if(mode == 0){ //skip if mode is auto
				//signal led
				write (led1_fd, "1", sizeof("1"));

				duty+=10;	//on press increase the speed by 10%
				if(duty > 100)
				duty = 100;
				sprintf(buf,"%d",duty);
				write(fduty,buf,sizeof(buf));
			}

			timerfd_settime(timfd, 0, &its, NULL);			
		}
		if(events.data.fd == sw2_fd){
			// flush the event
			read(sw2_fd,&dummy,sizeof(dummy));
			lseek(sw2_fd, 0, SEEK_SET);

			if(mode == 0){ //skip if mode is auto
				//signal led
				write (led2_fd, "1", sizeof("1"));

				duty-=10;	//on press decrease the speed by 10%
				if(duty < 0)
				duty = 0;
				sprintf(buf,"%d",duty);
				write(fduty,buf,sizeof(buf));
			}

			timerfd_settime(timfd, 0, &its, NULL);			
		}
		if(events.data.fd == sw3_fd){
			// flush the event
			read(sw3_fd,&dummy,sizeof(dummy));
			lseek(sw3_fd, 0, SEEK_SET);

			//signal led
			if(mode==0){
				mode=1;
				write (led3_fd, "0", 1);
				write(fmode,"auto\n",strlen("auto\n"));
			}
			else{
				mode=0;
				write (led3_fd, "1", 1);
				write(fmode,"manual\n",strlen("manual\n"));
				sprintf(buf,"%d",duty);
				write(fduty,buf,sizeof(buf)); //refresh the duty to current
			}

			//timerfd_settime(timfd, 0, &its, NULL);			
		}
		if(events.data.fd == timfd){
			// flush the event
			read(timfd,&dummy,sizeof(dummy));
			lseek(timfd, 0, SEEK_SET);
			
			//clear leds
			write (led1_fd, "0", sizeof("0"));
			write (led2_fd, "0", sizeof("0"));
		}
		if(events.data.fd == temfd){
			// flush the event
			read(temfd,&dummy,sizeof(dummy));
			//lseek(temfd, 0, SEEK_SET);
			
			ret = read(ftemp,buf,sizeof(buf)); //read temperature
			lseek(ftemp, 0, SEEK_SET);
			temp = atoi(buf)/1000;

			//printf("timer, temp%d\n",temp);
			//timerfd_settime(temfd, 0, &tts, NULL); //reinitialize the timer
		}
		//server part
		if(events.data.fd == sfd){
			//we have a notification on the listening socket, which
			//means one or more incoming connections. 
			while (1){
				struct sockaddr in_addr;
				socklen_t in_len;
				int infd;
				char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

				in_len = sizeof in_addr;
				infd = accept (sfd, &in_addr, &in_len);
				if (infd == -1){
					if ((errno == EAGAIN) ||
							(errno == EWOULDBLOCK)){
						//We have processed all incoming
						//connections.
						break;
					}
					else{
						perror ("accept");
						break;
					}
				}

				ret = getnameinfo (&in_addr, in_len,
				hbuf, sizeof hbuf,
				sbuf, sizeof sbuf,
				NI_NUMERICHOST | NI_NUMERICSERV);
				if (ret == 0){
					printf("Accepted connection on descriptor %d "
					"(host=%s, port=%s)\n", infd, hbuf, sbuf);
				}

				//Make the incoming socket non-blocking and add it to the
				//list of fds to monitor.
				ret = make_socket_non_blocking (infd);
				if (ret == -1)
					abort ();

				ev.data.fd = infd;
				ev.data.u32 = SOCKETU32;	//configure all sockets to be recognized
				ev.events = EPOLLIN | EPOLLET;
				ret = epoll_ctl (epfd, EPOLL_CTL_ADD, infd, &ev);
				if (ret == -1){
					perror ("epoll_ctl");
					abort ();
				}
			}
			continue;
		}
		//socket part (on client sending data)
		if(events.data.u32 == SOCKETU32){
			int done = 0;

			while (1){
				ssize_t count;
				char buf[512] = "";

				count = read (events.data.fd, buf, sizeof buf);
				if (count == -1){
					//if errno == EAGAIN, that means we have read all
					//data. So go back to the main loop.
					if (errno != EAGAIN){
						perror ("read socket");
						done = 1;
					}
					break;
				}
				else if (count == 0){
					//end of file. The remote has closed the
					//connection.
					done = 1;
					break;
				}
				
				//write the buffer to standard output
				/*ret = write (1, buf, strlen(buf));
				if (ret == -1){
					perror ("write");
					abort ();
				}*/

				char *cmd = buf;
				char *data = NULL;
				char response[512];
				int success=0;

				printf("buf: %s cmd: %s\n",buf, cmd);
				
				//decode the command
				if(strstr(buf,"=") != NULL){
					cmd = strtok(buf,"=");
					data = strtok(NULL,"=");
					printf("cmd: %s data: %s\n",cmd,data);
				}
				
				for(int i=0;-1!=protocolcmd[i].index;++i){
					if(strcmp(cmd,protocolcmd[i].cmd) == 0){
						success = 1;
						switch(protocolcmd[i].index){
						case 1:{	//show
								char *mmode = "manual";
								if(mode)
								mmode = "auto";

								ret = read(ftemp,buf,sizeof(buf)); //read temperature
								lseek(ftemp, 0, SEEK_SET);
								temp = atoi(buf)/1000;
								
								sprintf(response,"mode=[%s],duty=[%d],temp=[%d]\n",
								mmode,duty,temp);
								write(events.data.fd,response,strlen(response));
							}
							break;
						case 2: //mode
							if(data != NULL){
								int tmode;
								if((tmode = strcmp(data,"auto\n")) == 0 || strcmp(data,"manual\n") == 0){
									if(tmode == 0){
										write(led3_fd, "0", 1);
										mode = 1;
									}
									else{
										write(led3_fd, "1", 1);
										mode = 0;
									}
									write(fmode,data,strlen(data));
									sprintf(buf,"%d",duty);
									write(fduty,buf,sizeof(buf));
									write(events.data.fd,data,strlen(data));
								}
								else{
									char *msg = "Error mode not defined\n";
									write(events.data.fd,msg,strlen(msg));
								}
							}
							else{
								char *msg = "Missing parameter\n";
								write(events.data.fd,msg,strlen(msg));
							}
							break;
						case 3: //duty
							if(data != NULL){
								int tduty = atoi(data);
								if(tduty >= 0 && tduty <= 100 && mode==0){
									duty = tduty;
									sprintf(response,"Duty Cycle changed to %d\n",duty);
									write(fduty,data,sizeof(data));
									write(events.data.fd,response,strlen(response));
								}
								else{
									char *msg = "Error duty cycle range\n";
									write(events.data.fd,msg,strlen(msg));
								}
							}
							else{
								char *msg = "Missing parameter\n";
								write(events.data.fd,msg,strlen(msg));
							}
							break;
						case 4:
							sprintf(response,"-mode=auto,manual set the mode\n-duty=0-100 set the duty cycle\n-show show the current status\n");
							write(events.data.fd,response,strlen(response));
							break;
						}
						break;
					}
				}
				if(!success){
					char *msg = "Error in the command\n";
					write(events.data.fd,msg,strlen(msg));
				}
			}

			if (done){
				printf ("Closed connection on descriptor %d\n",
				events.data.fd);
				//Closing the descriptor will make epoll remove it
				//from the set of descriptors which are monitored.
				close (events.data.fd);
			}
		}
	}
}

int main(int argc, char** argv)
{
	// 1. fork off the parent process
	fork_process();

	// 2. create new session
	if (setsid() == -1) {
		syslog (LOG_ERR, "ERROR while creating new session");
		exit (1);
	}

	// 3. fork again to get rid of session leading process
	fork_process();

	// 4. capture all required signals
	struct sigaction act; //= {.sa_handler = catch_signal,};
	act.sa_handler = catch_signal;
	sigaction (SIGHUP,  &act, NULL);  //  1 - hangup
	sigaction (SIGINT,  &act, NULL);  //  2 - terminal interrupt
	sigaction (SIGQUIT, &act, NULL);  //  3 - terminal quit
	sigaction (SIGABRT, &act, NULL);  //  6 - abort
	sigaction (SIGTERM, &act, NULL);  // 15 - termination
	sigaction (SIGTSTP, &act, NULL);  // 19 - terminal stop signal

	// 5. update file mode creation mask
	umask(0027);

	// 6. change working directory to appropriate place
	if (chdir ("/") == -1) {
		syslog (LOG_ERR, "ERROR while changing to working directory");
		exit (1);
	}

	// 7. close all open file descriptors
	for (int fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--) {
		close (fd);
	}

	// 8. redirect stdin, stdout and stderr to /dev/null
	if (open ("/dev/null", O_RDWR) != STDIN_FILENO) {
		syslog (LOG_ERR, "ERROR while opening '/dev/null' for stdin");
		exit (1);
	}
	if (dup2 (STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) {
		syslog (LOG_ERR, "ERROR while opening '/dev/null' for stdout");
		exit (1);
	}
	if (dup2 (STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) {
		syslog (LOG_ERR, "ERROR while opening '/dev/null' for stderr");
		exit (1);
	}

	// 9. option: open syslog for message logging
	openlog (NULL, LOG_NDELAY | LOG_PID, LOG_DAEMON);
	syslog (LOG_INFO, "Daemon has started...");

	// 10. option: get effective user and group id for appropriate's one
	struct passwd* pwd = getpwnam ("root");
	if (pwd == 0) {
		syslog (LOG_ERR, "ERROR while reading daemon password file entry");
		exit (1);
	}

	// 11. option: change root directory
	if (chroot (".") == -1) {
		syslog (LOG_ERR, "ERROR while changing to new root directory");
		exit (1);
	}

	// 12. option: change effective user and group id for appropriate's one
	if (setegid (pwd->pw_gid) == -1) {
		syslog (LOG_ERR, "ERROR while setting new effective group id");
		exit (1);
	}
	if (seteuid (pwd->pw_uid) == -1) {
		syslog (LOG_ERR, "ERROR while setting new effective user id");
		exit (1);
	}
	
	// 13. launch the body function
	body();

	syslog (LOG_INFO, "daemon stopped. Number of signals catched=%d\n", signal_catched);
	closelog();

	return 0;
}