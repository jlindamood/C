/*login: LINDA140
date: 03/10/18
name: JAMES_LINDAMOOD
id: LINDA140 */



/*
 * VCforStudents.c
 * Programming Assignment 2: Directory organized votecounting
 *
 *  Created on: Feb 2, 2018
 *      Author: ayushi
 */


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

void votecounter(char* dirPath) {

	int i = 0;

	char* inputName[1024];
	chdir(dirPath);
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	char* currentDirectory = getCurrentDir(cwd);
	printf("Current Directory: %s\n", currentDirectory);
	strcpy(inputName, currentDirectory);
	strcat(inputName, ".txt");
	printf("Input name is: %s\n", inputName);

	/* Open the input file */
	FILE* inFD = fopen(inputName, "r+");
	if (inFD == NULL) {
		printf("Failed to open file.\n");	// Satisfies output specification #1
		exit(1);
	}

	char** inputStrings = (char*)malloc(100*sizeof(char));
	for (i = 0; i < 100; i++) {
		inputStrings[i] = (char*)malloc(1024*sizeof(char));
	}
	char** candidateAndVotes = (char*)malloc(2*sizeof(char));
	for (i = 0; i<2; i++) {
		candidateAndVotes = (char*)malloc(1024*sizeof(char));
	}

	/* Prepare structure to store candidates and their vote count */
		char* buffer[1024];
		i = 0;
		int inArray_tf;
		int k = 0;

		tally_t* voteTally = malloc(sizeof(struct tally) * 100);
		
		while ( fgets(buffer, 1024, inFD) != NULL ) {
			trimwhitespace(buffer);
			printf("Buffer is: %s\n", buffer);

			makeargv(buffer, ",", &inputStrings);
			while (inputStrings[k] != '\0') {
				printf("K = %d\n", k);
				printf("Current substring = %s\n", inputStrings[k]);
				makeargv(inputStrings[k], ":", &candidateAndVotes);

				printf("Candidate votes after makearg = %d\n", atoi(candidateAndVotes[1]));
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
					printf("Candidate %s, Votes = %d\n", candidateAndVotes[0], atoi(candidateAndVotes[1]));
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
	int maxIndex = 0;
	i = 0;
	while (strlen(voteTally[i].name) != 0) {
		if (voteTally[i].votes > voteTally[maxIndex].votes) {
			maxIndex = i;
		}
		i++;
	}

	printf("Appending file with Winner:%s\n", voteTally[maxIndex].name);

	fprintf(inFD, "Winner:%s", voteTally[maxIndex].name);
	
}


int main(int argc, char **argv){

	if (argc != 2){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

	pid_t pid = fork();
	/* Call child process to run Aggregate_Votes on this directory */
	if (pid == 0) {
		char* inputString[1024];
		strcpy(inputString, "./Aggregate_Votes");
		strcat(inputString, " ");
		strcat(inputString, argv[1]);
		char** inputStrings = (char*)malloc(2*sizeof(char));
		for (int i = 0; i < 2; i++) {
			inputStrings[i] = (char*)malloc(1024*sizeof(char));
		}
		makeargv(inputString, " ", &inputStrings);
		execv("./Aggregate_Votes", inputStrings);
	} else {
		wait(NULL);
	}
	
	/* Parent will fall out here */
	votecounter(argv[1]);

	return 0;
}