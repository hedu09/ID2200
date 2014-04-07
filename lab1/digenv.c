/* Include-rader ska vara allra först i ett C-program */
#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */


#define PIPE_READ_SIDE ( 0 ) 
#define PIPE_WRITE_SIDE ( 1 )

#define TRUE ( 1 ) /* definierar den Boolska konstanten TRUE */
#define FALSE ( 0 ) /* define the bool constant FALSE*/

pid_t childpid; /* för child-processens PID vid fork() */

int main(int argc, char **argv, char **envp)
{
	
	 fprintf( stderr, "Parent (Parent, pid %ld) started\n",
           (long int) getpid() );

	printf("argc: %d\n", argc);

	int pipe_fileDesc[2]; /* File descriptiors for pipe*/
	int returnValue; /* Return value, to findout if an error occured*/

	returnValue = pipe( pipe_fileDesc ); /* Create a pipe */

	if ( -1 == returnValue)
	{
		perror("Cannot create pipe");
		exit(1); 
	}

	childpid = fork(); /* Create the child process for printenv */ 
	if (0 == childpid)
	{

		fprintf( stderr, "Child (printenv, pid %ld) started\n",	
             (long int) getpid() );

		/* Close the write end of this pipe, child doesn't have to write data*/
		returnValue = close (pipe_fileDesc[1]);
		if (-1 == returnValue)
		{
			perror("Cannot close pipe");
			exit(1);
		}

		returnValue = read( pipe_fileDesc[0], &line, sizeof(line));
		if (argc == 1)
		{



		}
		else if (argc >= 2) // Grep!
		{
			/* code */
		}
		else{
			printf("ERROR! argc \n" );
			exit(1);
		}
	}

	else{
		// Parent
		if (-1 == childpid) // Kaboom
		{
			char * errorMessage = "UNKNOWN";

			if (EAGAIN == errno)
			{
				errorMessage = "cannot allocate page table";
			}

			if (ENOMEM == errno)
			{
				errorMessage = "cannot allocate kernel data";
			}

			fprintf(stderr, "fork() blew up becuse: %s\n", errorMessage);
			exit(1);
		}

		printf("Parent\n");
		// Parent code starts here

		/* Close the read end of the pipe, we only need to write to the child */
		returnValue = close (pipe_fileDesc[0]);
		if (-1 == returnValue)
		{
			perror("Parent cannot close read end");
			exit(1);
		}


		int i;
		for (i = 0; envp[i] != NULL; ++i)
		{
		
		printf("%2d:%s\n", i , envp[i]);
		returnValue = write( pipe_fileDesc[1], &envp[i], sizeof(envp[i]));

			if (-1 == returnValue)
			{
				perror("Parent cannot write to pipe");
				exit(1);
			}
		}
	}
	exit(0); // Nomral terminate
}