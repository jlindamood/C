#define _BSD_SOURCE
#define NUM_ARGS 2

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "util.h"


node_t* readDAG(char* dagtxt, node_t* nodes) {

	//
	// INTERPRET THE DAG.TXT AND FORM NODES, LINKING PARENTS TO CHILDREN
	//

	/* Open the input file */


	FILE* inFD = fopen(dagtxt, "r");
	if (inFD == NULL) {
		printf("error: input directory does not exist\n");
		exit(1);
	}

	/* Prepare structure to store candidates and their vote count */
	char* buffer[1024];
	int i;

	char** dagLine = (char**)malloc(10*sizeof(char*));
	for (i = 0; i < 10; i++) {
		dagLine[i] = (char**)malloc(1024*sizeof(char*));
	}

	int idCounter = 1;

	while ( fgets(buffer, 1024, inFD) != NULL ) {
		trimwhitespace(buffer);

		int numNodes = makeargv(buffer, ":", &dagLine);

		char* rootName = dagLine[0];


		if (findnode(&nodes[1], rootName) == NULL) {
				struct node* newNode = (struct node*)malloc(sizeof(struct node));
				strcpy(newNode->name, rootName);
				strcpy(newNode->name, trimwhitespace(newNode->name));
				newNode->closed = -1;
				newNode->num_children = 0;
				newNode->id = idCounter;
				nodes[idCounter] = *newNode;
				idCounter++;
		}

		node_t* root = findnode(&nodes[1], rootName);

		for (i = 1; i < numNodes; i++) {

				// Create the Node;
				struct node* newNode = (struct node*)malloc(sizeof(struct node));
				strcpy(newNode->name, dagLine[i]);
				strcpy(newNode->name, trimwhitespace(newNode->name));
				strcpy(newNode->parentName, rootName);
				newNode->num_children = 0;
				newNode->id = idCounter;
				newNode->closed = -1;
				nodes[idCounter] = *newNode;
				idCounter++;

				// Link the NewNode to the Root of the Subtree
				root->childNodes[root->num_children] = newNode;
				root->children[root->num_children] = newNode->id;
				root->num_children++;
				strcpy(newNode->parentName, root->name);
		}

	}
	return nodes;
}


return_t* return_winner (node_t* nodes) {

	return_t* result = malloc(sizeof(return_t));

	int i = 1;

	while (strlen(nodes[i].name) != 0) {
		if (nodes[i].closed == 0) {
			strcpy(result->code, "RO");
			strcpy(result->result, "\0");
			return result;
		}
		i++;
	}

	node_t* currentNode = findnode(&nodes[1], nodes[1].name);
	char winner[1024];

	/* Find the node */
	node_t* root = &nodes[1];
	int mostVotes = -1;
	for (i = 0; i < 100; i++) {
		if (root->tallies[i].votes > mostVotes) {
			mostVotes = root->tallies[i].votes;
			strcpy(winner, root->tallies[i].name);
		}
	}


////////////////////////DEBUG/////////////////////////////
	char voteDebug[1024];
	mostVotes = -1;
	i = 0;
	while (strlen(currentNode->tallies[i].name) != 0) {
			strcat(voteDebug, currentNode->tallies[i].name);
			sprintf(voteDebug, "%s:%d", voteDebug, currentNode->tallies[i].votes);
			if (strlen(currentNode->tallies[i+1].name) != 0) {
				strcat(voteDebug, ",");
			}
			i++;

	}
	printf("Votes in root is %s\n", voteDebug);
///////////////////////////////////////////////////////////

	char winnerfinal[1024];
	strcpy(winnerfinal, "Winner:");
	strcat(winnerfinal, winner);

	strcpy(result->result, winnerfinal);
	strcpy(result->code, "SC");
	return result;
}

return_t* open_polls (node_t* nodes, char* nodename) {
	return_t* result = malloc(sizeof(return_t));



	node_t* currentNode = findnode(&nodes[1], nodename);
	if (currentNode->closed == 0) {
		strcpy(result->code, "PF");
		strcpy(result->result, nodename);
		return result;
	}
	openPollHelper(currentNode, nodes);
	strcpy(result->code, "SC");
	strcpy(result->result, "\0");
}

return_t* close_polls (node_t* nodes, char* nodename) {
	return_t* result = malloc(sizeof(return_t));

	node_t* currentNode = findnode(&nodes[1], nodename);
	if (currentNode->closed == 1) {
		strcpy(result->code, "PF");
		strcpy(result->result, nodename);
		return result;
	}
	closePollHelper(currentNode, nodes);
	strcpy(result->code, "SC");
	strcpy(result->result, "\0");
}

