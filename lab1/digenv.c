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

		returnValue = dup2(pipe_fileDesc[ PIPE_READ_SIDE ], STDIN_FILENO);

		/* Close the write end of this pipe, child doesn't have to write data*/
		returnValue = close (pipe_fileDesc[PIPE_WRITE_SIDE]);
		if (-1 == returnValue)
		{
			perror("Cannot close pipe");
			exit(1);
		}

		if (argc == 1) /* Directly to sort */
		{
			(void) execlp("sort", "sort", (char *) 0);
			perror("Cannot exec sort");
			exit(1);
		}
		else if (argc >= 2) /* Grep first!*/
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

			fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
			exit(1);
		}

		// Parent code starts here
		returnValue = dup2(pipe_fileDesc[PIPE_WRITE_SIDE], STDOUT_FILENO);
		if (-1 == returnValue)
		{
			perror("Parent cannot dup2 write end");
			exit(1);
		}

		/* Close the read end of the pipe, we only need to write to the child */
		returnValue = close (pipe_fileDesc[PIPE_READ_SIDE]);
		if (-1 == returnValue)
		{
			perror("Parent cannot close read end");
			exit(1);
		}

		int i;
		for (i = 0; envp[i] != NULL; ++i)
		{
			char newLine = '\n';
			//int test = sizeof(**envp[i]);
			/* printf("DEBUG: %2d:%s\n", i , envp[i]); */
			returnValue = write(pipe_fileDesc[PIPE_WRITE_SIDE], *&envp[i], sizeof(envp[i]));
			//printf("DEBUG: Size of envp: %d\n", **envp[i]);
			returnValue = write(pipe_fileDesc[PIPE_WRITE_SIDE], &newLine, sizeof(newLine));

			if (-1 == returnValue)
				{
					perror("Parent cannot write to pipe");
					exit(1);
				}
			//printf("%s\n", envp[i]);
		} 
	}
	exit(0); // Nomral terminate
}