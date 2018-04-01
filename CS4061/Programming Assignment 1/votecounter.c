/* Written by: James Lindamood (LINDA140) &
				Omar Abdelfatah (ABDEL127)
*/



/*
 * VCforStudents.c
 *
 *  Created on: Feb 2, 2018
 *      Author: ayushi
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "makeargv.h"

#define MAX_NODES 100


//Function signatures

/**Function : parseInput
 * Arguments: 'filename' - name of the input file
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Total Allocated Nodes
 * About parseInput: parseInput is supposed to
 * 1) Open the Input File [There is a utility function provided in utility handbook]
 * 2) Read it line by line : Ignore the empty lines [There is a utility function provided in utility handbook]
 * 3) Call parseInputLine(..) on each one of these lines
 ..After all lines are parsed(and the DAG created)
 4) Assign node->"prog" ie, the commands that each of the nodes has to execute
 For Leaf Nodes: ./leafcounter <arguments> is the command to be executed.
 Please refer to the utility handbook for more details.
 For Non-Leaf Nodes, that are not the root node(ie, the node which declares the winner):
 ./aggregate_votes <arguments> is the application to be executed. [Refer utility handbook]
 For the Node which declares the winner:
 This gets run only once, after all other nodes are done executing
 It uses: ./find_winner <arguments> [Refer utility handbook]
 */
int parseInput(char *filename, node_t *n);

/**Function : parseInputLine
 * Arguments: 's' - Line to be parsed
 * 			  'n' - Pointer to Nodes to be allocated by parsing
 * Output: Number of Region Nodes allocated
 * About parseInputLine: parseInputLine is supposed to
 * 1) Split the Input file [Hint: Use makeargv(..)]
 * 2) Recognize the line containing information of
 * candidates(You can assume this will always be the first line containing data).
 * You may want to store the candidate's information
 * 3) Recognize the line containing "All Nodes"
 * (You can assume this will always be the second line containing data)
 * 4) All the other lines containing data, will show how to connect the nodes together
 * You can choose to do this by having a pointer to other nodes, or in a list etc-
 * */
int parseInputLine(char *s, node_t *n, int lineNumber);

/**Function : execNodes
 * Arguments: 'n' - Pointer to Nodes to be allocated by parsing
 * About execNodes: parseInputLine is supposed to
 * If the node passed has children, fork and execute them first
 * Please note that processes which are independent of each other
 * can and should be running in a parallel fashion
 * */
void execNodes(node_t *n, node_t *mainN);

int parseInput(char *filename, node_t *n) {

	/* 1) Open the input file */
	FILE* file = file_open(filename);
	
	/* Parse it into lines, an array of strings*/
	char** lines;
	char* buffer[1024];
	char* firstline[1024];
	int allocatedNodes = 0;

	/* Read in each line and parse it with parseInputLine */
	int i = 1;

	/* Prepare structure to store number of candidates and their names */
	char** candidateStructure = (char*)malloc(10*sizeof(char));
	for (i = 0; i < 10; i++) {
		candidateStructure[i] = (char*)malloc(1024*sizeof(char));
	}

	i = 1;
	while (read_line(buffer,file) != NULL) {
		// printf("The buffer is: %s\n", buffer);
		printf("Attempting to read file line by line, i is %d\n", i);
		printf("Buffer is %s\n", buffer);
		allocatedNodes = allocatedNodes + parseInputLine(buffer, n, i);
		if (i == 1) {
			/* For the first line, parse it to determine candidates */
			/* candidateStructure[1:NUMBER_OF_CANDIDATES - 1] are the candidate names */
			strcpy(firstline, trimwhitespace(buffer));
			makeargv(buffer, " ", &candidateStructure);
			int NUMBER_OF_CANDIDATES = atoi(candidateStructure[0]);
		}
		i++;
	}

	/* Assign node->"prog" and other properties*/
	i = 0;

	/* Assign to other nodes */
	for (i = 0; i < allocatedNodes; i++) {
		/* Assign candidate structure, or the first line of the file */
		strcpy(n[i].candidates, firstline);
		/* Assign output file name */
		char* outputprepend = "Output_";
		strcpy(n[i].output, n[i].name);
		strcpy(n[i].output, prepend(n[i].output, outputprepend));
		if (n[i].num_children == 0) {
			/* Determine input file names*/
			/* Leaf node inputs are simply the files with their name */
			printf("i = %d, num_children = %d\n", i, n[i].num_children);
			strcpy(n[i].input[0], n[i].name);
			strcpy(n[i].prog, "./leafcounter");
		} else {
			/* Determine input and output parameters */
			/* Aggregate votes have an input file for each child, so loop and create */
			int child_so_far = 0;
			printf("i = %d, num_children = %d\n", i, n[i].num_children);
			while (n[i].num_children - child_so_far > 0) {
				/* Find child node and crete its output (might be duplicate work, but oh well */
				node_t* child = findNodeByID(n, n[i].children[child_so_far]);
				strcpy(child->output, child->name);
				strcpy(child->output, prepend(child->output, outputprepend));
				/* Copy child's output name into appropriate slot in input sources array */
				strcpy(n[i].input[child_so_far], child->output);	
				child_so_far++;
				/* i.e. after first iteration, node[i]->input[0] will be the first child's output file name */
			}
			if (i == 0) {
				/* Assign first to root */
				strcpy(n[i].prog, "./find_winner");
			} else {
				strcpy(n[i].prog, "./aggregate_votes");
			}
		}
	}

	return allocatedNodes;

}


