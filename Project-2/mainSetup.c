#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <wait.h>
#include <signal.h>
#include <string.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
 
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void setup(char inputBuffer[], char *args[],int *background)
{
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)

        exit(0);            /* ^d was entered, end of user command stream */

/* the signal interrupted the read system call */
/* if the process is in the read() system call, read returns -1
  However, if this occurs, errno is set to EINTR. We can check this  value
  and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	exit(-1);           /* terminate with error code of -1 */
    }

	printf(">>%s<<",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

        switch (inputBuffer[i]){
	    case ' ':
	    case '\t' :               /* argument separators */
		if(start != -1){
                	args[ct] = &inputBuffer[start];    /* set up pointer */
			ct++;
		}
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
		start = -1;
		break;

            case '\n':                 /* should be the final char examined */
		if (start != -1){
                	args[ct] = &inputBuffer[start];     
		    	ct++;
		}
               inputBuffer[i] = '\0';
               args[ct] = NULL; /* no more arguments to this command */
               break;

	    default :             /* some other character */
		if (start == -1)
		    start = i;
                if (inputBuffer[i] == '&'){
		    *background  = 1;
                    inputBuffer[i-1] = '\0';
		}
	} /* end of switch */
     }    /* end of for */
     args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++)
		printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

char* find_given_command(char* args[]) {

	char* path = getenv("PATH");
	FILE* file;
	char* token = strtok(path, ":");
	char* my_command = args[0];
	char* filename;
	int is_exists = 0;
	while (token != NULL) {
		filename = malloc(strlen(token) + strlen(my_command) + 2);
		strcpy(filename, token);
		strcat(filename, "/");
		strcat(filename, my_command);

		if (file = fopen(filename, "r")) {
			printf("Your file is here %s\n", filename);
			is_exists = 1;
			return filename;
		}
		token = strtok(NULL, ":");
	}
}

int main(void)
{
            char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
            int background; /* equals 1 if a command is followed by '&' */
            char *args[MAX_LINE/2 + 1]; /*command line arguments */
            char *path;
            pid_t childpid;
            while (1){
                       background = 0;
                       printf("myshell: ");
                       /*setup() calls exit() when Control-D is entered */
                       setup(inputBuffer, args, &background);
                       childpid = fork();
			if (childpid == -1 ) {
				perror("failed to fork");
				return 1;
			}
			if (childpid == 0) { // in child
				//execv(const char *path, char *const argv[]);
				path = find_given_command(args);
				printf("the Path is : %s\n", path);
				execv(path, args);

			} else { // in Parent
				if ( background == 0 ) {
					wait(NULL);
				} else {
					setup(inputBuffer, args, &background);		
				}
			}
			
                        /** the steps are:
                        (1) fork a child process using fork()
                        (2) the child process will invoke execv()
						(3) if background == 0, the parent will wait,
                        otherwise it will invoke the setup() function again. */
            }
}