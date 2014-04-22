#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */
#include <string.h> /* Define strcmp */
#include <sys/wait.h> /* Prevent gcc -Wall errors in Mac */

#define PIPE_READ_SIDE ( 0 ) /* Define the read side for a pipe to simplify */
#define PIPE_WRITE_SIDE ( 1 ) /* Define the write side for a pipe to simplify */

#define NO_READ ( 0 ) /* Define the read side for a pipe to simplify */
#define READWRITE_CLOSE ( 1 ) /* Define the write side for a pipe to simplify */
#define READWRITE_NO_CLOSE ( 2 ) /* Define the write side for a pipe to simplify */

pid_t childpid; /* childProcessID for printenvp */
int returnValue; /* Return value, to findout if an error occured */
int status; /* Return codes for children */ 

/* createChild will create a child that will execute a command and change STDOUT to be sent to the write pipe and STDIN sent to read pipe.
@Param	pipe_readfiledesc[2] - pipe filedescriptor the child reads from 
@Param	pipe_writefiledesc[2]- pipe filedescriptor the child writes to 
@Param	command[] - command that the child will run in execlp unless it grep it will run in execvp 
@Param	read	- if 0 do not read from pipe, if 1 read and close write pipe of pipe_readfiledesc, 
		if 2 read from pipe but dont close write pipe_readfiledesc
@Param	write	- if 0 do not write from pipe, if 1 write and close read pipe of pipe_writefiledesc, 
		if 2 write from pipe but dont close read of pipe_writefiledesc
@Param	**argv	- Same param as in main, arguments to be sent to run with the command grep. 
*/
void createChild(int pipe_readfiledesc[2], int pipe_writefiledesc[2], char command[], int read, int write, char **argv)
{	
	childpid = fork(); /* Create a child process*/
	if (-1 == childpid) /* Check if fork failed and print error message*/
	{
		char * errorMessage = "UNKNOWN"; /* if no known error message print UNKNOWN*/
		if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
		if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
		fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
		exit(1);/* Exit with error*/
	}

	/* ---------------------------- Child ---------------------------- */
	else if (0 == childpid) 
	{	/* child running*/
		fprintf( stderr, "Child (%s, pid %ld) started\n", command,	
			(long int) getpid() );
		if (read >= 1){ /* Check if child will need to read from pipe*/
			returnValue = dup2(pipe_readfiledesc[PIPE_READ_SIDE], STDIN_FILENO); /* copy pipe read side to STDIN
			so everything sent to STDIN will be sent to the pipe instead*/
			if (read == 1){ /*Check if we want to close the pipe write side*/
				returnValue = close(pipe_readfiledesc[PIPE_WRITE_SIDE]); /*close pipe*/
				if (-1 == returnValue)	{ perror("Cannot close pipe RFD");	exit(1); } /* Check if pipe was closed correctly*/
			}
		}
		if (write >= 1){
			returnValue = dup2(pipe_writefiledesc[PIPE_WRITE_SIDE], STDOUT_FILENO);	/* copy pipe write side to STDOUT
								so everything writen to STDOUT will be written to the pipe instead*/
			if( write == 1){ /*Check if we want to close the pipe read side*/
				returnValue = close(pipe_writefiledesc[PIPE_READ_SIDE]);/*close pipe*/
				if (-1 == returnValue)	{ perror("Cannot close pipeWFD");	exit(1); } /* Check if pipe was closed correctly*/
			}	
		}
		
		if (strcmp( "grep", command) == 0) /* Run grep with execvp instead of execlp */
		{
			argv[0]= "grep"; 	/*replace argv first element with grep 
						(only place where we use argv so its okay to replace the value)*/
			(void) execvp("grep", argv);	/* execute grep with arguments in argv
							we will not return unless an error occured*/
			perror("No matches found"); 	/* most common error from grep, it will also print this line even if there is other errors, 
							its not perfectly created to catch other error */
			exit(1); /*exit with error*/
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
		perror("Cannot exec perror");
		exit(1); /*exit with error*/
	}
	/* ---------------------------- Parent ---------------------------- */
	if (read >= 1){ /* if child reads we need to close the parents read pipe*/
		returnValue = close(pipe_readfiledesc[PIPE_READ_SIDE]);/*close pipe*/
		if (-1 == returnValue)	{ perror("Cannot close pipe Parent R");	exit(1); }/* Check if pipe was closed correctly*/
	}
	if (write >= 1){/* if child writes we need to close the parents write pipe*/
		returnValue = close(pipe_writefiledesc[PIPE_WRITE_SIDE]);/*close pipe*/
		if (-1 == returnValue)	{ perror("Cannot close pipe Parent W");	exit(1); }/* Check if pipe was closed correctly*/
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
/* Handles the child grep when the grep has terminated by signal or died with or without error
we need to handle the child when it dies or it will become a zombie 
childHandlerGrep() works as childHandler() except that if it terminate normally it has an extra check that checks if grep fails to find a pattern finds*/
void childHandlerGrep()
{
	childpid = wait( &status ); /* waiting on a child process */
	if ( -1 == childpid) { perror( "wait() failed unexpectedly" ); exit( 1 ); }/* Check if wait works correctly*/


 	if( WIFEXITED( status ) ) /* Check if child has terminated normally or by signal */
	{
		int child_status = WEXITSTATUS( status );/*get the exit code specified by the child process*/
		if( 0 != child_status ) /* child had problems */
		{
			if (1 == child_status)/* check if no pattern was found by grep*/
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
			int child_signal = WTERMSIG( status ); /*get the exit code specified by the child process*/
			fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
				(long int) childpid, child_signal );
		}
	}	
}
/* Main creates pipes and then creates children that will run commands "printEnvp | sort | less" when there is no arguments
pipes printEnvp -> pipe_fileDesc -> sort -> pipe_fileDesc2 -> less -> STDOUT
If we have more than 1 arguments we will instead run "printEnvp | grep argument | sort | less"
pipes printEnvp -> pipe_fileDesc3 -> grep -> pipe_fileDesc -> sort -> pipe_fileDesc2 -> less -> STDOUT
At the end main will wait for each child status to handle them with signal if they die normaly or terminate by error.
@ Param int argc - number of arguments we have started our program with 
@ Param char **argv - all the arguments we have is in a vector 
*/
int main(int argc, char **argv)
{	
	char *pagerEnv; /* char pointer for the inviorment varible for the pager*/
	pagerEnv = getenv("PAGER"); /* Get the page variable if it is set*/
	printf("DEBUG: Selected pager: %s\n", pagerEnv);
	if (NULL == pagerEnv) { /* Page enviorment isn't set*/
		pagerEnv = "less"; /* Set it to less*/ 
	}

	fprintf( stderr, "Parent (Parent, pid %ld) started\n",
	(long int) getpid() ); /* printing parents id for easier debuggning*/

	int pipe_fileDesc[2]; /* File descriptiors for pipe, printEnvp to sort -> grep  */
	int pipe_fileDesc2[2];  /* File descriptiors for pipe, printEnvp or grep -> sort -> less*/
	int pipe_fileDesc3[2]; /* File descriptiors for pipe, printEnvp -> grep -> sort */

	
	returnValue = pipe( pipe_fileDesc); /* Create a pipe */
	if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); } /* check return value for errors*/

	if (argc == 1){ /* no extra arguments then create a child printenv that pipes printenv -> sort */
		createChild( pipe_fileDesc, pipe_fileDesc,  "printenv", NO_READ, READWRITE_CLOSE, argv);
	}

	else if (argc >= 2)/* extra arguments then create a child printenv that pipes printenv -> grep */
	{
		returnValue = pipe(pipe_fileDesc3);/* create pipe for printenv -> grep -> sort*/
		if ( -1 == returnValue) { perror("Cannot create pipe");	exit(1); }/* check return value for errors*/
		
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
	if (argc >= 2) {
		childHandlerGrep();
	}
	childHandler();
	childHandler();

	exit(0); // Normal terminate
}
