/*login: LINDA140
date: 03/10/18
name: JAMES_LINDAMOOD
id: LINDA140 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "util.h"


typedef struct tally{
	char* name[1024];
	int votes;
}tally_t;

tally_t* leafcounter(char* dirPath) {

	/* Form the input file name */
	char* VOTE_FILE = "/votes.txt";
	char* filename[1024];
	strcpy(filename, dirPath);
	strcat(filename, VOTE_FILE);
	printf("Input file name is %s\n", filename);

	/* Open the input file */
	FILE* inFD = fopen(filename, "r");
	if (inFD == NULL) {
		printf("Not a leaf node.\n");	// Satisfies output specification #1
		exit(1);
	}

	/* Prepare structure to store candidates and their vote count */
	char* buffer[1024];
	tally_t* voteTally = malloc(sizeof(struct tally) * 100);
	int i;
	int inArray_tf;
	while ( fgets(buffer, 1024, inFD) != NULL ) {
		trimwhitespace(buffer);
		printf("Buffer is: %s\n", buffer);
		i = 0;
		inArray_tf = 0;
		/* Check if candidate has been seen before. If so, just increment votes */
		while (strlen(voteTally[i].name) != 0) {	
			if( (strcmp(voteTally[i].name, buffer)==0)){
        		voteTally[i].votes++;
        		inArray_tf = 1;
      		}
      		i++;
		}

		/* If the candidate is new, malloc appropriately, set name, and incr votes*/
		if (inArray_tf == 1) {
			continue;
		} else {
			printf("New candidate found, adding to voteTally\n");
			struct tally* newCandidate = (struct tally*)malloc(sizeof(struct tally));
			strcpy(newCandidate->name, buffer);
			newCandidate->votes = 1;
			printf("Attempting to assign candidate #%d\n", i);
			voteTally[i] = *newCandidate;
		}
		printf("New candidate added: %s\n", voteTally[i].name);

	}

	printf("After fgets loop?\n");

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
	printf(outputName);
	close(outFD);
	close(inFD);
	printf("Votes tallied and saved in %s\n", outputName);
	return voteTally;


}

int main(int argc, char **argv){

	if (argc != 2){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

	leafcounter(argv[1]);


	return 0;
}