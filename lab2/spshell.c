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

int main(int argc, char **argv)
{	


	exit(0); // Normal terminate
}
