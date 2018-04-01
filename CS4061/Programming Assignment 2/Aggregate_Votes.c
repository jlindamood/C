/*login: LINDA140
date: 03/10/18
name: JAMES_LINDAMOOD
id: LINDA140 */


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include "util.h"

typedef struct tally{
	char* name[1024];
	int votes;
}tally_t;

void aggregateVotes(char* dirPath, node_t* mainN) {

	/* Open directory */
	DIR* dir = opendir(dirPath);
	struct dirent* entry;

	if (dir == NULL) {
		printf("Error: cannot open directory %s", dirPath);
		exit(1);
	}

	int i = 0;
	/* Navigate down the entries in the directory */
	/* Form the node->child structure */
		while ((entry = readdir(dir)) != NULL) {

			if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;

			/* If the entry is another directory, it is thus a child
			of the current directory */
			if (entry->d_type == DT_DIR) {
				printf("Subdirectory %s found, attempting to add as child.\n", entry->d_name);
				i = 0;
				while (mainN->childNodes[i] != NULL) {
					i++;
				}
				struct node* newNode = (struct node*)malloc(sizeof(struct node));
				strcpy(newNode->name, entry->d_name);
				newNode->id = i;
				newNode->num_children = 0;
				mainN->childNodes[i] = newNode;
				mainN->num_children++;
			} else {

			}
		}

		/* Check if current directory/node is a leaf */
		/* If so, execute Leaf_counter */

		if (mainN->num_children == 0) {
			printf("This directory is a leaf. Executing Leaf_counter\n");
			char* inputString[1024];
			strcpy(inputString, "./Leaf_Counter");
			strcat(inputString, " ");
			strcat(inputString, dirPath);
			char** inputStrings = (char*)malloc(2*sizeof(char));
			for (i = 0; i < 2; i++) {
				inputStrings[i] = (char*)malloc(1024*sizeof(char));
			}
			makeargv(inputString, " ", &inputStrings);
			execv("./Leaf_Counter", inputStrings);
		/* If not, fork the number of children and execute aggregateVotes again */
		} else {
			i = 0;
			while (i < mainN->num_children) {
				printf("Forking child %d\n", i + 1);
				pid_t pid = fork();
				if (pid == 0) {
					char* inputString[1024];
					strcpy(inputString, "./Aggregate_Votes");
					strcat(inputString, " ");
					strcat(inputString, dirPath);
					strcat(inputString, mainN->childNodes[i]->name);
					char** inputStrings = (char*)malloc(2*sizeof(char));
					for (i = 0; i < 2; i++) {
						inputStrings[i] = (char*)malloc(1024*sizeof(char));
					}
					makeargv(inputString, " ", &inputStrings);
					execv("./Aggregate_Votes", inputStrings);
				} else {
					wait(NULL);
					i++;
				}
			}

		}

	/* After we have hit the leaf nodes, we will fall here */
	/* We need to access each child directory and tally */

	printf("After forking?\n");
	/* Form the output file name */
	char* outputName[1024];
	chdir(dirPath);
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	char* currentDirectory = getCurrentDir(cwd);
	printf("Current Directory: %s\n", currentDirectory);
	strcpy(outputName, currentDirectory);
	strcat(outputName, ".txt");
	printf("Output name is: %s\n", outputName);

	/* Specification #2.1 & 3. Open and write to output file */
	FILE* outFD = fopen(outputName, "w");

	printf("Open outFD?\n");

	tally_t* voteTally = malloc(sizeof(struct tally) * 100);
	FILE* inFD;

	int j = 0;
	while (j < mainN->num_children) {

		char** inputStrings = (char*)malloc(100*sizeof(char));
		for (i = 0; i < 100; i++) {
			inputStrings[i] = (char*)malloc(1024*sizeof(char));
		}


		char** candidateAndVotes = (char*)malloc(2*sizeof(char));
		for (i = 0; i<2; i++) {
			candidateAndVotes = (char*)malloc(1024*sizeof(char));
		}
		/* Form the input file name */
		char* filename[1024];
		strcpy(filename, "");
		strcat(filename, mainN->childNodes[j]->name);
		strcat(filename, "/");
		strcat(filename, mainN->childNodes[j]->name);
		strcat(filename, ".txt");

		printf("Trying to open subdirectory file %s\n", filename);

	
		/* Open the input file */
		inFD = fopen(filename, "r");
		if (inFD == NULL) {
			printf("Child vote count file does not exist.\n");	// Satisfies output specification #1
			printf("Attempting file name: %s\n", filename);
			exit(1);
		}

		/* Prepare structure to store candidates and their vote count */
		char* buffer[1024];
		int i = 0;
		int inArray_tf;
		int k = 0;
		
		while ( fgets(buffer, 1024, inFD) != NULL ) {
			trimwhitespace(buffer);
			printf("Buffer is: %s\n", buffer);

			makeargv(buffer, ",", &inputStrings);
			while (inputStrings[k] != '\0') {
				printf("K = %d\n", k);
				printf("Current substring = %s\n", inputStrings[k]);
				makeargv(inputStrings[k], ":", &candidateAndVotes);

				printf("Candidate votes after makearg = %s\n", candidateAndVotes[1]);
				// makeargv(inputStrings[k], ":", &candidateAndVotes); THIS COMPLETELY BREAKS IT?
				// candidateAndVotes[0] = candidatename
				// candidateAndVotes[1] = #votes
				// printf("Get here?\n");
				i = 0;
				inArray_tf = 0;
				/* Check if candidate has been seen before. If so, just increment votes */
				while (strlen(voteTally[i].name) != 0) {	
					if( (strcmp(voteTally[i].name, candidateAndVotes[0])==0)){
        				voteTally[i].votes = voteTally[i].votes + atoi(candidateAndVotes[1]);
        				inArray_tf = 1;
      				}
      				i++;
				}

				/* If the candidate is new, malloc appropriately, set name, and incr votes*/
				if (inArray_tf == 1) {
					k++;
					continue;
				} else {
					printf("New candidate found, adding to voteTally\n");
					struct tally* newCandidate = (struct tally*)malloc(sizeof(struct tally));
					strcpy(newCandidate->name, candidateAndVotes[0]);
					printf("Candidate %s, Votes = %d\n", candidateAndVotes[0], candidateAndVotes[1]);
					newCandidate->votes = atoi(candidateAndVotes[1]);
					printf("Attempting to assign candidate #%d\n", i);
					voteTally[i] = *newCandidate;
				}
			
				printf("New candidate added: %s\n", voteTally[i].name);
				k++;
				freemakeargv(candidateAndVotes);
			}

		freemakeargv(inputStrings);

		}
	// fclose(inFD);
	j++;
	
	}

	printf("After fgets loop?\n");

	i = 0;
	while (strlen(voteTally[i].name) != 0) {
		printf("Attempting to write out %s:%d,\n", voteTally[i].name, voteTally[i].votes);
		fprintf(outFD, "%s:%d", voteTally[i].name, voteTally[i].votes);
		/* Convert tally.votes int to string */
		if (strlen(voteTally[i+1].name) != 0) {
			fprintf(outFD, ",");
		}
		i++;
	}

	fprintf(outFD, "\n");

	/* Specification #2.2 */
	printf("%s\n", outputName);
	close(outFD);
	close(inFD);
	printf("Votes tallied and saved in %s\n", outputName);

}

int main(int argc, char **argv){

	if (argc != 2){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

	struct node* node = (struct node*)malloc(sizeof(struct node));
	strcpy(node->name, argv[1]);
	node->id = 1;

	aggregateVotes(argv[1], node);

	return 0;
}