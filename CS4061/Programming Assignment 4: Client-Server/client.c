#define _BSD_SOURCE
#define NUM_ARGS 3

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "util.h"
#include <pthread.h>




char* Handle_Votes(char* votes_file, char* requestfile){
		char* outline = (char*) malloc (sizeof(char) * 1024);
		FILE* inFD;

		chdir(changedir(requestfile));

	 	inFD = fopen(votes_file, "r");
		if (inFD ==NULL) {
			printf("Cannot find %s file.\n", votes_file);	// Satisfies output specification #1
			exit(0);
		}
		printf("%s file is open \n",votes_file );

		char buffer[1024];
		 tally_t* voteTally =  malloc(sizeof(struct tally) * 100);
		int i;
		int inArray_tf;
		while ( fgets(buffer, 1024, inFD) != NULL ) {
			trimwhitespace(buffer);
			i = 0;
			inArray_tf = 0;
			while (strlen(voteTally[i].name) != 0) {
				if( (strcmp(voteTally[i].name, buffer)==0)){
	        		voteTally[i].votes++;
	        		inArray_tf = 1;
	      		}
	      		i++;
					}
				if (inArray_tf == 1) {
				continue;
			} else {
				struct tally* newCandidate = (struct tally*)malloc(sizeof(struct tally));
				strcpy(newCandidate->name, buffer);
				newCandidate->votes = 1;
				voteTally[i] = *newCandidate;
			}
		}
		i = 0;
		char append[256];
		while (strlen(voteTally[i].name) != 0) {
			if(i==0){
				sprintf(append,"%s:%d",voteTally[i].name, voteTally[i].votes);
				strcpy(outline,append);
			 }
			 else{
				sprintf(append,"%s:%d",voteTally[i].name, voteTally[i].votes);
				strcat(outline,append);
			}
				if (strlen(voteTally[i+1].name) != 0){strcat(outline, ",");
				}
				i++;
		}
	fclose(inFD);
	printf("%s return from Handle_Votes function\n", outline );
	return trimwhitespace(outline);
}




