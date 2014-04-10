/* Include-rader ska vara allra först i ett C-program */
#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */


#define PIPE_READ_SIDE ( 0 ) /* Define the read side for a pipe to simplify */
#define PIPE_WRITE_SIDE ( 1 ) /* Define the write side for a pipe to simplify */

#define TRUE ( 1 ) /* definierar den Boolska konstanten TRUE */
#define FALSE ( 0 ) /* define the bool constant FALSE*/

pid_t childpid; /* för child-processens PID vid fork() */
pid_t childpid2;
pid_t childpid3;

int main(int argc, char **argv, char **envp)
{	
	fprintf( stderr, "Parent (Parent, pid %ld) started\n",
           (long int) getpid() );

	printf("argc: %d\n", argc);

	int pipe_fileDesc[2]; /* File descriptiors for pipe */
	int pipe_fileDesc2[2];
	//int pipe_fileDesc3[2]; /*  Grep */

	int returnValue; /* Return value, to findout if an error occured */
	int status; /* Return codes for children */ 

	returnValue = pipe( pipe_fileDesc ); /* Create a pipe */
	if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }

	childpid = fork(); /* Create the child process for printenv */ 
	if (-1 == childpid) /* Fork failed*/
		{
			char * errorMessage = "UNKNOWN";
			if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
			if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
			fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
			exit(1);
		}
	/* ---------------------------- Child printEnv ---------------------------- */
	if (0 == childpid)
	{
		fprintf( stderr, "Child (printenv, pid %ld) started\n",	
             (long int) getpid() );

		returnValue = dup2(pipe_fileDesc[PIPE_WRITE_SIDE], STDOUT_FILENO);

		/* Close the read end of this pipe, child doesn't have to write data*/
		returnValue = close(pipe_fileDesc[PIPE_READ_SIDE]);
		if (-1 == returnValue)
		{ perror("Cannot close pipe");	exit(1); }

		(void) execlp("printenv", "printenv", (char *) 0);
		perror("Cannot exec printenv");
		exit(1);
	}
	returnValue = close(pipe_fileDesc[PIPE_WRITE_SIDE]);
    if ( -1 == returnValue) { perror("Cannot close pipe");	exit(1); }

	returnValue = pipe( pipe_fileDesc2 );
	if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }

	childpid2 = fork();
	if (-1 == childpid2) /* Fork failed*/
		{
			char * errorMessage = "UNKNOWN";
			if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
			if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
			fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
			exit(1);
		}
	/* ---------------------------- Child sort ---------------------------- */
	else if (0 == childpid2)
	{
		fprintf( stderr, "Child (sort, pid %ld) started\n",	
             (long int) getpid() );

		returnValue = dup2(pipe_fileDesc[PIPE_READ_SIDE], STDIN_FILENO);
		returnValue = dup2(pipe_fileDesc2[PIPE_WRITE_SIDE], STDOUT_FILENO);

		returnValue = close(pipe_fileDesc2[PIPE_READ_SIDE]);
		if (-1 == returnValue)	{ perror("Cannot close pipe");	exit(1); }

		(void) execlp("sort", "sort", (char *) 0);
		perror("Cannot exec printenv");
		exit(1);
	}
	returnValue = close(pipe_fileDesc[PIPE_READ_SIDE]);
	if (-1 == returnValue)	{ perror("Cannot close pipe");	exit(1); }
	returnValue = close(pipe_fileDesc2[PIPE_WRITE_SIDE]);
	if (-1 == returnValue)	{ perror("Cannot close pipe");	exit(1); }

	/*
	returnValue = pipe( pipe_fileDesc2 );
	if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); } */
	childpid3 = fork(); 
	if (-1 == childpid3) /* Fork failed*/
		{
			char * errorMessage = "UNKNOWN";
			if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
			if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
			fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
			exit(1);
		}
	/* ---------------------------- Child less ---------------------------- */
	else if (0 == childpid3)
	{
		fprintf( stderr, "Child (less, pid %ld) started\n",	
             (long int) getpid() );

		returnValue = dup2(pipe_fileDesc2[PIPE_READ_SIDE], STDIN_FILENO);

		(void) execlp("less", "less", (char *) 0);
		perror("Cannot exec printenv");
		exit(1);
	}
	returnValue = close(pipe_fileDesc2[PIPE_READ_SIDE]);
    if (-1 == returnValue)	{ perror("Cannot close pipe");	exit(1); }

/* ---------------------------- Parent ---------------------------- */
	
 	childpid = wait( &status ); /* Vänta på ena child-processen */
 	if ( -1 == childpid) { perror( "wait() failed unexpectedly" ); exit( 1 ); }

 	if( WIFEXITED( status ) ) /* child-processen har kört klart */
  	{
    	int child_status = WEXITSTATUS( status );
	    if( 0 != child_status ) /* child had problems */
	    {
	      fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
	               (long int) childpid, child_status );
	    }
  	}
  	else
  	{
	    if( WIFSIGNALED( status ) ) /* child-processen avbröts av signal */
	    {
	      int child_signal = WTERMSIG( status );
	      fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
	               (long int) childpid, child_signal );
	    }
	}

	childpid2 = wait( &status ); /* Vänta på ena child-processen */
 	if ( -1 == childpid2) { perror( "wait() failed unexpectedly" ); exit( 1 ); }

 	if( WIFEXITED( status ) ) /* child-processen har kört klart */
  	{
    	int child_status = WEXITSTATUS( status );
	    if( 0 != child_status ) /* child had problems */
	    {
	      fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
	               (long int) childpid2, child_status );
	    }
  	}
  	else
  	{
	    if( WIFSIGNALED( status ) ) /* child-processen avbröts av signal */
	    {
	      int child_signal = WTERMSIG( status );
	      fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
	               (long int) childpid2, child_signal );
	    }
	}

	
	childpid3 = wait( &status ); /* Vänta på ena child-processen */
 	if ( -1 == childpid3) { perror( "wait() failed unexpectedly" ); exit( 1 ); }

 	if( WIFEXITED( status ) ) /* child-processen har kört klart */
  	{
    	int child_status = WEXITSTATUS( status );
	    if( 0 != child_status ) /* child had problems */
	    {
	      fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
	               (long int) childpid3, child_status );
	    }
  	}
  	else
  	{
	    if( WIFSIGNALED( status ) ) /* child-processen avbröts av signal */
	    {
	      int child_signal = WTERMSIG( status );
	      fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
	               (long int) childpid3, child_signal );
	    }
	}

	exit(0); // Nomral terminate
}
