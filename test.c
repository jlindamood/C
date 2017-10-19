#include <stdio.h>

void helloWorld() {
	printf("Hello World!\n");
}

void function(char* arg) {
	
	int result;

	result = arg[0] - '0';

	printf("%d\n", result);

}

void main() {
	helloWorld();
	function("b");
}