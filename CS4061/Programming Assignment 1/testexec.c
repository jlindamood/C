#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int main() {
	execl("./leafcounter", "./leafcounter", "Region_4 Output_Region_4 3 A B C", (char *)NULL);
}