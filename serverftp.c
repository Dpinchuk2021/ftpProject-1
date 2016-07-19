/* 
 * server FTP program
 *
 * NOTE: Starting homework #2, add more comments here describing the overall function
 * performed by server ftp program
 * This includes, the list of ftp commands processed by server ftp.
 *
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define SERVER_FTP_PORT 14227
#define DATA_CONNECTION_PORT  14228


/* Error and OK codes */
#define OK 0
#define ER_INVALID_HOST_NAME -1
#define ER_CREATE_SOCKET_FAILED -2
#define ER_BIND_FAILED -3
#define ER_CONNECT_FAILED -4
#define ER_SEND_FAILED -5
#define ER_RECEIVE_FAILED -6
#define ER_ACCEPT_FAILED -7


/* Function prototypes */

int svcInitServer(int *s);
int clntConnect(char *serverName, int *s);
int sendMessage (int s, char *msg, int  msgSize);
int receiveMessage(int s, char *buffer, int  bufferSize, int *msgSize);


/* List of all global variables */

char userCmd[1024];	    /* user typed ftp command line received from client */
char userCmdCopy[1024]; /* used for temporary String manipulation. */
char chgCmd[1024];      /* used to change a command into a UNIX-recognizable command */
char *cmd;   		    /* ftp command (without argument) extracted from userCmd */
char *argument;         /* argument (without ftp command) extracted from userCmd */
char replyMsg[1024];    /* buffer to send reply message to client */
char ftpData[100];		/* buffer to send/receive file data to/from client */
int  ftpBytes      = 0; /* Used to count the total number of bytes transferred during ftp */
int  fileBytesRead = 0; /* The number of bytes read by fread */
int  bytesReceived = 0; /* The number of bytes received in a single ftp message. */

char *userList[]  = {"kchin", "Joe", "Jim", "Mary"}; /* The list of valid user names */
char *passList[]  = {"0", "1", "2", "3"};            /* The list of passwords associated with the index of userList */
int  isLoggedIn   = -1;                              /* -1 (default), 0 if the User is not logged in, 1 otherwise. */
int  userIndex    = -1;                              /* The index of userList that identifies the current user. See Login Codes. */

FILE *filePtr;                                       /* Used to point to the temporary file that logged command output */
int  bytesRead    = -1;                              /* The number of bytes read by fread() */
char fileData[1024];                                 /* Used to store the byte data obtained by fread(). Max size = 1024 bytes per in-class discussion */

/* Login Codes */
#define LOGGED_IN 1
#define LOGGED_OUT 0
#define NO_USER -1

/*
 * main
 *
 * Function to listen for connection request from client
 * Receive ftp command one at a time from client
 * Process received command
 * Send a reply message to the client after processing the command with staus of
 * performing (completing) the command
 * On receiving QUIT ftp command, send reply to client and then close all sockets
 *
 * Parameters
 * argc		- Count of number of arguments passed to main (input)
 * argv  	- Array of pointer to input parameters to main (input)
 *		   It is not required to pass any parameter to main
 *		   Can use it if needed.
 *
 * Return status
 *	0			- Successful execution until QUIT command from client 
 *	ER_ACCEPT_FAILED	- Accepting client connection request failed
 *	N			- Failed stauts, value of N depends on the command processed
 */

