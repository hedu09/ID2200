#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */
#include <string.h> /* Define strcmp */
#include <sys/wait.h> /* Prevent gcc -Wall errors in Mac */

/* Function declarations */
void createChild (char**);
void childHandler();

pid_t childpid; /* childProcessID for printenvp */
int status; /* Return codes for children */

#define BUFFERSIZE (81) /* Define maximum size of the buffer, assumption from lab specification */
#define ARGVSIZE (6) /* Define maximum size of the ARGC, assumption from lab specification */

/* createChild will create a child that will execute a command and change STDOUT to be sent to the write pipe and STDIN sent to read pipe.
@Param	**input	- Input arguments to be run, that has been feed into the shell. */
void createChild(char **input)
{
	childpid = fork();
	if (-1 == childpid)
	{
		char * errorMessage = "UNKNOWN"; /* if no known error message print UNKNOWN*/
		if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
		if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
		fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
		exit(1);/* Exit with error*/
	}
	else if(0 == childpid)/* Child returns 0 */
	{
		fprintf( stderr, "Child (%s, pid %ld) started\n", input[0], (long int) getpid() );
		(void) execvp(input[0], input); /* Execute command */
		perror("Cannot exec perror");
		exit(1); /*exit with error*/
	}
}
/* Handles the child when the child has terminated by signal or died with or without error
we need to handle the child when it dies or it will become a zombie */
void childHandler()
{
	childpid = wait( &status ); /* waiting on a child process */
	if ( -1 == childpid) { perror( "wait() failed unexpectedly" ); exit( 1 ); }/* Check if wait works correctly*/

 	if( WIFEXITED( status ) ) /* Check if child has terminated normally or by signal */
	{
		int child_status = WEXITSTATUS( status ); /*get the exit code specified by the child process*/
		if( 0 != child_status ) /* child had problems */
		{
			fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
				(long int) childpid, child_status );
		}
	}
	else
	{
		if( WIFSIGNALED( status ) ) /* child-process terminated by signal */
		{
			int child_signal = WTERMSIG( status ); /*get the exit code specified by the child process*/
			fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
				(long int) childpid, child_signal );
		}
	}	
}

int main(int argc, char **argv)
{	
	fprintf( stderr, "Parent (Parent, pid %ld) started\n", (long int) getpid() );  /* printing parents id for easier debuggning*/

	char inputBuffer[BUFFERSIZE]; /* Declare input buffer for command, hard code to Stack */
	char **arguments = (char **) calloc (ARGVSIZE,  BUFFERSIZE); /* Allocate memory for toknizer */
	
	while(1){ /* Loop forever */
		printf("DEBUG: Start of loop\n");
		fgets(inputBuffer, BUFFERSIZE , stdin); /* Read command from terminal */
		inputBuffer[strlen(inputBuffer)-1]= '\0'; /* Remove the newline char and replace it with null */ 
		/*
		if (strcmp("\n", inputBuffer) == 0)
		{
			printf("Say again?\n");
			continue;
		}
		*/
		char *arg = strtok(inputBuffer, " "); /* Split the input on space */

		if (strcmp( "exit", arg) == 0) /* Exit!*/
		{
			free(arguments); /* Relese memory */
			printf("Thank you come again!\n");
			exit(0);
		}
		
		int i = 0;
		while( arg != NULL){ /* Read until NULL */ 
			arguments[i] = arg; /* Point to the input */
			printf("DEBUG: input<:%s:>\n", arguments[i]);
			arg = strtok(NULL, " "); /* Move on */
			i++;
		}

		createChild(arguments);
		childHandler();
		printf("DEBUG: End of loop\n");
	}

	free(arguments); /* Relese memory */
	exit(0);
}
