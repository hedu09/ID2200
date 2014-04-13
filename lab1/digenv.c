/* Include-rader ska vara allra först i ett C-program */
#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */


#define PIPE_READ_SIDE ( 0 ) /* Define the read side for a pipe to simplify */
#define PIPE_WRITE_SIDE ( 1 ) /* Define the write side for a pipe to simplify */

pid_t childpid; /* childProcessID for printenvp */
pid_t childpidSort; /* childProcessID for sort */
pid_t childpidLess; /* childProcessID for less */

int main(int argc, char **argv, char **envp)
{	
	printf("DEBUG: argc: %d\n", argc); /* Debug for argc */

	char *pagerEnv; /* char pointer for the inviorment varible for the pager*/
	pagerEnv = getenv("PAGER"); /* Get the page variable if it is set*/
	printf("DEBUG: Selected pager: %s\n", pagerEnv);
	if (NULL == pagerEnv) { /* Page enviorment isn't set*/
		pagerEnv = "less"; /* Set it to less*/
	}

	fprintf( stderr, "Parent (Parent, pid %ld) started\n",
		(long int) getpid() );

	int returnValue; /* Return value, to findout if an error occured */
	int status; /* Return codes for children */ 

	int pipe_fileDescPrintToSort[2]; /* File descriptiors for pipe, printEnvp to sort */
	int pipe_fileDescSortToLess[2]; /* File descriptiors for pipe, sort to less */

	if (argc == 2) /*  Grep */
	{
		printf("DEBUG: Run grep! \n");
		int pipe_fileDescGrep[2];  /* File descriptiors for pipe, used in Grep case */
		pid_t childpidGrep;  /* childProcessID for grep */
	}

	if (argc >= 3) /* Invalid input terminate! */
	{
		printf("To many input parameters! \n");
		exit(0); /* TODO: 0 or 1 ?? */
	}
	
	returnValue = pipe( pipe_fileDescPrintToSort ); /* Create a pipe */
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

		returnValue = dup2(pipe_fileDescPrintToSort[PIPE_WRITE_SIDE], STDOUT_FILENO);

		/* Close the read end of this pipe, child doesn't have to write data*/
		returnValue = close(pipe_fileDescPrintToSort[PIPE_READ_SIDE]);
		if (-1 == returnValue)
			{ perror("Cannot close pipe");	exit(1); }

		(void) execlp("printenv", "printenv", (char *) 0);
		perror("Cannot exec printenv");
		exit(1);
	}

	/* Close write side of pipe, it is not needed anymore */
	returnValue = close(pipe_fileDescPrintToSort[PIPE_WRITE_SIDE]);
	if ( -1 == returnValue) { perror("Cannot close pipe");	exit(1); }

	returnValue = pipe( pipe_fileDescSortToLess );
	if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }

	childpidSort = fork();
	if (-1 == childpidSort) /* Fork failed*/
	{
		char * errorMessage = "UNKNOWN";
		if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
		if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
		fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
		exit(1);
	}
	/* ---------------------------- Child sort ---------------------------- */
	else if (0 == childpidSort)
	{
		fprintf( stderr, "Child (sort, pid %ld) started\n",	
			(long int) getpid() );

		returnValue = dup2(pipe_fileDescPrintToSort[PIPE_READ_SIDE], STDIN_FILENO);
		returnValue = dup2(pipe_fileDescSortToLess[PIPE_WRITE_SIDE], STDOUT_FILENO);

		returnValue = close(pipe_fileDescSortToLess[PIPE_READ_SIDE]);
		if (-1 == returnValue)	{ perror("Cannot close pipe");	exit(1); }

		(void) execlp("sort", "sort", (char *) 0);
		perror("Cannot exec sort");
		exit(1);
	}
	returnValue = close(pipe_fileDescPrintToSort[PIPE_READ_SIDE]);
	if (-1 == returnValue)	{ perror("Cannot close pipe");	exit(1); }
	returnValue = close(pipe_fileDescSortToLess[PIPE_WRITE_SIDE]);
	if (-1 == returnValue)	{ perror("Cannot close pipe");	exit(1); }

	/*
	returnValue = pipe( pipe_fileDescSortToLess );
	if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); } */
	childpidLess = fork(); 
	if (-1 == childpidLess) /* Fork failed*/
	{
		char * errorMessage = "UNKNOWN";
		if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
		if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
		fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
		exit(1);
	}
	/* ---------------------------- Child less ---------------------------- */
	else if (0 == childpidLess)
	{
		fprintf( stderr, "Child (less, pid %ld) started\n",	
			(long int) getpid() );

		returnValue = dup2(pipe_fileDescSortToLess[PIPE_READ_SIDE], STDIN_FILENO);

		(void) execlp(pagerEnv, pagerEnv, (char *) 0);
		perror("Cannot exec pager");
		exit(1);
	}
	returnValue = close(pipe_fileDescSortToLess[PIPE_READ_SIDE]);
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

	childpidSort = wait( &status ); /* Vänta på ena child-processen */
	if ( -1 == childpidSort) { perror( "wait() failed unexpectedly" ); exit( 1 ); }

 	if( WIFEXITED( status ) ) /* child-processen har kört klart */
	{
		int child_status = WEXITSTATUS( status );
	    if( 0 != child_status ) /* child had problems */
		{
			fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
				(long int) childpidSort, child_status );
		}
	}
	else
	{
	    if( WIFSIGNALED( status ) ) /* child-processen avbröts av signal */
		{
			int child_signal = WTERMSIG( status );
			fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
				(long int) childpidSort, child_signal );
		}
	}
	
	childpidLess = wait( &status ); /* Vänta på ena child-processen */
	if ( -1 == childpidLess) { perror( "wait() failed unexpectedly" ); exit( 1 ); }

 	if( WIFEXITED( status ) ) /* child-processen har kört klart */
	{
		int child_status = WEXITSTATUS( status );
	    if( 0 != child_status ) /* child had problems */
		{
			fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
				(long int) childpidLess, child_status );
		}
	}
	else
	{
	    if( WIFSIGNALED( status ) ) /* child-processen avbröts av signal */
		{
			int child_signal = WTERMSIG( status );
			fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
				(long int) childpidLess, child_signal );
		}
	}
	exit(0); // Nomral terminate
}