return_t* count_votes (node_t* nodes, char* nodename) {
	return_t* result = malloc(sizeof(return_t));
	strcpy(result->result, "");
	int i = 0;
	printf("Here in count_votes\n");
	printf("Node name is %s,\n", nodename);
	node_t* currentNode = findnode(&nodes[1], nodename);
	int mostVotes = -1;
	while (strlen(currentNode->tallies[i].name) != 0) {
		if (currentNode->tallies[i].votes > mostVotes) {
			mostVotes = currentNode->tallies[i].votes;
			strcpy(result->result, currentNode->tallies[i].name);
		}
		strcat(result->data, currentNode->tallies[i].name);
		sprintf(result->data, "%s:%d", result->data, currentNode->tallies[i].votes);
		if (strlen(currentNode->tallies[i+1].name) != 0) {
			strcat(result->data, ",");
		}
		i++;
	}

	strcpy(result->code, "SC");
	if (mostVotes == -1) {
		strcpy(result->code, "NV");
	}
	return result;
}

return_t* add_votes (node_t* nodes, char* nodename, char* dataString) {

	return_t* result = malloc(sizeof(return_t));

	node_t* currentNode = findnode(&nodes[1], nodename);

	if (currentNode->num_children != 0) {
		strcpy(result->code, "NL");
		strcpy(result->result, nodename);
		return result;
	}

	if (currentNode->closed == -1 || currentNode->closed == 1) {
		strcpy(result->code, "RC");
		strcpy(result->result, nodename);
		return result;
	}

	int i = 0;
	char** inputStrings = (char**)malloc(100*sizeof(char*));
	for (i = 0; i < 100; i++) {
		inputStrings[i] = (char**)malloc(1024*sizeof(char*));
	}

	char** candidateAndVotes = (char**)malloc(2*sizeof(char*));
	for (i = 0; i < 2; i++) {
		candidateAndVotes[i] = (char**)malloc(1024*sizeof(char*));
	}

	int k = 0;
	int inArray_tf = 0;

	makeargv(dataString, ",", &inputStrings);


	i = 0;
	while (inputStrings[i] != '\0') {
		makeargv(inputStrings[i], ":", &candidateAndVotes);
		printf("Working on candidate, votes: %s,%d\n", candidateAndVotes[0], atoi(candidateAndVotes[1]));
		k = 0;
		inArray_tf = 0;
		while (strlen(currentNode->tallies[k].name) != 0) {
			if( (strcmp(candidateAndVotes[0], currentNode->tallies[k].name)==0)){
        		currentNode->tallies[k].votes = currentNode->tallies[k].votes + atoi(candidateAndVotes[1]);
						printf("Tallies[%d].name = %s, votes = %d", k, currentNode->tallies[k].name, currentNode->tallies[k].votes);
        		inArray_tf = 1;
      		}
      		k++;
		}
		if (inArray_tf == 1) {
			i++;
			continue;
			} else {
				strcpy (currentNode->tallies[k].name, candidateAndVotes[0]);
				currentNode->tallies[k].votes = atoi(candidateAndVotes[1]);
				printf("Tallies[%d].name = %s, votes = %d", k, currentNode->tallies[k].name, currentNode->tallies[k].votes);
		}
		i++;
	}
	printf("Current node = %s\n", currentNode->name);
	printf("Node parent = %s\n", currentNode->parentName);

	// Recursively bubble the additional votes up to parent nodes
	if (strlen(currentNode->parentName) != 0) {
		add_votes2(nodes, currentNode->parentName, dataString);
	}

	strcpy(result->code, "SC");
	strcpy(result->result, "\0");
	return result;
}

