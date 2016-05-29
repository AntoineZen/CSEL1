#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>


static const char help_text[][100] = 
{
	"\n",
	"Fan controller remote-control\n",
	"=============================\n",
	"\n",
	"Commands are:\n",
	"\n",
	" help                  : Show this message & exit\n",
	" mode [auto|manual]    : Set the control mode\n",
	" duty [0-100]          : Set the duty-cycle\n",
	" show                  : Show current temperature, duty and mode\n",
	" exit                  : Exit this application\n",
	"\n"
};
 
static int socket_fd;

void print_help(void)
{
	// Print the help text
	for(unsigned int i = 0; i < sizeof(help_text)/100; i++)
	{
		printf(help_text[i]);
	}

}

void set_duty(int duty)
{
	char buffer[64];
	// set command text
	sprintf(buffer, "duty=%d\n", duty);
	// Sent the command
	write(socket_fd, buffer, strlen(buffer));
	// Clear buffer for read
	bzero(buffer, 64);
	// Read answer
	read(socket_fd, buffer, 64);
	// Display answer
	printf(buffer);
}

void set_mode(const char* mode)
{
	char buffer[64];
	// set command text
	sprintf(buffer, "mode=%s\n", mode);
	// Sent the command
	write(socket_fd, buffer, strlen(buffer));
	// Clear buffer for read
	bzero(buffer, 64);
	// Read answer
	read(socket_fd, buffer, 64);
	// Display answer
	printf(buffer);
}

void show()
{
	char buffer[64];
	// Clear read buffer
	bzero(buffer, 64);
	// Sent the command
	write(socket_fd, "show\n", strlen("show\n"));
	// Read answer
	read(socket_fd, buffer, 64);
	// Displa answer
	printf(buffer);
}

int open_socket()
{
	// Socket address
	struct sockaddr_in serv_addr;
	// Host name
	struct hostent *server;

	// Create a socket file descriptor
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);	
	if (sockfd < 0) 
	{
		fprintf(stderr,"ERROR opening socket");
		return -1;
	}

	// create a host object for host name (here an IP)
	server = gethostbyname("localhost");
	if (server == NULL) 
	{
		fprintf(stderr,"ERROR, no such host\n");
		return -1;
	}

	// Copy server address and port to the socket address structure
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(8080);

	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
        fprintf(stderr,"ERROR connecting\n");
        return -1;
	}

    return sockfd;
}



int main()
{
	char input_buffer[100];
	int duty_work;
	char mode_work[15];

	socket_fd = open_socket();

	print_help();

	while(1)
	{
		// Display the prompt
		fputs("> ", stdout);
		// Get user input
		fgets(input_buffer, sizeof(input_buffer), stdin);

		// Remove trailing new line
		input_buffer[strlen(input_buffer)-1] = '\0';

		// look for the command
		if( strcmp(input_buffer, "exit") == 0)
		{
			break;
		}
		else if( strcmp(input_buffer, "help") == 0)
		{
			// Print help text
			print_help();
		}
		else if( strstr(input_buffer, "mode") != NULL)
		{
			// Try to parse the mode argument
			if(sscanf(input_buffer+5, "%s", mode_work) == 1)
			{
				// Parsing OK, check that the value is legal
				if (strcmp(mode_work, "auto") != 0  && strcmp(mode_work, "manual") != 0)
				{
					printf("Error: Mode must be one of \"auto\" or \"manual\"!\n");
					continue;
				}
				// All ok, set the mode
				set_mode(mode_work);
			}
			else
			{
				printf("Error: Unkown value for mode \"%s\"!\n", input_buffer+5);
			}
		}
		else if( strstr(input_buffer, "duty") != NULL)
		{
			// try the parset the duty argument
			if( sscanf(input_buffer+5, "%d", &duty_work) == 1)
			{
				// check that the duty value is alowed
				if( duty_work > 100 || duty_work < 0)
				{
					printf("Error: Duty-cycle must be in range 0-100!\n");
					continue;
				}
				// Set the duty
				set_duty(duty_work);
			}
			else
			{
				printf("Error: Unkown value for duty \"%s\"!\n", input_buffer+5);
			}
		}
		else if( strcmp(input_buffer, "show") == 0)
		{
			// Do the show command
			show();
		}
		else
		{
			// command was not understood.
			printf("Error: Unknown command! Type \"help\" for help.\n");
		}
	}

	printf("Bye!\n");
	return 0;
}