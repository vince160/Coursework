/*CSci4061 S2016 Assignment 2
 *Brooke Padilla, Robert Brennan, Alex Vincent
 *padil031,       brenn502,       vince160
 *sec 5,          sec 12,         sec 2
 *5102672         5113407         5111801
 3/11/2016
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include "util.h"


/*
 * Utility function.
 * Given a command's input buffer, extract name.
 */
char *extract_name(int cmd, char *buf)
{
	char *s = NULL;

	s = strtok(buf, " ");
	s = strtok(NULL, " ");
	if (cmd == P2P)
		return s;	/* s points to the name as no newline after name in P2P */
	s = strtok(s, "\n");	/* other commands have newline after name, so remove it*/
	return s;
}

/*
 * Identify the command used at the shell 
 */
int parse_command(char *buf)
{
	int cmd;

	if (starts_with(buf, CMD_CHILD_PID))
		cmd = CHILD_PID;
	else if (starts_with(buf, CMD_P2P))
		cmd = P2P;
	else if (starts_with(buf, CMD_LIST_USERS))
		cmd = LIST_USERS;
	else if (starts_with(buf, CMD_ADD_USER))
		cmd = ADD_USER;
	else if (starts_with(buf, CMD_EXIT))
		cmd = EXIT;
	else if (starts_with(buf, CMD_KICK))
		cmd = KICK;
	else
		cmd = BROADCAST;

	return cmd;
}

/*
 * List the existing users on the server shell
 * Written by Brooke Padilla
 */
int list_users(user_chat_box_t *users, int fd)
{
	/* 
	 * Construct a list of user names
	 * Don't forget to send the list to the requester!
	 */
	 char list[MSG_SIZE]="";
   int i;
   
   for( i = 0 ; i < MAX_USERS ; i++ ){
     if( users[i].status == SLOT_FULL  ) {
       sprintf(list, "%s\n%s" , list, users[i].name );
     }
	 }
   if( write( fd , list , strlen(list) + 1 ) < 0 ){
     perror("Error: Could not output list of users");
     return -1;
   }
   return 0;
}

/*
 * Add a new user; returns user index on success
 * returns 0 if the maximum users has been reached
 * returns -1 if adding a user fails
 * Written by Alex Vincent
 */
int add_user(user_chat_box_t *users, char *buf, int server_fd)
{
  
  int i;
  int free_user_slot = 0;  //boolean, initially set to false; then set to true if slot available.
  int user_index;  //index of new user
  char * name= NULL;
	/***** Insert YOUR code *******/
	
	/* 
	 * Check if user limit reached.
	 *
	 * If limit is okay, add user, set up non-blocking pipes and
	 * notify on server shell
	 *
	 * NOTE: You may want to remove any newline characters from the name string 
	 * before adding it. This will help in future name-based search.
	 */ 
  for(i = 0; i < MAX_USERS; i++) {
	  if (users[i].status == SLOT_EMPTY) {
      user_index = i;  //ith user slot empty; Set as index
      free_user_slot = 1;  //set free slot boolean to true
     }
  }
  
  /* Checks if no free slot was found */  
  if(free_user_slot == 0) {
    printf("Failed to add user: User limit reached.\n");
    return -1;
  }
   
  /* Initializing user pipes */ 
  if( (pipe(users[user_index].ptoc)) == -1 || (pipe(users[user_index].ctop)) == -1 ) {
    perror("Could not open user pipes");
    return -1;
  }

  /* Setting user pipes to nonblocking */   
  if( (setnonblock(users[user_index].ptoc[0])) == -1  || (setnonblock(users[user_index].ptoc[1])) == -1
     || (setnonblock(users[user_index].ctop[0])) == -1 || (setnonblock(users[user_index].ctop[1])) == -1) {
    perror("Could not set user pipes non blocking flag");
    return -1;
  }
  
  /* Set user name */   
  name = extract_name(ADD_USER, buf);
  strcpy(users[user_index].name , name );
  /* Set slot status to full */
  users[user_index].status = SLOT_FULL;   
  
  printf("Adding user %s...\n", name);
   
  return user_index;   
}

/*
 * Broadcast message to all users. Completed for you as a guide to help with other commands :-).
 */
