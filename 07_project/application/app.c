#include <stdio.h>
#include <string.h>


static const char help_text[][100] = 
{
	"\n",
	"Fan controller remote-control\n",
	"=============================\n",
	"\n",
	"Commands are:\n",
	"\n",
	" help            : Show this message & exit\n",
	" mode [auto|man] : Set the control mode\n",
	" duty [0-100]    : Set the duty-cycle\n",
	" show            : Show current temperature, duty and mode\n",
	" exit            : Exit this application\n",
	"\n"
};

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
	printf("Duty setted to %d\n", duty);
}

void set_mode(const char* mode)
{
	printf("Mode setted to %s\n", mode);
}

void show()
{

}

int main()
{
	char input_buffer[100];
	int duty_work;
	char mode_work[15];

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
			if(sscanf(input_buffer+5, "%s", &mode_work) == 1)
			{
				// Parsing OK, check that the value is legal
				if (strcmp(mode_work, "auto") != 0  && strcmp(mode_work, "man") != 0)
				{
					printf("Error: Mode must be one of \"auto\" or \"man\"!\n");
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