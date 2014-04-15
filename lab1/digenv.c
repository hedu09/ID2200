/* Include-rader ska vara allra först i ett C-program */
#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */
#include <string.h>
#include <sys/wait.h>

#define PIPE_READ_SIDE ( 0 ) /* Define the read side for a pipe to simplify */
#define PIPE_WRITE_SIDE ( 1 ) /* Define the write side for a pipe to simplify */

#define NO_READ ( 0 ) /* Define the read side for a pipe to simplify */
#define READWRITE_CLOSE ( 1 ) /* Define the write side for a pipe to simplify */
#define READWRITE_NO_CLOSE ( 2 ) /* Define the write side for a pipe to simplify */

pid_t childpid; /* childProcessID for printenvp */

int returnValue; /* Return value, to findout if an error occured */
int status; /* Return codes for children */ 

/* createChild will create a child that will execute a command and change STDOUT will be sent to the write pipe and STDIN sent to read pipe.
@Param	pipe_readfiledesc[2] - pipe filedescriptor the child to read from 
@Param	pipe_writefiledesc[2]- pipe filedescriptor the child to write to 
@Param	command[] - command that the child will run in execlp unless it grep it will run in execvp 
@Param	read	- if 0 do not read from pipe, if 1 read and close write pipe of pipe_readfiledesc, 
		if 2 read from pipe but dont close write pipe_readfiledesc
@Param	write	- if 0 do not write from pipe, if 1 write and close read pipe of pipe_writefiledesc, 
		if 2 write from pipe but dont close read of pipe_writefiledesc
@Param	**argv	- Same param as in main, arguments to be sent to exec


*/
void createChild(int pipe_readfiledesc[2], int pipe_writefiledesc[2], char command[], int read, int write, char **argv){
	
	childpid = fork(); /* Create a child process*/
	if (-1 == childpid) /* Check if fork failed and print error message*/
	{
		char * errorMessage = "UNKNOWN"; /* if no known error message print UNKNOWN*/
		if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
		if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
		fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
		exit(1);
	}

	/* ---------------------------- Child ---------------------------- */
	else if (0 == childpid) 
	{	/* child running*/
		fprintf( stderr, "Child (%s, pid %ld) started\n", command,	
			(long int) getpid() );
		if (read >= 1){ /* Check if child will need to read from pipe*/
			returnValue = dup2(pipe_readfiledesc[PIPE_READ_SIDE], STDIN_FILENO); /* copy pipe read side to STDIN
			so everything sent to STDIN will be sent to the pipe*/
			if (read == 1){ /*Check if we want to close the pipe write side*/
				returnValue = close(pipe_readfiledesc[PIPE_WRITE_SIDE]); /*close pipe*/
				if (-1 == returnValue)	{ perror("Cannot close pipe RFD");	exit(1); }
			}
		}
		if (write >= 1){
			returnValue = dup2(pipe_writefiledesc[PIPE_WRITE_SIDE], STDOUT_FILENO);	/* copy pipe write side to STDOUT
			so everything writen to STDOUT will be written to the pipe*/
			if( write == 1){ /*Check if we want to close the pipe read side*/
				returnValue = close(pipe_writefiledesc[PIPE_READ_SIDE]);/*close pipe*/
				if (-1 == returnValue)	{ perror("Cannot close pipeWFD");	exit(1); }
			}	
		}
		
		if (strcmp( "grep", command) == 0) /* Run grep with execvp instead of execlp */
		{
			argv[0]= "grep"; /*replace argv first element with grep 
					(only place where we use argv so its okay to replace the value)*/
			(void) execvp("grep", argv);/* execute grep with arguments in argv
						we will not return unless an error occured*/
			perror("No matches found");
			exit(1);
		}
		else
		{
			(void) execlp(command, command, (char *) 0); /* execute command 
						we will not return unless an error occured*/
			if (strcmp( "less", command) == 0){ /* if less fails run more*/
				(void) execlp("more", "more", (char *) 0);/* execute command 
						we will not return unless an error occured*/
			}
		}
		perror("Cannot exec perror roar! row65");
		exit(1);
	}
	/* ---------------------------- Parent ---------------------------- */
	if (read >= 1){ /* if child reads we need to close the parents read pipe*/
		returnValue = close(pipe_readfiledesc[PIPE_READ_SIDE]);/*close pipe*/
		if (-1 == returnValue)	{ perror("Cannot close pipe Parent R");	exit(1); }/* check return value*/
	}
	if (write >= 1){/* if child writes we need to close the parents write pipe*/
		returnValue = close(pipe_writefiledesc[PIPE_WRITE_SIDE]);/*close pipe*/
		if (-1 == returnValue)	{ perror("Cannot close pipe Parent W");	exit(1); }/* check return value*/
	}
}
/* COMMMMMMENT HHHEEEEERRRREEE!*/
void childHandler()
{
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
}

void childHandlerGrep()
{
	childpid = wait( &status ); /* Vänta på ena child-processen */
	if ( -1 == childpid) { perror( "wait() failed unexpectedly" ); exit( 1 ); }

 	if( WIFEXITED( status ) ) /* child-processen har kört klart */
	{
		int child_status = WEXITSTATUS( status );
		if( 0 != child_status ) /* child had problems */
		{
			if (1 == child_status)
			{
				fprintf( stderr, "Child (pid %ld) failed, No matches %d\n",
				(long int) childpid, child_status );
			}
			else
			{
			fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
				(long int) childpid, child_status );
			}
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

	int pipe_fileDesc[2]; /* File descriptiors for pipe, printEnvp to sort -> grep  */
	int pipe_fileDesc2[2];  /* File descriptiors for pipe, printEnvp or grep -> sort -> less*/
	int pipe_fileDesc3[2]; /* File descriptiors for pipe, printEnvp -> grep -> sort */

	
	returnValue = pipe( pipe_fileDesc); /* Create a pipe */
if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); } /* check return value*/

if (argc == 1){ /* no extra arguments then create a child printenv that pipes printenv -> sort */
	createChild( pipe_fileDesc, pipe_fileDesc,  "printenv", NO_READ, READWRITE_CLOSE, argv);
}
else if (argc >= 2)/* extra arguments then create a child printenv that pipes printenv -> grep */
{
	returnValue = pipe(pipe_fileDesc3);/* create pipe for printenv -> grep -> sort*/
	if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }/* check return value*/
	
	/* execute printenv and pipe STDIN -> printenv -> grep */
	createChild(pipe_fileDesc, pipe_fileDesc3, "printenv", NO_READ, READWRITE_CLOSE, argv); 
	/* execute grep pipe printenv -> grep -> sort */
	createChild(pipe_fileDesc3, pipe_fileDesc, "grep", READWRITE_NO_CLOSE, READWRITE_NO_CLOSE, argv); 
}

	returnValue = pipe( pipe_fileDesc2); /* Create a pipe printenv/grep -> sort -> less*/
if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }/* check return value*/

	/* execute grep pipe printenv/grep -> sort -> less */
createChild( pipe_fileDesc, pipe_fileDesc2, "sort", READWRITE_CLOSE, READWRITE_NO_CLOSE, argv); 
	/* execute  pipe sort -> less -> STDOUT */
createChild( pipe_fileDesc2, pipe_fileDesc2, pagerEnv , READWRITE_NO_CLOSE, NO_READ, argv);

	/* Run one childhandler for each child that is being created */
	childHandler();
	if (argc >= 2)
	{
		childHandlerGrep();
	}
	childHandler();
	childHandler();

	exit(0); // Normal terminate
}