int broadcast_msg(user_chat_box_t *users, char *buf, int fd, char *sender)
{
	int i;
	const char *msg = "Broadcasting...", *s;
	char text[MSG_SIZE];
  
	/* Notify on server shell */
	if (write(fd, msg, strlen(msg) + 1) < 0)
		perror("writing to server shell");
	
	/* Send the message to all user shells */
	s = strtok(buf, "\n");
	
	sprintf(text, "%s : %s", sender, s);
	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY)
			continue;
		if (write(users[i].ptoc[1], text, strlen(text) + 1) < 0)
			perror("write to child shell failed");
	}
}

/*
 * Close all pipes for this user
 * Written by Alex Vincent
 */
void close_pipes(int idx, user_chat_box_t *users)
{
	/***** Insert YOUR code *******/
	//close pipes
  int i;

  if (close( users[idx].ptoc[1] ) == -1) {
    perror("Failed to close user pipe ptoc");
  }
  if (close( users[idx].ctop[0] ) == -1) {
    perror("Failed to close user pipe ctop");
  }
}

/*
 * Cleanup single user: close all pipes, kill user's child process, kill user 
 * xterm process, free-up slot.
 * Remember to wait for the appropriate processes here!
 * Written by Brooke Padilla
 */
void cleanup_user(int idx, user_chat_box_t *users)
{
	/***** Insert YOUR code *******/
  
  close_pipes(idx,users);
            
  if( (kill( users[idx].child_pid , SIGKILL )) == -1) {
    perror("failed to kill child");
  }
  if( (kill( users[idx].pid , SIGKILL )) == -1) {
    perror("failed to kill parent");
  }
	waitpid( &users[idx].child_pid );
	waitpid( &users[idx].pid );
  
  //List as free 
  users[idx].status = SLOT_EMPTY;

}

/*
 * Cleanup all users: given to you
 */
void cleanup_users(user_chat_box_t *users)
{
	int i;

	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY)
			continue;
		cleanup_user(i, users);
	}
}

/*
 * Cleanup server process: close all pipes, kill the parent process and its 
 * children.
 * Remember to wait for the appropriate processes here!
 */
void cleanup_server(server_ctrl_t server_ctrl)
{
  int i;
  
  for(i = 0 ; i < 2 ; i++) {
    if(close( server_ctrl.ptoc[i] ) == -1) {
      perror("Failed to close server ptoc pipe");
      }
    if(close( server_ctrl.ctop[i] ) == -1) {
      perror("Failed to close server ctop pipe");
      }
  }
  if(kill( server_ctrl.child_pid , SIGKILL ) == -1) {
    perror("Failed to kill child");
  }
  if(kill( server_ctrl.pid , SIGKILL ) == -1) {
    perror("Failed to kill parent");
  }
  if(wait( &server_ctrl.child_pid ) == -1) {
    perror("Failed to wait for child");
  }
  if(wait( &server_ctrl.pid ) == -1) {
    perror("Failed to wait for parent");
  }
  
}

/*
 * Utility function.
 * Find user index for given user name.
 */
int find_user_index(user_chat_box_t *users, char *name)
{
	int i, user_idx = -1;

	if (name == NULL) {
		fprintf(stderr, "NULL name passed.\n");
		return user_idx;
	}
	for (i = 0; i < MAX_USERS; i++) {
		if (users[i].status == SLOT_EMPTY)
			continue;
		if (strncmp(users[i].name, name, strlen(name)) == 0) {
			user_idx = i;
			break;
		}
	}

	return user_idx;
}


/*
 * Send personal message. Print error on the user shell if user not found.
 * Written by Brooke Padilla and Alex Vincent
 */
void send_p2p_msg(int idx, user_chat_box_t *users, char *buf)
{
	/* get the target user by name (hint: call (extract_name() and send message */
	char* name = NULL;     //eventually holds the recipient's name     //temporarily holds the buffer to extract the message
  int target_user, i;  
 	char text[MSG_SIZE];  //message stored here
	char *txt , *txt2; 

  
  name = extract_name(P2P , buf);

	txt = strtok( NULL , " " ); 
	while ( (txt2 = strtok( NULL , " " ) ) != NULL ) 
	{
		sprintf(txt , "%s %s" , txt , txt2 );
	}
	
  if( (target_user = find_user_index( users , name )) == -1) {  //if user does not exist; print message to sender
	  sprintf(text, "%s : %s", users[idx].name, "User not found\n");
    if (write(users[idx].ptoc[1], text, strlen(text) + 1) < 0)
    {
      perror("write to user shell failed");
      return;
    }
    return;
  }
  else {  //send message.
	  sprintf(text, "%s : %s", users[idx].name, txt);
  }
  if (write(users[target_user].ptoc[1], text, strlen(text) + 1) < 0)
  {
    perror("write to user shell failed");
    return;
  }
  
}