int parseInputLine(char *s, node_t *n, int lineNumber) {

	int i = 0;
	int numNodes = 0;

	/* Parse line number 1. We do not allocate nodes here */
	if (lineNumber == 1) {
		/* This is taken care of by parseInput */
		return numNodes;
	}
	/* Parse line number 2. We break the line into an array of strings, 
	then initialize nodes based on the size of that array */
	else if (lineNumber == 2) {
		char** outputStrings = (char*)malloc(10*sizeof(char));
		for (i = 0; i < 10; i++) {
			outputStrings[i] = (char*)malloc(1024*sizeof(char));
		}
		numNodes = makeargv(s, " ", &outputStrings);

		for (i = 0; i < numNodes; i++) {
			struct node* newNode = (struct node*)malloc(sizeof(struct node));
			strcpy(newNode->name, outputStrings[i]);
			strcpy(newNode->name, trimwhitespace(newNode->name));
			newNode->num_children = 0;
			newNode->id = i + 1;
			n[i] = *newNode;
			printf("Initializing node %d, name = %s\n", n[i].id, n[i].name);
		}
		return numNodes;
	}

	/* Parse all other lines. These will create the dependency tree */
	else {
		/* Create an array of size 2. Element 0 will be the node. Element 1 will be its dependencies */
		char** outputStrings = (char*)malloc(10*sizeof(char));
		for (i = 0; i < 10; i++) {
			outputStrings[i] = (char*)malloc(1024*sizeof(char));
		}
		makeargv(s, " : ", &outputStrings);

		/* Further split up the second element into an array of node names */
		/* char** dependencies = (char*)malloc(10*sizeof(char));
		for (int i = 0; i < 10; i++) {
			dependencies[i] = (char*)malloc(1024*sizeof(char));
		}
		printf("Outputstrings[1] is %s\n", outputStrings[1]);
		makeargv(outputStrings[1], " ", &dependencies);

		/* Use the dependencies array and findnode (by name) to assign children */
		node_t* parent = findnode(n, outputStrings[0]);
		i = 1;
		while (outputStrings[i] != NULL) {
			strcpy(outputStrings[i], trimwhitespace(outputStrings[i]));
			node_t* child = findnode(n, outputStrings[i]);
			printf("outputStrings[%d] = %s,\n", i, outputStrings[i]);
			parent->children[i - 1] = child->id;
			parent->num_children++;
			parent->childNodes[i - 1] = child;
			printf("Parent %s has been assigned child %s, %d, %d\n", parent->name, child->name, parent->id, child->id);
			i++;
		}
		return numNodes;
	}

	return 0;

}

