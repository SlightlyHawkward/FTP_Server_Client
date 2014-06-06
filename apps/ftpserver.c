/* ftpserver.c */

#include <stdlib.h>
#include <stdio.h>
//New stuff
#include <unistd.h>
#include <string.h>
#include <stdint.h>
//New Stuff: Lab04
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define BUFFSIZE		1000000

/*-----------------------------------------------------------------------
 *
 * Program: ftpserver
 * Purpose: wait for a connection from an ftpclient and Send or retrieve data
 * Usage:   ftpserver <appnum>
 *
 *-----------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	int		len, expect;
	char		buff[BUFFSIZE];
	char serverReply[BUFFSIZE];
	char command[10];
	char filename[1024];
	char string[BUFFSIZE]; //Data stream

	struct servent	*pse;	/* pointer to service information entry	*/
	struct protoent *ppe;	/* pointer to protocol information entry*/
	struct sockaddr_in sin;	/* an Internet endpoint address		*/
	int	s, type;	/* socket descriptor and socket type	*/
	int preSock;
	struct sockaddr_in	sockaddr, csockaddr;
	int qlen = 1;
	
	if (argc != 2) {
		(void) fprintf(stderr, "usage: %s <appnum>\n", argv[0]);
		exit(1);
	}

	/* wait for a connection from an echo client */

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

    /* Map service name to port number */
	sin.sin_port = htons((unsigned short)atoi(argv[1]));

    /* Map protocol name to protocol number */
	ppe = getprotobyname("tcp");

    /* Use protocol to choose a socket type */
	type = SOCK_STREAM;

    /* Allocate a socket */
	preSock = socket(PF_INET, type, ppe->p_proto);
	
	if (preSock < 0){
		(void) printf("Connection not found\n");
		exit(1);
	}

    /* Bind the socket */
	if (bind(preSock, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		(void) printf("Bind failed\n");
		exit(1);
	}

	if (type == SOCK_STREAM && listen(preSock, qlen) < 0) {
		(void) printf("Listen failed\n");
		exit(1);
	}
	/*Accept socket connection*/
	
	int csockaddrlen = sizeof(struct sockaddr_in);
	s = accept(preSock, (struct sockaddr *) &csockaddr, &csockaddrlen);

	/* Loop, waiting for data/queries */
	
	(void)printf("Server now listening\n");
	
	while((len = recvln(s, buff, BUFFSIZE)) > 0){ /*Changed to recvln to avoid the seizing up problem that was occuring.*/
		/*Got first recieve, acquired things*/
		sscanf(buff, "%s %s\n", command, filename);
		/*If appropriate, parse additional information*/
		if(strcmp(command, "GET") == 0){ //GET command recieved
			
			if(access(filename, R_OK) != 0){ //File not found
				strcpy(serverReply, "GETRESPONSE FAILED_FILE_NOT_FOUND ");
				strcat(serverReply, filename);
				strcat(serverReply, "\n");
				(void) send(s, serverReply, strlen(serverReply), 0);
			} else { //GETRESPONSE OKAY
				
				/*Assemble data for first send, send response*/
				strcpy(serverReply, "GETRESPONSE OKAY ");
				strcat(serverReply, filename);
				strcat(serverReply, "\n");
				(void) send(s, serverReply, strlen(serverReply), 0);
				

				/*Put together the data string, acquire length*/	
				FILE * filepointer = fopen(filename, "r");
				int readChar;
				int counter = 0;
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

				//Send the last two portions of data, the size and the data itself.
				(void) send(s, binaryString, sizeof(uint32_t), 0);
				(void) send(s, string, lengthofdata, 0);
				
				//(void) printf(RECEIVED_PROMPT);
				(void) fflush(stdout);
			}
		} else if(strcmp(command, "PUT") == 0){ // PUT command recieved
			if(access(filename, R_OK) == 0){ //File already exists
				strcpy(serverReply, "PUTRESPONSE FAILED_CANNOT_CREATE_FILE ");
				strcat(serverReply, filename);
				strcat(serverReply, "\n");
				(void) send(s, serverReply, strlen(serverReply), 0);
				
				/*Flush the server reply buffer*/
				char * emptyme[sizeof(uint32_t)];
				len = recv(s, emptyme, sizeof(uint32_t), 0);
		
				/*Convert length back*/
				uint32_t totranslate, somethingelse;
				memcpy(&totranslate, emptyme, sizeof(totranslate));
				somethingelse = ntohl(totranslate);
		
				/*Get data, print to file*/
		
				int received;
				expect = (int)somethingelse; /*Check this*/
				for (received = 0; received < expect;) {
					len = recv(s, buff, (expect - received) < BUFFSIZE ?
					(expect - received) : BUFFSIZE, 0);
					if (len < 0) {
					shutdown(s, 2);
					shutdown(preSock, 2);
					return 1;
				}
				received += len;
				}
				
			} else { //PUTRESPONSE OKAY
				//Get length
				char * unconvertedResponse[sizeof(uint32_t)];
				len = recv(s, unconvertedResponse, sizeof(uint32_t), 0);
				
				/*Convert length back*/
				uint32_t translatedInt, convertedResponseLength;
				memcpy(&translatedInt, unconvertedResponse, sizeof(translatedInt));
				convertedResponseLength = ntohl(translatedInt);
				/*Get data, print to file*/
				FILE * file;
				file = fopen(filename, "wb+");
				int received;
				expect = (int)convertedResponseLength; /*Check this*/
				for (received = 0; received < expect;) {
					len = recv(s, buff, (expect - received) < BUFFSIZE ?
						(expect - received) : BUFFSIZE, 0);
					if (len < 0) {
						shutdown(s, 2);
						shutdown(preSock, 2);
						return 1;
					}
					(void) fwrite(buff, sizeof(char), len, file);
					received += len;
				}						
				fclose(file);
				/*Print server response and send after file is done*/
				strcpy(serverReply, "PUTRESPONSE OKAY ");
				strcat(serverReply, filename);
				strcat(serverReply, "\n");
				
				(void) send(s, serverReply, strlen(serverReply), 0);
				
			}
		}
	
			

	}
		
		(void) send(s, buff, len, 0);
	shutdown(s, 2);
	shutdown(preSock, 2);
	return 0;
}
