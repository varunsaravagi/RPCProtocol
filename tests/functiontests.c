#include<stdio.h>
#include<stdlib.h>
#include <string.h>
void main(int argc, char **argv) {
	char *p = "hello123456";
	int size = sizeof(p);
	printf("string %s\n", p);
	printf("size of p: %d strlen %d\n", size,strlen(p));
}
