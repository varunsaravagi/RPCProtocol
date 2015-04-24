#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>


void main(){
	ssize_t bytes;
	unsigned int temp;

	bytes = -1;
	temp = htonl(temp);
	printf("size of ssize_t %zd int %zd\n", sizeof(ssize_t), sizeof(int));
	fprintf(stderr, "Bytes %zd\n", bytes);
	fprintf(stderr, "temp %u\n", temp);
	bytes = ntohl(temp);
	printf("Bytes %zd\n", bytes);

}