void read_Req(int sock, char* requestfile){
	int cltsock= sock;
	int line=0;
	char* buffer= (char*) malloc(10*sizeof (char));
	char** requestLine = (char**) malloc(10*sizeof (char*));
	for(int i=0; i<10; i++){
		requestLine [i] = (char**)malloc(1024*sizeof(char*));
	}

	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("\nCurrent Directory: %s\n", cwd);
	printf("%s:is the file we are looking for\n",requestfile );

	FILE* fd = fopen(requestfile,"r");
	if (fd == NULL) {
		printf("error: the input request file doesn't exist\n");
		exit(1);
	}
	printf("\nThe input request file: %s is open\n", requestfile );


		while (fgets(buffer, 1024, fd) != NULL ) {
		//sleep(1);
		trimwhitespace(buffer);
		int req = makeargv(buffer, " ", &requestLine);

		char request_Name [strlen(requestLine[0])];

		strcpy(request_Name,trimwhitespace(requestLine[0]));
		printf("\n Analysing line: %d with the request_name: %s\n", line, request_Name );

		if (strcmp(request_Name,"Open_Polls")== 0){
			char buf[256];
			memset(buf, '0', sizeof(buf));
			trimwhitespace(requestLine[1]);

			char Region_name [15];
			strcpy(Region_name,requestLine[1]);
			int lenRegionName = strlen(requestLine[1]);
			for (int i = 0; i < (15 -lenRegionName); i++) {
				strcat(Region_name, " ");
			}

			sprintf(buf,"%s;%s;\0","OP",Region_name);
			//buf[strlen(buf) + 1]= '\0';


			int n=	send (cltsock, buf, strlen(buf), 0 );
			if ( n<0){
				 printf("Error sending Open_Polls command to the  socket\n");
			 }
				printf("%s Is the message that was sent\n", buf);
				printf("message count_votes to the socket\n");
				printf("Sending request to server:%s\n","OP");

			 ssize_t RdMSG;
			 char buf1 [256];
			 memset(buf1, '0', sizeof(buf1));
			 RdMSG = recv(cltsock,buf1,sizeof(buf1)-1, 0);
			 buf1[RdMSG]='\0';
			 if ( RdMSG<0){
					printf("Error reading from socket\n");
				}
			 printf("Received from server: %s\n", buf1);
		}

		else if(strcmp(request_Name, "Add_Votes")==0){
			char buf[256];
			memset(buf, '0', sizeof(buf));
			//trimwhitespace(requestLine[1]);
			char Region_name [15];
			strcpy(Region_name,requestLine[1]);
			int lenRegionName = strlen(requestLine[1]);
			for (int i = 0; i < (15 -lenRegionName); i++) {
				strcat(Region_name, " ");
			}
			sprintf(buf,"%s;%s;%s","AV",Region_name,Handle_Votes(requestLine[2],requestfile));


			int n =	send (cltsock, buf, strlen(buf), 0);
			if ( n<0){
				 printf("Error sending Add_Votes command to the  socket\n");
			 }
			 	printf("%s Is the message that was sent\n", buf);
			 	printf("message was sent to the socket\n");
			  printf("Sending request:%s\n", "AV");

			 ssize_t RdMSG;
			 char buf1 [256];
			 memset(buf1, '0', sizeof(buf1));
			 RdMSG = recv(cltsock,buf1,sizeof(buf1)-1, 0);
			 buf1[RdMSG]='\0';
			 if ( RdMSG<0){
					printf("Error reading from socket\n");
				}
			 printf("Received from server: %s\n", buf1);

		}


		else if(strcmp(request_Name, "Remove_Votes")==0){
			char buf[256];
			memset(buf, '0', sizeof(buf));
			trimwhitespace(requestLine[1]);
			char Region_name [15];
			strcpy(Region_name,requestLine[1]);
			int lenRegionName = strlen(requestLine[1]);
			for (int i = 0; i < (15 -lenRegionName); i++) {
				strcat(Region_name, " ");
			}
			sprintf(buf,"%s;%s;%s","RV",Region_name,Handle_Votes(requestLine[2],requestfile));
			//buf[strlen(buf) + 1]= '\0';

			int n=	send (cltsock, buf, strlen(buf), 0 );
			if ( n<0){
				 printf("Error sending Add_Votes command to the  socket\n");
			 }
				printf("%s Is the message that was sent\n", buf);
			 	printf("message was sent to the socket\n");
				printf("Sending request:%s\n", "RV");

			 ssize_t RdMSG;
 			 char buf1 [256];
 			 memset(buf1, '0', sizeof(buf1));
 			 RdMSG = recv(cltsock,buf1,sizeof(buf1)-1, 0);
 			 buf1[RdMSG]='\0';
 			 if ( RdMSG<0){
 					printf("Error reading from socket\n");
 				}
 			 printf("Received from server: %s\n", buf1);
		}

		else if(strcmp(request_Name, "Close_Polls")==0){
			char buf[256];
			memset(buf, '0', sizeof(buf));
			trimwhitespace(requestLine[1]);
			char Region_name [15];
			strcpy(Region_name,requestLine[1]);
			int lenRegionName = strlen(requestLine[1]);
			for (int i = 0; i < (15 -lenRegionName); i++) {
				strcat(Region_name, " ");
			}
			sprintf(buf,"%s;%s;","CP",Region_name);



			int n=	send (cltsock, buf, strlen(buf), 0 );
			if ( n<0){
				 printf("Error sending Close_Polls command to the  socket\n");
			 }
				printf("%s Is the message that was sent\n", buf);
				printf("Sending request:%s\n", "CP");

			 ssize_t RdMSG;
 			 char buf1 [256];
 			 memset(buf1, '0', sizeof(buf1));
 			 RdMSG = recv(cltsock,buf1,sizeof(buf1)-1, 0);
 			 buf1[RdMSG]='\0';
 			 if ( RdMSG<0){
 					printf("Error reading from socket\n");
 				}
 			 printf("Received from server: %s\n", buf1);
		}


		else if(strcmp(request_Name, "Count_Votes")==0){
			char buf[256];
			memset(buf, '0', sizeof(buf));
			trimwhitespace(requestLine[1]);
			char Region_name [15];
			strcpy(Region_name,requestLine[1]);
			int lenRegionName = strlen(requestLine[1]);
			for (int i = 0; i < (15 -lenRegionName); i++) {
				strcat(Region_name, " ");
			}
			sprintf(buf,"%s;%s;", "CV",Region_name);



			int n=	send (cltsock, buf, strlen(buf), 0 );

			if ( n<0){
				 printf("Error sending Add_Votes command to the  socket\n");
			 }


			 	printf("%s Is the message that was sent\n", buf);
			 	printf("message count_votes to the socket\n");
			  printf("Sending request:%s\n", "CV");

				ssize_t RdMSG;
 			 char buf1 [256];
 			 memset(buf1, '0', sizeof(buf1));
 			 RdMSG = recv(cltsock,buf1,sizeof(buf1)-1, 0);
 			 buf1[RdMSG]='\0';
 			 if ( RdMSG<0){
 					printf("Error reading from socket\n");
 				}
 			 printf("Received from server: %s\n", buf1);
		}

		else if(strcmp(request_Name, "Return_Winner")==0){
			char buf[256];
			memset(buf, '0', sizeof(buf));
			//trimwhitespace(requestLine[1]);
			sprintf(buf, "%s;\0\0;","RW");

			int n=	send (cltsock, buf, strlen(buf), 0 );
			if ( n<0){
				 printf("Error sending Add_Votes command to the  socket\n");
			 }
				printf("%s Is the message that was sent\n", buf);
				printf("message count_votes to the socket\n");
				printf("Sending request:%s\n", "RW");

			 ssize_t RdMSG;
 			 char buf1 [256];
 			 memset(buf1, '0', sizeof(buf1));
 			 RdMSG = recv(cltsock,buf1,sizeof(buf1)-1, 0);
 			 buf1[RdMSG]='\0';
 			 if ( RdMSG<0){
 					printf("Error reading from socket\n");
 				}
 			 printf("Received from server: %s\n", buf1);
		}
		else{
			printf("%s command doesn't exist\n", request_Name );
		}
		line++;
		freemakeargv(requestLine);

 }// while loop
 close(cltsock);
 printf("This client has been closed\n" );
	fclose(fd);
}




int main(int argc, char** argv) {
	char serverIP[256];
 	char requestfile [255];
	int port , sock;
	pthread_t thread;

	if (argc < NUM_ARGS + 1) {
		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}
	strcpy(requestfile, argv[1]);
	strcpy(serverIP,(argv[2]));
	port = atoi(argv[3]);

	if ((sock=socket(AF_INET , SOCK_STREAM , 0)) ==-1){
		printf("Failed to create socket\n");
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = inet_addr(serverIP);

	// Connect to the server.
	if (connect(sock, (struct sockaddr*) &address,sizeof(address)) == 0) {
		printf("Initiated connection with server at %s : %d\n", serverIP, port);

		read_Req(sock,requestfile);
		printf("I'm back in main after reading all the lines from input.req\n");

		close(sock);

	} else {

		perror("Connection failed!");
	}

	return 0;
}
