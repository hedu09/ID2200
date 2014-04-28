#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */
#include <string.h> /* Define strcmp */
#include <sys/wait.h> /* Prevent gcc -Wall errors in Mac */

pid_t childpid; /* childProcessID for printenvp */
int returnValue; /* Return value, to findout if an error occured */
int status; /* Return codes for children */

#define BUFFERSIZE (81) /* Define maximum size of the buffer, assumption from lab specification */
#define ARGVSIZE (6) /* Define maximum size of the ARGC, assumption from lab specification */

void createChild(char command[])
{
	childpid = fork();
	if ( -1 == childpid)
	{
		char * errorMessage = "UNKNOWN"; /* if no known error message print UNKNOWN*/
		if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
		if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
		fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
		exit(1);/* Exit with error*/
	}
	else
	{
		(void) execvp(command[0], arguments); /* Execute command */
	}
}
void childHandler()
{

}
int main(int argc, char **argv)
{	
	fprintf( stderr, "Parent (Parent, pid %ld) started\n", (long int) getpid() );  /* printing parents id for easier debuggning*/

	char inputBuffer[BUFFERSIZE]; /* Declare input buffer for command, hard code to Stack */
	char **arguments = (char **) calloc (ARGVSIZE,  BUFFERSIZE); /* Allocate memory for toknizer */
	
	fgets(inputBuffer, BUFFERSIZE , stdin); /* Read command from terminal */
	inputBuffer[strlen(inputBuffer)-1]= '\0';
	
	
	char *arg = strtok(inputBuffer, " "); /* Split the input on space */
	
	int i = 0;
	while( arg != NULL){ /* Read until NULL */ 
		arguments[i] = arg; /* Point to the input */
		printf("DEBUG: input:%s:\n", arguments[i]);
		arg = strtok(NULL, " "); /* Move on */
		i++;
	}

	createChild(arguments[0]);
	childHandler();

	perror("Cannot exec perror");
	exit(1); // error
}