/*
 * The setnonblock from the power point
 */
int setnonblock (int fd) {
  int fdflags;
  if(fdflags = fcntl (fd, F_GETFL, 0) == -1)
    return -1;
  fdflags |= O_NONBLOCK; 
  if (fcntl (fd, F_SETFL, fdflags) == -1)
    return -1;
  return 0;
}

/*
 * main server function
 * Written by Alex Vincent and Brooke Padilla
 */
int main(int argc, char **argv)
{
	
	/***** Insert YOUR code *******/
	
	/* open non-blocking bi-directional pipes for communication with server shell */
  server_ctrl_t server;
  user_chat_box_t users[MAX_USERS];
  char buffer[MSG_SIZE];
  char fd_args[2][2];/* ptoc[0],ctop[1] */
  int i , curr_user , n , status ;
  char prog_path[MSG_SIZE];
  char * temp;

  
  if( pipe( server.ptoc ) == -1 || pipe( server.ctop ) == -1 )
  {
    perror("Could not open server pipes");
    return -1;
  }
  if ( setnonblock(server.ptoc[0]) == -1 || setnonblock(server.ptoc[1]) == -1
        || setnonblock(server.ctop[0]) == -1 || setnonblock(server.ctop[1]) == -1 ){
     perror("Could not set server pipes non blocking flag");   
     return -1;  
  }
  /* set all users.status to being an empty slot */
  for ( i = 0 ; i < MAX_USERS ; i++){
    users[i].status = SLOT_EMPTY;
  }
	/* Fork the server shell */
  if((server.pid = fork()) == 0){
		/* 
	 	 * Inside the child.
		 * Start server's shell.
	 	 * exec the SHELL program with the required program arguments.
	 	 */
    if (sprintf( prog_path, "%s/%s", CURR_DIR , SHELL_PROG ) == -1) {
      perror("Failed to print path");
      return -1;
    }
    
    if (sprintf( fd_args[0] , "%d" , server.ptoc[0] ) == -1) {
      perror("Failed to print argument 1"); 
      return -1;
    }
    
    if (sprintf( fd_args[1] , "%d" , server.ctop[1] ) == -1) {
      perror("Failed to print argument 2");
      return -1;
    }
		
    if( execl( prog_path, SHELL_PROG , fd_args[0], fd_args[1], "SERVER" , NULL ) < 0 )
    {
    	perror("could not launch server shell");
    	return -1;
    }
  }
  else 
  {

    
    sprintf( prog_path, "%s/%s", CURR_DIR , SHELL_PROG );
    
	/* Inside the parent. This will be the most important part of this program. */

		/* Start a loop which runs every 1000 usecs.
	 	 * The loop should read messages from the server shell, parse them using the 
	 	 * parse_command() function and take the appropriate actions. */
		while (1) {
			/* Let the CPU breathe */
			usleep(1000);

			/* 
		 	 * 1. Read the message from server's shell, if any
		 	 * 2. Parse the command
		 	 * 3. Begin switch statement to identify command and take appropriate action
		 	 *
		 	 * 		List of commands to handle here:
		 	 * 			CHILD_PID
		 	 * 			LIST_USERS
		 	 * 			ADD_USER
		 	 * 			KICK
		 	 * 			EXIT
		 	 * 			BROADCAST 
		 	 */
          
      if( (n = read(server.ctop[0],buffer,MSG_SIZE)) > 0 ) { //reads message from the server shell so long as it exists
        int cmd = parse_command(buffer);   
        switch(cmd) {
          case(CHILD_PID):
            temp = extract_name(CHILD_PID, buffer);
            server.child_pid = atoi( temp );
            break;
          case(ADD_USER):
            if ((curr_user = add_user( users, buffer , server.ptoc[1] )) > 0) {  //add user and open xterm only if adding successful
              if((users[curr_user].pid = fork()) == 0){
                  sprintf( fd_args[0] , "%d" , users[curr_user].ptoc[0] ); 
                  sprintf( fd_args[1] , "%d" , users[curr_user].ctop[1] );
                  
                  if( execl(XTERM_PATH, XTERM, "+hold", "-e", prog_path , fd_args[0], fd_args[1], users[curr_user].name , NULL ) < 0)
                  {
                  	perror("could not launch child shell");
                	return -1;
                  }
              }
              else{
                if(close(users[curr_user].ptoc[0]) == -1) { //close ptoc read
                  perror("Failed to close read end of user pipe");
                  return -1;
                }
                if(close(users[curr_user].ctop[1]) == -1) { //close ctop write
                  perror("Failed to close write end of user pipe");
                  return -1;
                }
              }
            }
            break;
          case(KICK):
            temp = extract_name(KICK, buffer); 
            curr_user = find_user_index( users, temp );
            if( curr_user != -1 ) {
              cleanup_user( curr_user, users );
              }
            else{
              perror("Failed to clean up user");
            }
            break;
          case(EXIT):
            cleanup_users( users );
            cleanup_server( server );
            return 0;
            break;
          case(LIST_USERS):
            if (list_users( users, users[i].ptoc[1] ) == -1) {
              return -1;
            }
            printf("\n");  //prints newline for readability on server shell
            break;
          default:
            if (broadcast_msg( users, buffer, server.ptoc[1], "SERVER") == -1) {
              return -1;
              }
            break;

				} /* End switch */
      } /* End SERVER SHELL read */
    	else if( n == 0 && waitpid(server.pid,status,WNOHANG) == -1) {
        close_pipes(i,users);
        //cleanup_users( users );
        if(waitpid(server.child_pid,status,WNOHANG) != -1)
        {
          if( (kill(server.child_pid , SIGKILL )) == -1) {
    				perror("failed to kill child");
    			}
    			if(waitpid( &server.child_pid )== -1)
    				perror("waitpid failed");
        }
      }
    	
			/* Fork a process if a user was added (ADD_USER) */
				/* Inside the child */
				/*
			 	 * Start an xterm with shell program running inside it.
			 	 * execl(XTERM_PATH, XTERM, "+hold", "-e", <path for the SHELL program>, ..<rest of the arguments for the shell program>..);
			 	 */

			/* Back to our main while loop for the "parent" */
			/* 
		 	 * Now read messages from the user shells (ie. LOOP) if any, then:
		 	 * 		1. Parse the command
		 	 * 		2. Begin switch statement to identify command and take appropriate action
		 	 *
		 	 * 		List of commands to handle here:
		 	 * 			CHILD_PID
		 	 * 			LIST_USERS
		 	 * 			P2P
		 	 * 			EXIT
		 	 * 			BROADCAST
		 	 *
		 	 * 		3. You may use the failure of pipe read command to check if the 
		 	 * 		user chat windows has been closed. (Remember waitpid with WNOHANG 
		 	 * 		from recitations?)
		 	 * 		Cleanup user if the window is indeed closed.
		 	 */

      for( i =0 ; i < MAX_USERS ; i++ ){
        if( (users[i].status == SLOT_FULL) && ((n = read(users[i].ctop[0],buffer,MSG_SIZE)) > 0) ) { //reads message from the server shell so long as it exists
          int cmd = parse_command(buffer);
          switch(cmd) {
            case(CHILD_PID):
              temp = extract_name(CHILD_PID, buffer);
              users[i].child_pid = atoi( temp );
              break;
            case(LIST_USERS):
              if (list_users( users, users[i].ptoc[1] ) == -1) {
                return -1;
              }
              break;
            case(P2P):
              send_p2p_msg( i, users, buffer );
              break;
            case(EXIT):
              cleanup_user( i, users );
              break;
            default:
							//fflush(stdin);
              if (broadcast_msg( users, buffer, server.ptoc[1], users[i].name ) == -1 ) {
                return -1;
              }
              break;

          } /* End switch */
				}
        else if( (users[i].status == SLOT_FULL) && n == 0 && waitpid(users[i].pid,users[i].status,WNOHANG) == -1) {
          close_pipes(i,users);
         	if(waitpid(users[i].child_pid,users[i].status,WNOHANG) != -1)
          {
          	if( (kill( users[i].child_pid , SIGKILL )) == -1) {
    					perror("failed to kill child");
    				}
    				if(waitpid( &users[i].child_pid )== -1)
    					perror("waitpid failed");
          }
          //List as free 
          users[i].status = SLOT_EMPTY;
        }
      } /* End USER SHELL read */
      
	  }	/* while loop ends when user shell sees the \exit command */
  } /* end of parent code */
	return 0;
}