return_t* remove_votes (node_t* nodes, char* nodename, char* dataString) {

	return_t* result = malloc(sizeof(return_t));

	node_t* currentNode = findnode(&nodes[1], nodename);

	if (currentNode->num_children != 0) {
		strcpy(result->code, "NL");
		strcpy(result->result, nodename);
		return result;
	}

	if (currentNode->closed == -1 || currentNode->closed == 1) {
		strcpy(result->code, "RC");
		strcpy(result->result, nodename);
		return result;
	}

	int i = 0;
	char** inputStrings = (char**)malloc(100*sizeof(char*));
	for ( i = 0; i < 100; i++) {
		inputStrings[i] = (char**)malloc(1024*sizeof(char*));
	}


	char** candidateAndVotes = (char**)malloc(2*sizeof(char*));
	for (i = 0; i < 2; i++) {
		candidateAndVotes[i] = (char**)malloc(1024*sizeof(char*));
	}

	int k = 0;
	int inArray_tf = 0;

	makeargv(dataString, ",", &inputStrings);

	char** candidateAndVotesIS = (char**)malloc(2*sizeof(char*));
	for (i = 0; i < 2; i++) {
		candidateAndVotesIS[i] = (char**)malloc(1024*sizeof(char*));
	}

	/* INVALID SUBTRACTION CHECK */
	i = 0;
	int invalidSubtraction_tf = 0;
	while (inputStrings[i] != '\0') {
		makeargv(inputStrings[i], ":", &candidateAndVotesIS);
		k = 0;
		while (strlen(currentNode->tallies[k].name) != 0) {
			if( (strcmp(candidateAndVotesIS[0], currentNode->tallies[k].name)==0)){
        		if (currentNode->tallies[k].votes - atoi(candidateAndVotesIS[1]) < 0) {
							strcpy(result->code, "IS");
							if (strlen(result->data) != 0) {
								strcat(result->data, ",");
							}
							strcat(result->data, candidateAndVotesIS[0]);
							invalidSubtraction_tf = 1;
						}
        		inArray_tf = 1;
      		}
      		k++;
		}
		if (inArray_tf == 1) {
			i++;
			continue;
		} else {
			strcpy(result->code, "IS");
			if (strlen(result->data) != 0) {
				strcat(result->data, ",");
			}
			strcat(result->data, candidateAndVotesIS[0]);
			invalidSubtraction_tf = 1;
		}
		i++;
	}

	if (invalidSubtraction_tf) {
		return result;
	}


	i = 0;
	while (inputStrings[i] != '\0') {
		makeargv(inputStrings[i], ":", &candidateAndVotes);
		printf("Working on candidate, subtracting votes: %s,%d\n", candidateAndVotes[0], atoi(candidateAndVotes[1]));
		k = 0;
		inArray_tf = 0;
		while (strlen(currentNode->tallies[k].name) != 0) {
			if( (strcmp(candidateAndVotes[0], currentNode->tallies[k].name)==0)){
        		currentNode->tallies[k].votes = currentNode->tallies[k].votes - atoi(candidateAndVotes[1]);
        		inArray_tf = 1;
      		}
      		k++;
		}
		if (inArray_tf == 1) {
			i++;
			continue;
			} else {
				// perror("IS: Illegal subtraction, name of candidate doesn't exist");
		}
		i++;
	}

	printf("Current node = %s\n", currentNode->name);
	printf("Node parent = %s\n", currentNode->parentName);

	if (strlen(currentNode->parentName) != 0) {
		remove_votes2(nodes, currentNode->parentName, dataString);
	}

	strcpy(result->code, "SC");
	strcpy(result->result, "\0");
	return result;
}


void* thread_Handler(void *args){
struct condQueue* cq = (struct condQueue*) args;
int cltsock;
cltsock= cq->cltsock;
char* dagtxt= cq->dagtxt;

char buffer[256];
ssize_t recvMSG;
memset(buffer, '0', sizeof(buffer));
//free(cq);

char** request = (char**) malloc(100*sizeof(char*));
for (int i = 0; i < 100; i++) {
	request[i] = (char**)malloc(1024*sizeof(char*));
}

readDAG(dagtxt, cq->nodes);
printgraph(&(cq->nodes[1]), 8);

pthread_mutex_lock(cq->mutex);
while(1){
recvMSG = recv(cltsock, buffer, sizeof(buffer)-1, 0);
buffer[recvMSG]='\0';


makeargv(buffer,";", &request);
if (recvMSG<0){ printf("Error reading from socket\n");}
if (strlen(buffer) == 0) {break;}
printf("Request received from client at %s:%d,%s,%s\n", cq->clientAddress, cq->port, trimwhitespace(request[0]),request[1]); //TODO the ip address
return_t* result = malloc(sizeof(return_t));
char message[1024];

//TODO analyse the Request
if (strcmp(request[0], "RW") == 0) {
	result = return_winner(cq->nodes);
	strcpy(message, result->code);
	strcat(message, ";");
	if (strcmp(result->code, "RO") == 0 || strcmp(result->code, "UE") == 0) {
	} else {
		strcat(message, result->result);
	}

}
else if (strcmp(request[0], "CV") == 0) {
	trimwhitespace(request[1]);
	char* nodename = request[1];
	result = count_votes (cq->nodes, nodename);
	strcpy(message, result->code);
	strcat(message, ";");
	if (strcmp(result->code, "NV") == 0) {
		strcat(message, "No votes.");
	} else {
		strcat(message, result->data);
	}
}
else if (strcmp(request[0], "OP") == 0) {
	trimwhitespace(request[1]);
	char* nodename = request[1];
	result = open_polls(cq->nodes,nodename);
	strcpy(message, result->code);
	strcat(message, ";");
	if (strcmp(result->code, "PF") == 0) {
		strcat(message, nodename);
		strcat(message, " open.");
	}
}
else if (strcmp(request[0], "AV") == 0) {
	trimwhitespace(request[1]);
	char* nodename = request[1];
	char data[1024];
	strcpy(data, request[2]);
	result = add_votes(cq->nodes, nodename, data);

	strcpy(message, result->code);
	strcat(message, ";");
	if (strcmp(result->code, "NL") == 0 || strcmp(result->code, "RC") == 0) {
		strcat(message, nodename);
	}
}
else if (strcmp(request[0], "RV") == 0) {
	trimwhitespace(request[1]);
	char* nodename = request[1];
	char data[1024];
	strcpy(data, request[2]);
	result = remove_votes(cq->nodes, nodename, data);

	strcpy(message, result->code);
	strcat(message, ";");
	if (strcmp(result->code, "NL") == 0 || strcmp(result->code, "RC") == 0) {
		strcat(message, nodename);
	}
	if (strcmp(result->code, "IS") == 0) {
		strcat(message, result->data);
	}
}
else if (strcmp(request[0], "CP") == 0) {
	trimwhitespace(request[1]);
	char* nodename = request[1];
	result = close_polls(cq->nodes, nodename);
	strcpy(message, result->code);
	strcat(message, ";");
	if (strcmp(result->code, "PF") == 0) {
		strcat(message, nodename);
		strcat(message, " closed.");
	}
}

// Fall through, invalid request
else {
	perror("Invalid request\n");
}

message[strlen(message)] = '\0';
int n=	send (cltsock, message, strlen(message), 0 );
printf("sending to the client the respone%s\n",message);
printf("Sending response to client at %s:%d,%s,%s\n",cq->clientAddress, cq->port,request[0], message);

	//close(newsock);
}//loop while(1)
pthread_detach(pthread_self);
//return (NULL);
pthread_mutex_unlock(cq->mutex);
printf("Closed connection with client at %s:%d\n",cq->clientAddress, cq->port);
return;
}