void execNodes(node_t *n, node_t *mainN) {

	int i;

	printf("Process of node name = %s\n", n->name);

	/* Recursive base case, is leaf node */
	if (n->num_children == 0) {
		/* Create input argument string */
		char* inputString = (char *)malloc(52*1024*sizeof(char));
		strcpy(inputString, n->prog);
		strcat(inputString, " ");
		strcat(inputString, n->input[0]);
		strcat(inputString, " ");
		/* Create "Input_File1 Input_File2 Input_File3 . . . Input_FileN "*/
		for (i = 1; i < n->num_children; i++) {
			strcat(inputString, n->input[i]);
			strcat(inputString, " ");
		}
		/* Create "InputFile_1 . . . InputFileN Output_File" */
		strcat(inputString, n->output);
		strcat(inputString, " ");
		/* Create "InputFile_1 . . . InputFileN Output_File # A B C . . . N" */
		strcat(inputString, n->candidates);

		printf("Input string is %s\n", inputString);
		char** inputStrings = (char*)malloc(20*sizeof(char));
		for (i = 0; i < 20; i++) {
			inputStrings[i] = (char*)malloc(1024*sizeof(char));
		}
		makeargv(inputString, " ", &inputStrings);
		/* Exec leafcounter */
		execv(inputStrings[0], inputStrings);
	} else {
		int childNumber = 0;
		while (n->num_children - childNumber > 0) {
			n->pid = fork();
			if (n->pid == 0) {
				/* Child Process */
				execNodes(&mainN[n->children[childNumber] - 1], mainN);
			} else {
				/* Increment childNumber and the parent continues the loop */
				wait(NULL);
				childNumber++;
			}
		}

		/* Create input argument string */
		char* inputString = (char *)malloc(52*1024*sizeof(char));
		strcpy(inputString, n->prog);
		strcat(inputString, " ");
		char snum[5];
		sprintf(snum, "%d", n->num_children);
		strcat(inputString, snum);
		strcat(inputString, " ");
		strcat(inputString, n->input[0]);
		strcat(inputString, " ");
		/* Create "Input_File1 Input_File2 Input_File3 . . . Input_FileN "*/
		for (i = 1; i < n->num_children; i++) {
			strcat(inputString, n->input[i]);
			strcat(inputString, " ");
		}
		/* Create "InputFile_1 . . . InputFileN Output_File" */
		strcat(inputString, n->output);
		strcat(inputString, " ");
		/* Create "InputFile_1 . . . InputFileN Output_File # A B C . . . N" */
		strcat(inputString, n->candidates);

		printf("Input string  of parent is %s\n", inputString);
		char** inputStrings = (char*)malloc(20*sizeof(char));
		for (i = 0; i < 20; i++) {
			inputStrings[i] = (char*)malloc(1024*sizeof(char));
		}
		makeargv(inputString, " ", &inputStrings);

		printf("Executing with command:\n");
		printf("%s %s %s . . .\n", inputStrings[0], inputStrings[0], inputStrings[1]);
		/* Finally, execute the parent node */
		execv(inputStrings[0], inputStrings);


	}

	return 0;
}


int main(int argc, char **argv){

	//Allocate space for MAX_NODES to node pointer
	struct node* mainnodes=(struct node*)malloc(sizeof(struct node)*MAX_NODES);

	if (argc != 2){
		printf("Usage: %s Program\n", argv[0]);
		return -1;
	}

	//call parseInput
	printf("File name is: %s\n", argv[1]);
	int num = parseInput(argv[1], mainnodes);


	for (int i = 0; i < num; i++) {
		printf("name = %s\n", mainnodes[i].name);
		printf("prog = %s\n", mainnodes[i].prog);
		printf("input = %s\n", mainnodes[i].input[0]);
		printf("output = %s\n", mainnodes[i].output);
		printf("candidates = %s\n", mainnodes[i].candidates);
		printf("num_children = %d\n\n", mainnodes[i].num_children);
		if (mainnodes[i].childNodes[0]->name != NULL) {
			printf("child node 1 = %s, %d\n\n", mainnodes[i].childNodes[0]->name, mainnodes[i].childNodes[0]->id);
		}
	}


	//Call execNodes on the root node
	execNodes(mainnodes, mainnodes);

	printf("The winner is located in the file Output_Who_Won\n");
	free(mainnodes);
	return 0;
}