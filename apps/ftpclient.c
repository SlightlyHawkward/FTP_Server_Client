/* ftpclient.c */

#include <stdlib.h>
#include <stdio.h>
//New stuff
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdint.h>

//New Stuff: Lab04
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define BUFFSIZE		1000000
#define INPUT_PROMPT		"Input   > "
#define RECEIVED_PROMPT		"Received> "

int readln(char *, int);

/*-----------------------------------------------------------------------
 *
 * Program: ftpclient
 * Purpose: contact ftpserver, send or request files
 * Usage:   echoclient <compname> [appnum]
 * Note:    Appnum is optional. If not specified the standard echo appnum
 *          (7) is used.
 *
 *-----------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	
	char		buff[BUFFSIZE];
	char 		string[BUFFSIZE];
	int		expect, received, len;
	char getfile[4] = "GET"; 
	char putfile[4] = "PUT";
	
	char command[10]; //Command storage
	char filename[1024]; //Filename Storage
	FILE * filepointer;

	struct hostent	*phe;	/* pointer to host information entry	*/
	struct servent	*pse;	/* pointer to service information entry	*/
	struct protoent *ppe;	/* pointer to protocol information entry*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, type;	/* socket descriptor and socket type	*/

	if (argc < 2 || argc > 3) {
		(void) fprintf(stderr, "usage: %s <compname> [appnum]\n",
			       argv[0]);
		exit(1);
	}

	/* convert the arguments to binary format comp and appnum */

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

    /* Map service name to port number */
	sin.sin_port = htons((unsigned short)atoi(argv[2])); //Convert app num

    /* Map host name to IP address, allowing for dotted decimal */
	if ( phe = gethostbyname(argv[1]) )
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	
    /* Map transport protocol name to protocol number */
	ppe = getprotobyname("tcp");

    /* Allocate a socket */
	s = socket(AF_INET, SOCK_STREAM, ppe->p_proto);

    /* Connect the socket */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		exit(1);
	}
	
	(void) printf(INPUT_PROMPT);
	(void) fflush(stdout);

	/* iterate: read input from the user, send to the server,	*/
	/*	    receive reply from the server, and display for user */
	while((len = readln(buff, BUFFSIZE)) > 0) {
		sscanf(buff, "%s %s\n", command, filename);
		/*Parse inputs...*/
		if(strcmp(command, putfile) == 0){ // 'PUT'found
			
			if (access(filename, R_OK) != 0){ // returns zero if file exists
				(void) printf("FILE_NOT_EXIST: %s\n", filename);
			
			} else {  // PUT the file through
				/*Cat data on to buff, send it through*/

				filepointer = fopen(filename, "r");
				int readChar;
				int counter = 0;

				/*Put together the buffer*/				
				while ((readChar = fgetc(filepointer)) != EOF){
					string[counter] = (char)readChar;
					counter++;
				}
				fclose(filepointer);
				int lengthofdata = strlen(string);

				/*Endianness stuff, convert to a char buffer so it goes through*/
				uint32_t translatedLength, lengthtocat;
				translatedLength = htonl(lengthofdata);
				char binaryString[sizeof(translatedLength)];
				memcpy(binaryString, &translatedLength, sizeof(translatedLength));
				
				/*To translate back:
					uint32_t translatedInt, originalInt;
					memcpy(&translatedInt, binaryString, sizeof(translatedInt));
					originalInt = ntohl(translatedInt);
				*/

				//send everything 
				(void) send(s, buff, strlen(buff), 0);
				(void) send(s, binaryString, sizeof(uint32_t), 0); //Length of data
				(void) send(s, string, lengthofdata, 0); //data
				
				(void) fflush(stdout);
				/* Get response */
				len = recvln(s, buff, BUFFSIZE);
				(void) write(STDOUT_FILENO, buff, len); // Write response to client
				/*Other than for the sake of printing it, we don't care what it says*/				

			} // end else
		} // end if
	
		else if(strcmp(command, getfile) == 0){ // 'GET'found
			if (access(filename, R_OK) == 0){ //Somehow getting stuck here...
				(void) printf("FILE_ALREADY_EXISTS: %s\n", filename);
			} else {  // Send command to request file, wait until response occurs
				/* send the get command to the echoserver */
				(void) send(s, buff, len, 0);
				//(void) printf(RECEIVED_PROMPT);
				(void) fflush(stdout);
				
				/*Wait for the server response*/
				len = recvln(s, buff, BUFFSIZE); /*Server response*/
				(void) printf("\n");
				(void) write(STDOUT_FILENO, buff, len); // Write response to client
				char response[200];
				sscanf(buff, "%*s %s %*s\n", response); /*Parse response*/
				
				if(strcmp(response, "OKAY") == 0){
					
					/*Get size*/
					char * unconvertedResponse[sizeof(uint32_t)];
					len = recv(s, unconvertedResponse, sizeof(uint32_t), 0);
					
					/*Convert length back*/
					uint32_t translatedInt, convertedResponseLength;
					memcpy(&translatedInt, unconvertedResponse, sizeof(translatedInt));
					convertedResponseLength = ntohl(translatedInt);
					
					/*Get data, print to file*/
					FILE * file;
					file = fopen(filename, "wb+");
					expect = (int)convertedResponseLength; /*Check this*/
					for (received = 0; received < expect;) {
						len = recv(s, buff, (expect - received) < BUFFSIZE ?
							(expect - received) : BUFFSIZE, 0);
						if (len < 0) {
							shutdown(s, 2);
							return 1;
						}
						(void) fwrite(buff, sizeof(char), len, file);
						received += len;
					}
					fclose(file);
					
				} 
				
			
			} // end else
		
		} // end get 
		else { // Neither PUT or GET found
			(void) printf("Invalid Command\n");
		}
		/*Purge the buffer*/
		(void) printf("\n");
		(void) printf(INPUT_PROMPT);
		(void) fflush(stdout);
		memset(buff, 0, strlen(buff));
	}
	/* iteration ends when EOF found on stdin */

	(void) shutdown(s, 2);
	(void) printf("\n");
	return 0;
}
