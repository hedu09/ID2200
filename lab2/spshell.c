#include <signal.h> /* For sigignore */
#include <sys/types.h> /* definierar bland annat typen pid_t */
#include <errno.h> /* definierar felkontrollvariabeln errno */
#include <stdio.h> /* definierar stderr, dit felmeddelanden skrivs */
#include <stdlib.h> /* definierar bland annat exit() */
#include <unistd.h> /* definierar bland annat fork() */
#include <string.h> /* Define strcmp */
#include <sys/wait.h> /* Prevent gcc -Wall errors in Mac */
#include <unistd.h> /* Add the cd command */
#include <sys/time.h> /* Get timeofday() function */

/* Function declarations */
void createChild (char**);
void childHandler();
void donothinghandler(int s);

pid_t childpid; /* childProcessID for printenvp */
int status; /* Return codes for children */

#define BUFFERSIZE (71) /* Define maximum size of the buffer, assumption from lab specification */
#define ARGVSIZE (6) /* Define maximum size of the ARGC, assumption from lab specification */
#define WAIT (0) /* We should wait for the process to terminate */
#define CHECKNOWAIT (1) /* Check child status but dont wait until it has terminated  */

/* createChild will create a child that will execute a command and change STDOUT to be sent to the write pipe and STDIN sent to read pipe.
@Param	**input	- Input arguments to be run, that has been feed into the shell. */
void createChild(char **input) {
	childpid = fork(); /* Create a new process */
	if (-1 == childpid) {
		char * errorMessage = "UNKNOWN"; /* if no known error message print UNKNOWN*/
		if (EAGAIN == errno){errorMessage = "cannot allocate page table";}
		if (ENOMEM == errno){errorMessage = "cannot allocate kernel data";}
		fprintf(stderr, "fork() blew up because: %s\n", errorMessage);
		exit(1);/* Exit with error*/
	}
	else if(0 == childpid) { /* Child returns 0 */
		fprintf( stderr, "Child (%s, pid %ld) started\n", input[0], (long int) getpid() );
		(void) execvp(input[0], input); /* Execute command */
		perror("Cannot exec perror");
		exit(1); /*exit with error*/
	}
}
/* Handles the child when the child has terminated by signal or died with or without error
we need to handle the child when it dies or it will become a zombie */
/* @Param	wait - Process will wait until it recieve a signal. */
void childHandler(int wait) {
	if(wait == WAIT){  /* Process will wait until it recieve a signal */
		waitpid( childpid , &status , 0 ); /* waiting on a child process */		
		if ( -1 == childpid) { perror( "wait() failed unexpectedly" ); exit( 1 ); } /* Check if wait works correctly*/

		if( WIFEXITED( status )) { /* Check if child has terminated normally or by signal */
			int child_status = WEXITSTATUS( status ); /*get the exit code specified by the child process*/
			if( 0 != child_status ) {/* child had problems */
				fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
					(long int) childpid, child_status );
			}
			printf("Foreground process: (pid %ld), terminated\n", (long int) childpid); 
		}
		else {
			if( WIFSIGNALED( status ) ) {/* child-process terminated by signal */
				int child_signal = WTERMSIG( status ); /*get the exit code specified by the child process*/
				fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
					(long int) childpid, child_signal );
			}
		}	
	}
	else if (wait == CHECKNOWAIT){  /* Check for signal but dont wait */
		int returnvalue; /* Dont hang when waiting, check and move on */
		while((returnvalue = waitpid(-1, &status ,WNOHANG)) > 0){			
			if( WIFEXITED( status )  ) { /* Check if child has terminated normally or by signal */
				int child_status = WEXITSTATUS( status ); /*get the exit code specified by the child process*/
				if( 0 != child_status ) {/* child had problems */
					fprintf( stderr, "Child (pid %ld) failed with exit code %d\n",
						(long int) childpid, child_status );
				}
				printf("Background process: (pid %ld), terminated\n", (long int) childpid); 
			}
			else {
				if( WIFSIGNALED( status ) ) { /* child-process terminated by signal */
					int child_signal = WTERMSIG( status ); /*get the exit code specified by the child process*/
					fprintf( stderr, "Child (pid %ld) was terminated by signal no. %d\n",
						(long int) childpid, child_signal );
				}
			}	
		}		
	}
}
/* This signal handler does absolutly nothing, due to spefications. 
@ Param int s - type of signal 
*/
void donothinghandler(int s){
        /*   printf("Caught signal %d\n and ignore it \n",s); */
}

