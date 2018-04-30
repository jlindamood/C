/*login: LINDA140
date: 03/10/18
name: JAMES_LINDAMOOD
id: LINDA140 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define REGION_NAME_SIZE 15
/* The maximum amount of bytes for a file name */
#define MAX_FILE_NAME_SIZE 255

/* The maximum amount of bytes for each I/O operation */
#define MAX_IO_BUFFER_SIZE 1024

#define BUFFER_SIZE 1024

/* maximun numbers of clients open*/
#define MAX_CONNECTIONS 5

/* Node structure, largely adapted from PA1 */
typedef struct tally{
  char* name[100];
  int votes;
  int num;
}tally_t;

typedef struct returnType {
  char result[1024];
  char code[1024];
  char data[1024];
}return_t;

typedef struct node{
  char name[1024];
  char prog[1024];
  char input[50][1024];
  char output[1024];
  struct node* childNodes[10];
  int children[10];
  int num_children;
  int children_processed;
  int status;
  pid_t pid;
  int id;
  struct tally tallies[100];
  int closed;
  char parentName[15];
}node_t;

typedef struct threadArgs {
  char* requestfile[MAX_FILE_NAME_SIZE];
  int cltsock;
  struct sockaddr_in* clientAddress;
  int index;
  int size;
}threadArgs_t;

typedef struct condQueue {
	//struct queue* q;
  char* requestfile[MAX_FILE_NAME_SIZE];
  int cltsock;
  int port;
  char  clientAddress[255];
	pthread_cond_t* cond;
	pthread_mutex_t* mutex;
  node_t* nodes;
  char dagtxt[1024];
  char inputPath[1024];
  char logCWD[1024];
}cqueue_t;

/*typedef struct arg_struct {
  cqueue_t* cq;
  node_t* nodes;
  char* inputPath[1024];
} args_t;*/

/**********************************
*
* Taken from Unix Systems Programming, Robbins & Robbins, p37
*
*********************************/
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }
   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

/**********************************
*
* Taken from Unix Systems Programming, Robbins & Robbins, p38
*
*********************************/
void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}

char *trimwhitespace(char *str) {
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;

  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

node_t* findnode(node_t* start, char* tobefound){
  //Find the node in question
    node_t* temp = start;
    while(temp->id!=NULL){
      if( (strcmp(temp->name, tobefound)==0)){
        return temp;
      }

      temp++;
    }
    return NULL;
}

node_t* findNodeByID(node_t* start, int tobefound){
  //Find the node in question
    node_t* temp = start;
    while(temp->id!=NULL){
      if( temp->id == tobefound){
        return temp;
      }

      temp++;
    }

    return NULL;
}


char* prepend(char* s, const char* t)
{
    size_t len = strlen(t);
    size_t i;

    memmove(s + len, s, strlen(s) + 1);

    for (i = 0; i < len; ++i)
    {
        s[i] = t[i];
    }
    return s;
}

void printgraph(node_t* mainnodes, int num){
      int p;
        for (p = 0; p < num; p++){
          if(mainnodes[p].num_children==0){
            printf("%s","\n Leaf Node : ");
            printf("%s",mainnodes[p].name);
          }
          else{
            printf("\n Non-Leaf Nodes %s",mainnodes[p].name);
            printf("\n Listing their children: ");
            int x;
            for(x=0;x<mainnodes[p].num_children;x++){
              printf("\n %d ",mainnodes[p].children[x]);
              printf(" %s ",(findNodeByID(mainnodes, mainnodes[p].children[x])->name));
            }

          }
        }
      printf("\n");
}

/* Stackoverflow user: Michi */
void *getCurrentDir(char *path){
    char *token;
    char *directory;
    size_t length;

    token = strrchr(path, '/');

    if (token == NULL) {
        printf("Error"); /* You decide here */
        exit(1);
    }

    length = strlen(token);
    directory = malloc(length);
    memcpy(directory, token+1, length);

    return directory;
}

/*
void* createDirHelper (node_t* currentNode, node_t* mainnodes) {
  chdir(currentNode->name);

  char cwd[1024];
  getcwd(cwd, sizeof(cwd));
  char* currentDirectory = getCurrentDir(cwd);
  //printf("\nCurrent Directory: %s\n", cwd);

  strcpy(currentNode->dir, cwd);

  //printf("Entered dirHelper for %s\n", currentNode->name);
  for (int i = 0; i<currentNode->num_children; i++) {
      char* dirToMake[1024];
      strcpy(dirToMake, currentNode->childNodes[i]->name);

      struct stat nodest = {0};
      if (stat(dirToMake, &nodest) == -1) {
        //printf("Making directory: %s\n", dirToMake);
        mkdir(dirToMake, 0700);
      } else {
        // ADD REMOVE CLAUSE HERE
      }
      createDirHelper(findnode(&mainnodes[1], dirToMake), &mainnodes[1]);
      chdir(cwd);

  }
}
*/

int is_regular_file(const char *path) {
  struct stat path_stat;
  stat (path, &path_stat);
  return S_ISREG(path_stat.st_mode);
}


char* changedir(char* path){
	char** Line= (char**) malloc(10*sizeof (char*));
	for(int i=0; i<10; i++){
		Line[i] = (char**)malloc(1024*sizeof(char*));
	}
	char* actual_dir = (char*) malloc(sizeof (char));
	char* dir = (char*) malloc(sizeof (char));
	int tokens = makeargv(path, "/", &Line);

	for(int i=0;i<(tokens-1);i++){
		if(i==0){
			sprintf(dir,"%s", Line[0]);
			strcpy(actual_dir, dir);
		}
		else{
			sprintf(dir,"/%s",Line[i]);
			strcat(actual_dir, dir);
		}
	}
return actual_dir;
}

void* openPollHelper (node_t* currentNode, node_t* mainnodes) {

  if (currentNode->closed == -1) {
    currentNode->closed = 0;
  }

  for (int i = 0; i<currentNode->num_children; i++) {
  openPollHelper(findnode(&mainnodes[1], currentNode->childNodes[i]->name), &mainnodes[1]);
  }

}

void* closePollHelper (node_t* currentNode, node_t* mainnodes) {
  if (currentNode->closed == -1 || currentNode->closed == 0) {
    currentNode->closed = 1;
  }
  for (int i = 0; i<currentNode->num_children; i++) {
  closePollHelper(findnode(&mainnodes[1], currentNode->childNodes[i]->name), &mainnodes[1]);
  }

}

return_t* add_votes2 (node_t* nodes, char* nodename, char* dataString) {

	return_t* result = malloc(sizeof(return_t));

	node_t* currentNode = findnode(&nodes[1], nodename);

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
	// Recursively bubble the additional votes up to parent nodes
	if (strlen(currentNode->parentName) != 0) {
		add_votes2(nodes, currentNode->parentName, dataString);
	}

	strcpy(result->code, "SC");
	strcpy(result->result, "\0");
	return result;
}

return_t* remove_votes2 (node_t* nodes, char* nodename, char* dataString) {

	return_t* result = malloc(sizeof(return_t));

	node_t* currentNode = findnode(&nodes[1], nodename);

  /*
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
  */

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


	i = 0;
	while (inputStrings[i] != '\0') {
		makeargv(inputStrings[i], ":", &candidateAndVotes);
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

	if (strlen(currentNode->parentName) != 0) {
		remove_votes2(nodes, currentNode->parentName, dataString);
	}

	strcpy(result->code, "SC");
	strcpy(result->result, "\0");
	return result;
}