int main(	
	int argc,
	char *argv[]
	)
{
	/* List of local varibale */

	int msgSize;        /* Size of msg received in octets (bytes) */
	int listenSocket;   /* listening server ftp socket for client connect request */
	int ccSocket;       /* Control connection socket - to be used in all client communication */
	int dcSocket;       /* Data connection socket - to be used in all server communication */
	int status;


	/*
	 * NOTE: without \n at the end of format string in printf,
         * UNIX will buffer (not flush)
	 * output to display and you will not see it on monitor.
	*/
	printf("Started execution of server ftp\n");


	 /*initialize server ftp*/
	printf("Initialize ftp server\n");	/* changed text */

	status=svcInitServer(&listenSocket);
	if(status != 0)
	{
		printf("Exiting server ftp due to svcInitServer returned error\n");
		exit(status);
	}


	printf("ftp server is waiting to accept connection\n");

	/* wait until connection request comes from client ftp */
	ccSocket = accept(listenSocket, NULL, NULL);

	printf("Came out of accept() function \n");

	if(ccSocket < 0)
	{
		perror("cannot accept connection:");
		printf("Server ftp is terminating after closing listen socket.\n");
		close(listenSocket);  /* close listen socket */
		return (ER_ACCEPT_FAILED);  // error exist
	}

	printf("Connected to client, calling receiveMsg to get ftp cmd from client \n");


	/* Receive and process ftp commands from client until quit command.
 	 * On receiving quit command, send reply to client and 
         * then close the control connection socket "ccSocket". 
	 */
	do
	{
		userCmd[0] = NULL;
		status = 1;
		printf("Top of main do loop.\n"); /* TEST */
		printf("status: %d.\n", status); /* TEST */
	    /* Receive client ftp commands until */
 	    status=receiveMessage(ccSocket, userCmd, sizeof(userCmd), &msgSize);
	    if(status < 0)
	    {
		printf("Receive message failed. Closing control connection \n");
		printf("Server ftp is terminating.\n");
		break;
	    }


/*
 * Starting Homework#2 program to process all ftp commandsmust be added here.
 * See Homework#2 for list of ftp commands to implement.
 */
	
		/* Re-initialize the first element of cmd and argument */
		cmd      = NULL;
		argument = NULL;
		
		printf("Message received. status: %d.\n", status); /* TEST */
		if(strcmp(userCmd, NULL) == 0){
			printf("userCmd is NULL.\n"); /* TEST */
		}
		else if(strcmp(userCmd, "") == 0){
			printf("userCmd is the empty set.\n"); /* TEST */
		}
		printf("cmd and arg initialized.\n"); /* TEST */
		printf("cmd: %s.\n", cmd);
		printf("arg: %s.\n", argument);
		
		/* Separate command and argument from userCmd */
		strcpy(userCmdCopy, userCmd);
		cmd = strtok(userCmdCopy, " ");
		argument = strtok(NULL, " ");
		
		printf("Received command: %s.\n", userCmd);
		printf("cmd: %s.\n", cmd);
		printf("arg: %s.\n", argument);
		
		/* Process the help command */
		if(strcmp(cmd, "help") == 0){
			strcpy(replyMsg,"214 cd dele help ls mkdir pass pwd quit rmdir stat user\n");  /* All implemented commands */
		}
		
		/* Proccess the user command. Logout(flush) current user if a user is logged in (per http://www.ietf.org/rfc/rfc0959.txt).
		 * Set userIndex, if the user is valid.
		 */
		else if(strcmp(cmd, "user") == 0){
			isLoggedIn = NO_USER;
			userIndex  = -1;
			if(argument[0] == NULL || strcmp(argument, "") == 0){
				strcpy(replyMsg,"501 No username given. Usage: \"user <username>\".\n");
				userIndex = -1;
			}
			else{
				/* check for matching user */
				for(int i = 0; i < (sizeof(userList)/sizeof(char*)); i++){
					if(strcmp(userList[i], argument) == 0){
						userIndex = i;
						isLoggedIn = LOGGED_OUT;
						strcpy(replyMsg,"331 User name okay, need password.\n");
						break;
					}
				} /* end i */
				/* The specified user was not found. */
				if(userIndex < 0){
					strcpy(replyMsg,"530 Invalid username. Not logged in.\n");
				}
			}
		}
		
		/* Proccess the pass command. Must be preceded by a successful use of the user command.
		 * If a user is already logged in, but supplies an invalid password, the user is logged out.
		 * If the user provides a valid password, the user is flagged as logged in.
		 */
		else if(strcmp(cmd, "pass") == 0){
			/* If the user did not provide a password, notify user of bad syntax. */
			if(argument[0] == NULL || strcmp(argument, "") == 0){
				strcpy(replyMsg,"501 Syntax error. Use: \"pass <password>\".\n");
			}
			/* If the user did not successfully use the "user" command, notify user to use choose a user name. */
			else if(isLoggedIn == NO_USER || userIndex < 0){
				strcpy(replyMsg,"503 Bad sequence of commands. Use \"user <username>\" first.\n");
			}
			/* Successful user/password match. Set isLoggedIn and send reply */
			else if(strcmp(passList[userIndex], argument) == 0){
				isLoggedIn = LOGGED_IN;
				strcpy(replyMsg,"230 User logged in, proceed.\n");
			}
			/* Bad password. userIndex is preserved, but isLoggedIn is ensured as LOGGED_OUT. */
			else{
				isLoggedIn = LOGGED_OUT;
				if(userIndex >= 0){
					strcpy(replyMsg,"530 Invalid password for ");
					strcat(replyMsg, userList[userIndex]);
					strcat(replyMsg, ". Use \"pass\" command to try again, or \"user\" command to switch users.\n");
				}
				/* This should never occur. Notify user of error. Shutdown system. */
				else{
					strcpy(replyMsg,"421 An error occurred. Please contact the system admin.");
					strcpy(cmd, "quit");
				}
			}
		}
	
		/* Process the stat command */
		else if(strcmp(cmd, "stat") == 0 || strcmp(cmd, "status") == 0){
			strcpy(replyMsg,"211 Transfer mode is ASCII.\n"); /* per in-class discussion (Can't fail)*/
		}
		
		/* Allow usage of further commands if a valid user is logged in. */
		
		/* Process the mkdir command */
		else if(strcmp(cmd, "mkdir") == 0 || strcmp(cmd, "mkd") == 0){
			if(argument[0] == NULL || strcmp(argument, "") == 0){
					strcpy(replyMsg,"501 Syntax error. Use: \"mkdir <directory_path>\".\n");
			}
			else if(isLoggedIn == LOGGED_IN){
				if(strcmp(cmd, "mkd") == 0){
					strcpy(chgCmd, "mkdir ");
					strcat(chgCmd, argument);
					status = system(chgCmd);
				}
				else{
					status = system(userCmd);
				}
				if(status == 0){
					strcpy(replyMsg, "257 Created the directory \"");
					strcat(replyMsg, argument);
					strcat(replyMsg, "\".\n");
				}
				else{
					strcpy(replyMsg, "500 Unable to create \"");
					strcat(replyMsg, argument);
					strcat(replyMsg, "\".\n");
				}
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
		
		/* Process the rmdir command */
		else if(strcmp(cmd, "rmdir") == 0 || strcmp(cmd, "rmd") == 0){
			if(argument[0] == NULL || strcmp(argument, "") == 0){
					strcpy(replyMsg,"501 Syntax error. Use: \"rmdir <directory_path>\".\n");
			}
			else if(isLoggedIn == LOGGED_IN){
				if(strcmp(cmd, "rmd") == 0){
					strcpy(chgCmd, "rmdir ");
					strcat(chgCmd, argument);
					status = system(chgCmd);
				}
				else{
					status = system(userCmd);
				}
				if(status == 0){
					strcpy(replyMsg, "250 Removed the directory \"");
					strcat(replyMsg, argument);
					strcat(replyMsg, "\".\n");
				}
				else{
					strcpy(replyMsg, "500 Unable to remove \"");
					strcat(replyMsg, argument);
					strcat(replyMsg, "\".\n");
				}
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
	
		/* Process the dele command */
		else if(strcmp(cmd, "dele") == 0){
			if(argument[0] == NULL || strcmp(argument, "") == 0){
					strcpy(replyMsg,"501 Syntax error. Use: \"dele <file_path>\".\n");
			}
			else if(isLoggedIn == LOGGED_IN){
				strcpy(chgCmd, "rm ");
				strcat(chgCmd, argument);
				status = system(chgCmd);
				if(status == 0){
					strcpy(replyMsg, "250 Removed the file \"");
					strcat(replyMsg, argument);
					strcat(replyMsg, "\".\n");
				}
				else{
					strcpy(replyMsg, "500 Unable to remove \"");
					strcat(replyMsg, argument);
					strcat(replyMsg, "\".\n");
				}
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
		
		/* Process the cd command */
		else if(strcmp(cmd, "cd") == 0){
			if(isLoggedIn == LOGGED_IN){
				/* Home is the login directory of the user who ran this program. */
				if(argument[0] == NULL || strcmp(argument, "") == 0){
					status = chdir(getenv("HOME"));
					strcpy(argument, "HOME directory");
				}else{
					status = chdir(argument);
				}
				if(status == 0){
					strcpy(replyMsg, "250 Current working directory changed to \"");
					strcat(replyMsg, argument);
					strcat(replyMsg, "\".\n");
				}
				else{
					strcpy(replyMsg, "500 Unable to change to the specified directory.\n");
				}
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
			
		/* Process the pwd command */
		else if(strcmp(cmd, "pwd") == 0){
			if(isLoggedIn == LOGGED_IN){
				status = system("pwd > ./serverftp_temp");
				if(status == 0){
					filePtr = fopen("serverftp_temp", "r");
					/* If the file doesn't exist or couldn't be opened, notify user of failure. */
					if(filePtr == NULL){
						strcpy(replyMsg, "500 Unable to display current working directory (filePtr missing).\n");
					}
					/* Read output file. */
					else{
						bytesRead = fread(fileData, 1, 1024, filePtr);
						/* If the file is empty, something went wrong. Notify user of failure. */
						if(bytesRead <= 0){
							strcpy(replyMsg, "500 Unable to display current directory (read error).\n");
						}
						/* File read successful. Send user the server response. */
						else{
							fileData[bytesRead] = NULL; /* NULL-terminate the file data */
							strcpy(replyMsg, "257 ");
							strcat(replyMsg, fileData);
							strcat(replyMsg, "\n");
						}
					}
					/* Clean up */
					fclose(filePtr);
					system("rm ./serverftp_temp");
				}
				/* Output file could not be created. Notify user of failure */
				else{
					strcpy(replyMsg, "500 Unable to display current directory (can not process command).\n");
				}
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
		
		/* Process the ls command */
		else if(strcmp(cmd, "ls") == 0){
			if(isLoggedIn == LOGGED_IN){
				status = system("ls > ./serverftp_temp");
				if(status == 0){
					filePtr = fopen("./serverftp_temp", "r");
					/* If the file doesn't exist or couldn't be opened, notify user of failure. */
					if(filePtr == NULL){
						strcpy(replyMsg, "451 Unable to display directory contents (filePtr missing).\n");
					}
					/* Read output file. */
					else{
						bytesRead = fread(fileData, 1, 1024, filePtr);
						/* Unable to read file. Notify user of failure. */
						if(bytesRead < 0){
							strcpy(replyMsg, "451 Unable to display directory contents (read error).\n");
						}
						/* File read successful. Send user the server response. */
						else{
							if(bytesRead == 0){
								strcpy(fileData, "The current directory is empty.");
							}
							fileData[bytesRead] = NULL; /*NULL-terminate the file data */
							strcpy(replyMsg, "\n");
							strcat(replyMsg, fileData);
							strcat(replyMsg, "\n250 Requested file action okay, completed.\n");
						}
					}
					/* Clean up */
					fclose(filePtr);
					system("rm ./serverftp_temp");
				}
				/* Output file could not be created. Notify user of failure */
				else{
					strcpy(replyMsg, "451 Unable to display directory contents (can not process command).\n");
				}
			}
			else{
				strcpy(replyMsg, "530 Not logged in.\n");
			}
		}
		
		/* Process the 'send' command. */
		else if(strcmp(cmd, "send") == 0){
		
			/* Attempt data connection despite any error to prevent client from endlessly listening. */
			printf("Calling clntConnect to connect to the client.\n");
			status=clntConnect("192.168.200.230", &dcSocket);  /* Off-campus IP: "143.241.37.230" */
			if(status != 0){
				/* Data connection failed. Reply to client. Close data connection socket. */
				strcpy(replyMsg, "425 'send' could not open data connection. Closing data connection socket.\n");
			}
			else{
				/* Data connection established. Ensure user is logged in and provided a filename. */
				printf("Data connection established to client.\n");
				if(argument[0] == NULL || strcmp(argument, "") == 0){
					/* Invalid argument. Throw away any message received on dcSocket */
				
					strcpy(replyMsg, "501 Invalid syntax. Use: \"send <filename>\". Closing data connection.\n");
				}
				else if(isLoggedIn == LOGGED_IN){
					/* User is logged in. Ensure the specified file can be written to. */
					filePtr = NULL;
					filePtr = fopen(argument, "w");
					if(filePtr == NULL){
						/* Can't write to file. Throw away any message received on dcSocket */
						strcpy(replyMsg, "550 The file \"");
						strcat(replyMsg, argument);
						strcat(replyMsg, "\" could not be opened/created. Closing data connection.\n");
					}else{
						/* File can be written. Begin receiving file until no bytes received or no message can be retrieved. */
						ftpBytes = 0; /* initialize byte count */
						printf("Waiting for file transmission from client.\n");
						do{
							bytesReceived = 0;
							status = receiveMessage(dcSocket, ftpData, sizeof(ftpData), &bytesReceived);
							fwrite(ftpData, 1, bytesReceived, filePtr); /* Success or fail, fwrite returns >= 0. This info is useless. */
							ftpBytes = ftpBytes + bytesReceived;
						}while(bytesReceived > 0 && status == OK); /* end data connection read loop */
					
						sprintf(replyMsg, "226 Received %d", ftpBytes);
						strcat(replyMsg, " bytes. Closing data connection.\n");
					}
					fclose(filePtr); /* Close the file, whether it was received or not. */
				}
				else{
					/* User not logged in. Throw away any message received on dcSocket */
					strcpy(replyMsg, "530 Not logged in. Closing data connection.\n");
				}
			}
			printf("Data connection closed.\n");
			close(dcSocket); /* Close the data connection, whether an error occurred or not. */
		}
		
		/* Process the 'recv' command. */
		else if(strcmp(cmd, "recv") == 0){
			/* Attempt data connection despite any error to prevent client from endlessly listening. */
			printf("Calling clntConnect to connect to the client.\n");
			status=clntConnect("192.168.200.230", &dcSocket);  /* Off-campus IP: "143.241.37.230" */
			if(status != 0){
				/* Data connection failed. Reply to client. Close data connection socket. */
				strcpy(replyMsg, "425 'recv' could not establish data connection. Closing data connection socket.\n");
			}else{
				printf("Data connection to client successful.\n");
				/* Data connection successful. Ensure user is logged in and provided a filename. */
				if(argument[0] == NULL || strcmp(argument, "") == 0){
					strcpy(replyMsg, "501 Invalid syntax. Use: \"send <filename>\". Closing Data connection.\n");
				}
				else if(isLoggedIn == LOGGED_IN){
				
					/* Ensure the specified file can be read. */
					filePtr = NULL;
					filePtr = fopen(argument, "r");
					if(filePtr == NULL){
						strcpy(replyMsg, "550 The file \"");
						strcat(replyMsg, argument);
						strcat(replyMsg, "\" could not be opened. Closing data connection.");
					}
					else{
						/* File is readable. Begin sending file until EOF. */
						ftpBytes = 0; /* initialize byte count */
						printf("Sending file.\n");
						do{
							fileBytesRead = 0;
							fileBytesRead = fread(ftpData, 1, 100, filePtr); /* Read error returns number of bytes read. Can not distinguish error. */
							status = sendMessage(dcSocket, ftpData, fileBytesRead); /* Do not alter file data. Do not add an element for NULL. */
							ftpBytes = ftpBytes + fileBytesRead;
						}while(!feof(filePtr) && status == OK); /* End send loop */
						
						 sprintf(replyMsg, "226 Sent %d", ftpBytes);
						/* strcpy(replyMsg, "226 Sent \n"); */
						strcat(replyMsg, " bytes. Closing data connection.");
					}
					fclose(filePtr); /* Close the file, whether it was sent or not. */
				}
				else{
					strcpy(replyMsg, "530 Not logged in. Closing data connection.\n");
				}
			}
			close(dcSocket); /* Close the data connection, whether an error occurred or not. */
		}
		
		
		/* Send response to quit command */
		else if(strcmp(cmd, "quit") == 0){
			strcpy(replyMsg, "221 Service closing data and control connections.\n");
		}
		
		/* cmd is not a valid command */
		else{
			strcpy(replyMsg, "502 Command not implemented. Use \"help\" for a list of valid commands.\n");
		}
		
	    /*
 	     * ftp server sends only one reply message to the client for 
	     * each command received in this implementation.
	     */
	    status=sendMessage(ccSocket,replyMsg,strlen(replyMsg) + 1);	/* Added 1 to include NULL character in */
				/* the reply string strlen does not count NULL character */
	    if(status < 0)
	    {
		break;  /* exit while loop */
	    }
	}
	while(strcmp(cmd, "quit") != 0);
	
	printf("Closing data connection socket.\n");
	close (dcSocket);  /* Close server data connection socket */

	printf("Closing control connection socket.\n");
	close (ccSocket);  /* Close client control connection socket */

	printf("Closing listen socket.\n");
	close(listenSocket);  /*close listen socket */

	printf("Existing from server ftp main. \n");

	return (status);
}


/*
 * svcInitServer
 *
 * Function to create a socket and to listen for connection request from client
 *    using the created listen socket.
 *
 * Parameters
 * s		- Socket to listen for connection request (output)
 *
 * Return status
 *	OK			- Successfully created listen socket and listening
 *	ER_CREATE_SOCKET_FAILED	- socket creation failed
 */

int svcInitServer (
	int *s 		/*Listen socket number returned from this function */
	)
{
	int sock;
	struct sockaddr_in svcAddr;
	int qlen;

	/*create a socket endpoint */
	if( (sock=socket(AF_INET, SOCK_STREAM,0)) <0)
	{
		perror("cannot create socket");
		return(ER_CREATE_SOCKET_FAILED);
	}

	/*initialize memory of svcAddr structure to zero. */
	memset((char *)&svcAddr,0, sizeof(svcAddr));

	/* initialize svcAddr to have server IP address and server listen port#. */
	svcAddr.sin_family = AF_INET;
	svcAddr.sin_addr.s_addr=htonl(INADDR_ANY);  /* server IP address */
	svcAddr.sin_port=htons(SERVER_FTP_PORT);    /* server listen port # */

	/* bind (associate) the listen socket number with server IP and port#.
	 * bind is a socket interface function 
	 */
	if(bind(sock,(struct sockaddr *)&svcAddr,sizeof(svcAddr))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}

	/* 
	 * Set listen queue length to 1 outstanding connection request.
	 * This allows 1 outstanding connect request from client to wait
	 * while processing current connection request, which takes time.
	 * It prevents connection request to fail and client to think server is down
	 * when in fact server is running and busy processing connection request.
	 */
	qlen=1; 


	/* 
	 * Listen for connection request to come from client ftp.
	 * This is a non-blocking socket interface function call, 
	 * meaning, server ftp execution does not block by the 'listen' funcgtion call.
	 * Call returns right away so that server can do whatever it wants.
	 * The TCP transport layer will continuously listen for request and
	 * accept it on behalf of server ftp when the connection requests comes.
	 */

	listen(sock,qlen);  /* socket interface function call */

	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /*successful return */
}

/*
 * clntConnect
 *
 * Copied from clientftp.c.
 *
 * Function to create a socket, bind local client IP address and port to the socket
 * and connect to the client.
 *
 * Parameters
 * serverName	- IP address of client in dot notation (input)
 * s		    - Control connection socket number (output)
 *
 * Return status
 *	OK			- Successfully connected to the client
 *	ER_INVALID_HOST_NAME	- Invalid client name
 *	ER_CREATE_SOCKET_FAILED	- Cannot create socket
 *	ER_BIND_FAILED		- bind failed
 *	ER_CONNECT_FAILED	- connect failed
 */
int clntConnect (
	char *serverName, /* client IP address in dot notation (input) */
	int  *s 		  /* control connection socket number (output) */
	)
{
	int sock;	/* local variable to keep socket number */

	struct sockaddr_in clientAddress;  	/* local server IP address */
	struct sockaddr_in serverAddress;	/* client IP address */
	struct hostent	   *serverIPstructure;	/* host entry having server IP address in binary */


	/* Get IP address of client in binary from client name (IP in dot notation) */
	if((serverIPstructure = gethostbyname(serverName)) == NULL)
	{
		printf("%s is unknown server. \n", serverName);
		return (ER_INVALID_HOST_NAME);  /* error return */
	}

	/* Create control connection socket */
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("cannot create socket ");
		return (ER_CREATE_SOCKET_FAILED);	/* error return */
	}

	/* initialize server address structure memory to zero */
	memset((char *) &clientAddress, 0, sizeof(clientAddress));

	/* Set local server IP address, and port in the address structure */
	clientAddress.sin_family = AF_INET;	/* Internet protocol family */
	clientAddress.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY is 0, which means */
						 /* let the system fill server IP address */
	clientAddress.sin_port = 0;  /* With port set to 0, system will allocate a free port */
			  /* from 1024 to (64K -1) */

	/* Associate the socket with local server IP address and port */
	if(bind(sock,(struct sockaddr *)&clientAddress,sizeof(clientAddress))<0)
	{
		perror("cannot bind");
		close(sock);
		return(ER_BIND_FAILED);	/* bind failed */
	}


	/* Initialize serverAddress memory to 0 */
	memset((char *) &serverAddress, 0, sizeof(serverAddress));

	/* Set ftp client ftp address in serverAddress */
	serverAddress.sin_family = AF_INET;
	memcpy((char *) &serverAddress.sin_addr, serverIPstructure->h_addr, 
			serverIPstructure->h_length);
	serverAddress.sin_port = htons(DATA_CONNECTION_PORT);

	/* Connect to the client */
	if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
	{
		perror("Cannot connect to client ");
		close (sock); 	/* close the control connection socket */
		return(ER_CONNECT_FAILED);  	/* error return */
	}


	/* Store listen socket number to be returned in output parameter 's' */
	*s=sock;

	return(OK); /* successful return */
}  // end of clntConnect() */


/*
 * sendMessage
 *
 * Function to send specified number of octet (bytes) to client ftp
 *
 * Parameters
 * s		- Socket to be used to send msg to client (input)
 * msg  	- Pointer to character arrary containing msg to be sent (input)
 * msgSize	- Number of bytes, including NULL, in the msg to be sent to client (input)
 *
 * Return status
 *	OK		- Msg successfully sent
 *	ER_SEND_FAILED	- Sending msg failed
 */

int sendMessage(
	int    s,	/* socket to be used to send msg to client */
	char   *msg, 	/* buffer having the message data */
	int    msgSize 	/* size of the message/data in bytes */
	)
{
	int i;


	/* Print the message to be sent byte by byte as character */
	for(i=0; i<msgSize; i++)
	{
		printf("%c",msg[i]);
	}
	printf("\n");

	if((send(s, msg, msgSize, 0)) < 0) /* socket interface call to transmit */
	{
		perror("unable to send ");
		return(ER_SEND_FAILED);
	}

	return(OK); /* successful send */
}


/*
 * receiveMessage
 *
 * Function to receive message from client ftp
 *
 * Parameters
 * s		- Socket to be used to receive msg from client (input)
 * buffer  	- Pointer to character arrary to store received msg (input/output)
 * bufferSize	- Maximum size of the array, "buffer" in octent/byte (input)
 *		    This is the maximum number of bytes that will be stored in buffer
 * msgSize	- Actual # of bytes received and stored in buffer in octet/byes (output)
 *
 * Return status
 *	OK			- Msg successfully received
 *	R_RECEIVE_FAILED	- Receiving msg failed
 */


int receiveMessage (
	int s, 		/* socket */
	char *buffer, 	/* buffer to store received msg */
	int bufferSize, /* how large the buffer is in octet */
	int *msgSize 	/* size of the received msg in octet */
	)
{
	int i;

	*msgSize=recv(s,buffer,bufferSize,0); /* socket interface call to receive msg */

	if(*msgSize<0)
	{
		perror("unable to receive");
		return(ER_RECEIVE_FAILED);
	}

	/* Print the received msg byte by byte */
	for(i=0;i<*msgSize;i++)
	{
		printf("%c", buffer[i]);
	}
	printf("\n");

	return (OK);
}