int main(int argc, char** argv) {

	pthread_t thread[5];
	int sock, port, newsock;
	struct sockaddr_in clientAddress;

	if (argc !=(NUM_ARGS + 1)) {
		printf("Wrong number of args, expected %d, given %d\n", NUM_ARGS, argc - 1);
		exit(1);
	}

	port = atoi(argv[2]);
	struct sockaddr_in servAddress;
	servAddress.sin_family = AF_INET;  //address family is set ad internet
	servAddress.sin_port = htons(port); //using the port that was passed
	servAddress.sin_addr.s_addr = htonl(INADDR_ANY);  //setting the IP address


	if ((sock=socket(AF_INET , SOCK_STREAM , 0)) ==-1){
		printf("Failed to create socket\n");
	}
	if ((bind(sock, (struct sockaddr *) &servAddress, sizeof(servAddress)))==-1){
		printf("Failed to bind the socket to port%d\n", port);
	}

	if (listen(sock, MAX_CONNECTIONS )!=0){
			printf ("Exceed the numbers of clientts");
			exit(0);
		}

	printf("Server listening on port %d\n", port );

	// A server typically runs infinitely, with some boolean flag to terminate.
	while (1) {

		// Now accept the incoming connections.
		struct sockaddr_in clientAddress;
		socklen_t size = sizeof(struct sockaddr_in);

		if ((newsock = accept(sock, (struct sockaddr*) &clientAddress, (socklen_t*) &size))<0){
			printf("Error on accept\n");
		}

		printf("Connection initiated from client at %s:%d\n",inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

		struct condQueue* cq  = (struct condQueue*) malloc(sizeof(struct condQueue));
	 	cq->nodes = (struct node*) malloc(sizeof(struct node) * 100);
		cq->cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
		cq->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));

		/* Initializing the condition mutexes*/
		pthread_cond_init(cq->cond, NULL);
		pthread_mutex_init(cq->mutex, NULL);
		cq->cltsock = newsock;
		cq->port = ntohs(clientAddress.sin_port);
		strcpy(cq->clientAddress, inet_ntoa(clientAddress.sin_addr));
		strcpy(cq->dagtxt, argv[1]);

		//cq->clientAddress = clientAddress;

		if(pthread_create(&thread, NULL, thread_Handler, (void*) cq)< 0){
			printf("Failed to create the pthread for the client");
			return(1);
		}
		pthread_join(thread, NULL);
		pthread_mutex_destroy(cq->mutex);
		pthread_cond_destroy(cq->cond);
		pthread_exit(NULL);

}//loop while(1)2

	// Close the server socket.
	//close(newsock);


	close(sock);

}
