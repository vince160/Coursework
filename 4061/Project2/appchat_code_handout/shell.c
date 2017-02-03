#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "util.h"

/*
 * Read a line from stdin.
 */
char *sh_read_line(void)
{

	char *line ;
	ssize_t bufsize = 0;
	if( getline(&line, &bufsize, 0) == -1)
		perror("Shell cannot read from stdin");
  return line;
}

/*
 * Do stuff with the shell input here.
 */
int sh_handle_input(char *line, int fd_toserver)
{
	
	/***** Insert YOUR code *******/
 	/* Check for \seg command and create segfault */
	if(starts_with(line, CMD_SEG)) {
    char*n = NULL;
    *n = 1;
    return -1; 
	}
	/* Write message to server for processing */
  if(write(fd_toserver, line, strlen(line)+1) == -1) {
    perror("Failed to write to server");
  }
	return 0;
}

/*
 * Check if the line is empty (no data; just spaces or return)
 */
int is_empty(char *line)
{
	while (*line != '\0') {
		if (!isspace(*line))
			return 0;
		line++;
	}
	return 1;
}

/*
 * Start the main shell loop:
 * Print prompt, read user input, handle it.
 */
void sh_start(char *name, int fd_toserver)
{
  char buf[MSG_SIZE]; 
  int empty;
   
  while(1)
  {
    printf( "%s >> " , name );

		if(fflush(stdout) == EOF) {
      perror("Failed to flush stdout");
    }
    if(fflush(stdin) == EOF) {
      perror("Failed to flush stdin");
    }
    memset(buf,'\0',sizeof(buf));
    if( read(0, buf , MSG_SIZE)  < 0 ) 
		{
			perror("could not read from stdin" );
      return;
		}
    if(!is_empty(buf)){
      if(sh_handle_input( buf , fd_toserver ) == -1){
        perror("seg didn't work");
      }
    }
    usleep(1000);
  }
}

/* 
 * main shell function
 * Written by Rob Brennan and Brooke Padilla
 */
int main(int argc, char **argv)
{
	
  /*variable declarations*/
  int ptoc;
  int ctop;
  char name[MSG_SIZE];
  pid_t pid;
  char buf[MSG_SIZE];
  
	/* Extract pipe descriptors and name from argv */
  if(argc != 4)
  {
    perror( "Innapropriate number command line arguements for shell.c" );
    return -1;
  }
  
  ptoc = atoi(argv[1]);
  ctop = atoi(argv[2]);
	/* Fork a child to read from the pipe continuously */
  if( ( pid = fork() ) == 0 ) 
  {
	  //close(ctop);
	/*
	 * Once inside the child
	 * look for new data from server every 1000 usecs and print it
	 */
    while(1)
    { 
      if(read(ptoc,buf,MSG_SIZE) > 0)
        printf("%s\n", buf);
      usleep(1000);
    }
  } 
  else
  {/* Rebellious teenager loop */
	/* Inside the parent
	 * Send the child's pid to the server for later cleanup
	 * Start the main shell loop
	 */
    //close(ptoc);
 	  sprintf(buf, "%s %d", CMD_CHILD_PID, pid);
		if (write(ctop,buf, strlen(buf) + 1) < 0){
			perror("write to parent shell failed");
      exit(0);
    }

    sh_start( argv[3] , ctop );  
  }//End parent code
}
