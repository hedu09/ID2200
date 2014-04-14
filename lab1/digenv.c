/* Include-rader ska vara allra först i ett C-program */
#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */


#define PIPE_READ_SIDE ( 0 ) /* Define the read side for a pipe to simplify */
#define PIPE_WRITE_SIDE ( 1 ) /* Define the write side for a pipe to simplify */

#define NO_READ ( 0 ) /* Define the read side for a pipe to simplify */
#define READWRITE_CLOSE ( 1 ) /* Define the write side for a pipe to simplify */
#define READWRITE_NO_CLOSE ( 2 ) /* Define the write side for a pipe to simplify */

pid_t childpid; /* childProcessID for printenvp */
pid_t childpidSort; /* childProcessID for sort */
pid_t childpidLess; /* childProcessID for less */
pid_t childpidGrep;  /* childProcessID for grep */

int returnValue; /* Return value, to findout if an error occured */

void createChild(int pipe_readfiledesc[2], int pipe_writefiledesc[2], char command[], int read, int write, char **argv){
	
	childpid = fork();
	if (-1 == childpid) /* Fork failed*/
	{
		char * errorMessage = "UNKNOWN";
		if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
		if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
		fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
		exit(1);
	}

	/* ---------------------------- Child ---------------------------- */
	else if (0 == childpid)
	{
		fprintf( stderr, "Child (%s, pid %ld) started\n", command,	
			(long int) getpid() );
		if (read >= 1){
			returnValue = dup2(pipe_readfiledesc[PIPE_READ_SIDE], STDIN_FILENO);
			if (read == 1){
				returnValue = close(pipe_readfiledesc[PIPE_WRITE_SIDE]);
				if (-1 == returnValue)	{ perror("Cannot close pipe RFD");	exit(1); }
			}
		}
		if (write >= 1){
			returnValue = dup2(pipe_writefiledesc[PIPE_WRITE_SIDE], STDOUT_FILENO);	
			if( write == 1){
				returnValue = close(pipe_writefiledesc[PIPE_READ_SIDE]);
				if (-1 == returnValue)	{ perror("Cannot close pipeWFD");	exit(1); }
			}	
		}
		
		if (command == "grep")
		{
			(void) execlp("grep", "grep", argv[1] , (char *) 0);	
		}
		else
		{
			(void) execlp(command, command, (char *) 0);
		}
		perror("Cannot exec perror roar! row48");
		exit(1);
	}
	/*parent kör här */
	if (read >= 1){ 
		returnValue = close(pipe_readfiledesc[PIPE_READ_SIDE]);
		if (-1 == returnValue)	{ perror("Cannot close pipe Parent R");	exit(1); }
	}
	if (write >= 1){
		returnValue = close(pipe_writefiledesc[PIPE_WRITE_SIDE]);
		if (-1 == returnValue)	{ perror("Cannot close pipe Parent W");	exit(1); }
	}
}


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


	int status; /* Return codes for children */ 

	int pipe_fileDesc[2]; /* File descriptiors for pipe, printEnvp to sort */
	int pipe_fileDesc2[2];  /* File descriptiors for pipe, used in Grep case */
	int pipe_fileDesc3[2]; /* File descriptiors for pipe, sort to less */

	if (argc == 2) /*  Grep */
{
	printf("DEBUG: Run grep! \n");
}

	if (argc >= 9000) /* Invalid input terminate! */
{
	printf("To many input parameters! \n");
		exit(0); /* TODO: 0 or 1 ?? */
}

	returnValue = pipe( pipe_fileDesc); /* Create a pipe */
if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }

if (argc == 1){
	createChild( pipe_fileDesc, pipe_fileDesc,  "printenv", NO_READ, READWRITE_CLOSE, argv);
}
else if (argc >= 2)
{
	returnValue = pipe(pipe_fileDesc3);
	if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }

	createChild(pipe_fileDesc, pipe_fileDesc3,  "printenv", NO_READ, READWRITE_CLOSE, argv);
	createChild(pipe_fileDesc3, pipe_fileDesc, "grep", READWRITE_NO_CLOSE, READWRITE_NO_CLOSE, argv);
}

	returnValue = pipe( pipe_fileDesc2); /* Create a pipe */
if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }

createChild( pipe_fileDesc, pipe_fileDesc2, "sort", READWRITE_CLOSE, READWRITE_NO_CLOSE, argv);

createChild( pipe_fileDesc2, pipe_fileDesc2, "less", READWRITE_NO_CLOSE, NO_READ, argv);

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

if(argc >= 2){
		childpidGrep = wait( &status ); /* Vänta på ena child-processen */
	if ( -1 == childpidGrep) { perror( "wait() failed unexpectedly" ); exit( 1 ); }

	 	if( WIFEXITED( status ) ) /* child-processen har kört klart */
	{
		int child_status = WEXITSTATUS( status );
		    if( 0 != child_status ) /* child had problems */
		{
			fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
				(long int) childpidGrep, child_status );
		}
	}
	else
	{
		    if( WIFSIGNALED( status ) ) /* child-processen avbröts av signal */
		{
			int child_signal = WTERMSIG( status );
			fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
				(long int) childpidGrep, child_signal );
		}
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
