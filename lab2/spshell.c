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

int main(int argc, char **argv)
{	
	fprintf( stderr, "Parent (Parent, pid %ld) started\n", (long int) getpid() );  /* printing parents id for easier debuggning*/

	char inputBuffer[BUFFERSIZE]; /* Declare input buffer for command, hard code to Stack */

	fgets(inputBuffer, BUFFERSIZE , stdin); /* Read command from terminal */

	printf("DEBUG: input: %s\n", inputBuffer);

	exit(0); // Normal terminate
}
