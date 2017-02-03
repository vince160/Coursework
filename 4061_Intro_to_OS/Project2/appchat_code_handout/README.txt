PROJECT 2 4061
**Multi-Process Chatting Application**

/*
 *CSci4061 S2016 Assignment 2
 *Brooke Padilla, Robert Brennan, Alex Vincent
 *padil031,       brenn502,       vince160
 *sec 5,          sec 12,         sec 2
 *5102672         5113407         5111801
 3/11/2016
*/


/*****************************************************************/
/*********************PURPOSE OF THE PROGRAM**********************/
/*****************************************************************/

This program was created with the intent of simulating a multi-user 
chatting application within a single user machine. It does not 
actually connect via a protocol to other machines, so in reality it
is not a working chat interface. The code could me modified relatively
easily to add such functionality however. All input/output is handled
on the same machine.



/*****************************************************************/
/******************HOW TO COMPILE THE PROGRAM*********************/
/*****************************************************************/

A makefile has been provided to streamline the compilation process.

To compile, enter the following commands:
> make clean
> make

After successfully compiling, to run the shell simply type:
> ./server

which runs the server program in the current terminal window.


/*****************************************************************/
/*************************USING THE PROGRAM***********************/
/*****************************************************************/

after entering "./server" in the terminal window, the application
will begin.

As the program begins, only the server shell will be running. It will
display:

SERVER >>

At this point, users must be added by entering:

SERVER >> \add user

which, in this case, would open up an XTERM window for the user "user"
======================================================================
Once you have a few users running, you can start using the application.
In this example, suppose you have added 3 users, named "Alan", "Bill",
and "Bob"

If Bob wanted to send a message to Bill, he would simply type:
>> \p2p Bill Hi
Which would send the message "Hi" to Bill, but not to Alan.

If Bob wanted to send a message to all users he would simply type:
>> Hi Everyone!
Which would send the message "Hi Everyone!" to both Alan and Bill.

If Bob wanted to leave the chatroom, he would simply type?
>> \exit
And his shell process would exit.
=======================================================================

USEFUL COMMANDS: (Note: "\text" means that the "\" must be the first 
item typed at the prompt. Otherwise, the text will be processed like
any non-command text.)
=======================================================================
Some commands that the server can execute:
=======================================================================
\add Bill
->Adds another user who's name is "Bill" (can be any name)

\kick user
->Kicks the user who's name is "user" and cleans up the relevant processes

\list
->Lists all currently running users

\exit
->exits the program, closing all users and processes

<Any Other Text>
->Typing any other text followed by pressing "Enter" will broadcast
	the message that was typed to all users currently running.

======================================================================
Some commands that each user shell (i.e. Alan's terminal) can execute:
=======================================================================
\list
->Just like in the server shell; Lists all currently running users

\exit
->exit's that specific user's terminal. The server cleans up the
	process for that user if it needs to as well. All other
	users remain running.

\p2p username message
->Sends the message specified by "message" to the user specified
 	by "username". If the user specified does not exist, a
	message indicating that the user specified does not exist
	is sent the sender.

\seg
-> Segfaults the user by attempting to assign a null pointer a value.
	This causes the user shell to exit and the user to be removed.
<Any Other Text>
->Just like in the server shell; Typing any other text followed by
	pressing "Enter" will broadcast the message that was typed
	to all other users currently running.


/*****************************************************************/
/************************ASSUMPTIONS MADE*************************/
/*****************************************************************/
-We assume that the user of this program will be running it on a
 CSELABS Linux machine.
-We assume that \seg will not be called in the server process.





/*****************************************************************/
/***************STRATEGIES FOR ERROR HANDLING*********************/
/*****************************************************************/

In the program source code, all system calls are checked for error
return values (-1). If a -1 is returned from a system call, perror()
is subsequently called, printing to standard error a relevant error
message.

The following user-defined functions return -1 on failure:
-list_users
-add_user 
-find_user_index
-setnonblock
-sh_handle_input

When these functions return -1 (excluding sh_handle_input), the return 
value is caught in main, which subsequently returns -1 in response.