/* Main acts as the parents process in this program, the program starts with entering an infintate loop in order to capture commands given by the user.
This is done by a string tokenizer that uses diffrent if cases to detect and sanatize input. A user can change directory and give the exit command.
Programs may be started as foreground or background processes with the use of the '&' sign. When starting a foreground process some statistics is 
presented for the user and then the loop starts again.
@ Param int argc - number of arguments we have started our program with 
@ Param char **argv - all the arguments we have is in a vector 
*/
int main(int argc, char **argv) {	
	fprintf( stderr, "Parent (Parent, pid %ld) started\n", (long int) getpid() );  /* printing parents id for easier debuggning*/
	
	struct sigaction sa; /* Create a struct for sa */
	sa.sa_handler = donothinghandler; /* Set the handler to our donothinghandler */
	sa.sa_flags = SA_RESTART; /* Restart function if interrupted by handler */
	sigaction(SIGINT, &sa, NULL); /* Call sighandler with interupt signal */

	while(1){ /* Loop forever */
		printf("DEBUG: Start of loop\n");		
		char inputBuffer[BUFFERSIZE]; /* Declare input buffer for command, hard code to Stack */
		char **arguments = (char **) calloc (ARGVSIZE,  BUFFERSIZE); /* Allocate memory for toknizer */		
		double timeElapsed; /* Declare a time elapsed varible */

		fgets(inputBuffer, BUFFERSIZE , stdin); /* Read command from terminal */

		childHandler(CHECKNOWAIT); /* Check in there is any signal */

		inputBuffer[strlen(inputBuffer)-1]= '\0'; /* Remove the newline char and replace it with null */ 		
		char *arg = strtok(inputBuffer, " "); /* Split the input on space */
		
		/* Incase of spaces or enter*/
		if (arg == NULL) {
			printf("Say again?\n");
			continue; /* Reloop */
		}

		if (strcmp( "exit", arg) == 0) {/* Exit command was given */
			free(arguments); /* Relese memory */
			printf("Thank you come again!\n");
			exit(0); /* Terminate the program normaly due to exit */
		}
		
		int length = 0; /* this is the length of the number of arguments we have */
		while( arg != NULL){ /* Read until NULL */ 
			arguments[length] = arg; /* Point to the input */
			arg = strtok(NULL, " "); /* Move on */
			length++;
		}
		
		if (strcmp( "cd", arguments[0]) == 0) {/* check if first argument is cd */
			if (-1 == chdir(arguments[1])) {/* Something went wrong */
				char *homeEnv; /* char pointer for the home varible */
				homeEnv = getenv("HOME"); /* Get the home */
				printf("DEBUG: Selected home: %s\n", homeEnv);
				chdir(homeEnv); /* Fallback */
			}
			else { /* chdir returned 0 */
				chdir(arguments[1]); 
			}
			free(arguments); /* Relese memory */
			continue; /* Move on */
		}
				
		if (strcmp("&", arguments[length-1] ) == 0) { /* Check if we should run in foreground or background */
			printf("Running: BACKGROUND\n"); 
			arguments[length-1]= (char *) NULL;
			createChild(arguments);
		}
		else {
			/* Create the struct for our time variables according to the man page*/
			struct timeval starTime , endTime;
			gettimeofday(&starTime, NULL); /* Get start time */
			printf("Running: FOREGROUND\n");
			createChild(arguments); /*create a child */
			childHandler(WAIT); /* Send PID */
			gettimeofday(&endTime, NULL); /* Get end time*/
		
			/* Calculate and then convert to micro seconds */
			timeElapsed = ((endTime.tv_sec - starTime.tv_sec) * 1000000.0); 
			timeElapsed += (endTime.tv_usec - starTime.tv_usec);	

			printf("Wallclock time: %f msec \n", timeElapsed); 
		}		
		free(arguments); /* Relese memory */
		printf("DEBUG: End of loop\n");
	}
	exit(0);
}