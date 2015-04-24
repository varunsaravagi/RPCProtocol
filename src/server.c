#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include "pack.h"
#include <dirtree.h>


/*--------------------SERVER.C---------------------------------
|Description: 	This file contains the code for the server.
|		1. It receives packet from the client, separates the
|		header and passes the remaining packet to the 
|		appropriate deserialization routine
|		2. It also contains code to handle multiple clients
|
*---------------------------------------------------------------*/

// Function declaration to receive from client
void receive_from_client(int sessfd);

int main(int argc, char**argv) {
	char *serverport;
	unsigned short port;
	int sockfd, sessfd, rv, i;
	struct sockaddr_in srv, cli;
	socklen_t sa_size;
	
	// Get environment variable indicating the port of the server
	serverport = getenv("serverport15440");
	if (serverport) port = (unsigned short)atoi(serverport);
	else port=15440;
	
	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	// TCP/IP socket
	if (sockfd<0) err(1, 0);			// in case of error
	
	// setup address structure to indicate server port
	memset(&srv, 0, sizeof(srv));			// clear it first
	srv.sin_family = AF_INET;			// IP family
	srv.sin_addr.s_addr = htonl(INADDR_ANY);	// don't care IP address
	srv.sin_port = htons(port);			// server port

	// bind to our port
	rv = bind(sockfd, (struct sockaddr*)&srv, sizeof(struct sockaddr));
	if (rv<0) err(1,0);
	
	// start listening for connections
	rv = listen(sockfd, 5);
	if (rv<0) err(1,0);
	while(1) {
		// wait for client, get session socket
		sa_size = sizeof(struct sockaddr_in);
		sessfd = accept(sockfd, (struct sockaddr *)&cli, &sa_size);
		if (sessfd<0) err(1,0);
		rv = fork();
		if (rv==0){
			close(sockfd);
			receive_from_client(sessfd);
			close(sessfd);
			exit(0);
		}		
		close(sessfd);
	}
	close(sockfd);
	return 0;
}


// Function definition to get packet from client
void receive_from_client(int sessfd){
	char buf[MAXMSGLEN];
	size_t rv, rv_total;
	while((rv=recv(sessfd,buf,MAXMSGLEN,0))>0) {
			size_t offset = 0, size = sizeof(int);
			int func_d, no_of_packets, length;
			
			rv_total = rv;
			//printf("Server Received %d\n", rv);
			while(rv_total < MAXMSGLEN){
				rv = recv(sessfd, buf+rv_total, MAXMSGLEN-rv_total, 0);
				if (rv == -1)
					rv = 0;			
			rv_total += rv;
			}

			memcpy(&func_d, buf, size);
			offset += size;
			//printf("Function code %d\n", func_d);
			memcpy(&no_of_packets, buf+offset, size);
			offset += size;

			memcpy(&length, buf+offset, size);
			offset += size;
			//printf("Length %d\n", length);

			//allocate local buffer to copy the parameters. Local buffer
			//is allocated memory equal to the length of parameters expected.
			int length_of_params = length - 3*sizeof(int);	
			char* buf_local = (char*)calloc(1,length_of_params);
			int length_to_copy = length >= MAXMSGLEN ? MAXMSGLEN-3*sizeof(int) : length_of_params; 				
			memcpy(buf_local, buf+offset, length_to_copy);
			offset = length_to_copy;

			// get additional packets if required
			//while(no_of_packets > 1) {
			//	rv = recv(sessfd,buf,MAXMSGLEN,0);
			//	memcpy(buf_local+offset, buf, rv);
			//	no_of_packets -= 1;
			//	offset+=rv;
			//}
			while(no_of_packets > 1) {
				memset(buf, '\0', MAXMSGLEN);
				//rv = recv(sockfd,buf,MAXMSGLEN,0);
			
				rv_total = 0;
				while(rv_total < MAXMSGLEN){
					rv = recv(sessfd, buf+rv_total, MAXMSGLEN-rv_total, 0);
					if (rv == -1)
						rv = 0;			
					rv_total += rv;
				}			
				length_to_copy = offset+MAXMSGLEN > length_of_params ? length_of_params - offset : MAXMSGLEN;				
				memcpy(buf_local+offset, buf, length_to_copy);
				no_of_packets -= 1;
				offset += MAXMSGLEN;
			}

			// pass the parameter data to the appropriate deserialization routine
			switch(func_d) {
				case OPEN:
					deserialize_in_open_send(buf_local, sessfd);
					break;
				case CLOSE:
					deserialize_in_close_send(buf_local, sessfd);
					break;
				case WRITE:
					deserialize_in_write_send(buf_local, sessfd);
					break;
				case READ:
					deserialize_in_read_send(buf_local, sessfd);
					break;
				case LSEEK:
					deserialize_in_lseek_send(buf_local, sessfd);
					break;
				case XSTAT:
					deserialize_in_xstat_send(buf_local, sessfd);
					break;
				case UNLINK:
					deserialize_in_unlink_send(buf_local, sessfd);
					break;
				case GETDIRENTRIES:
					deserialize_in_getdirentries_send(buf_local,sessfd);
					break;
				case GETDIRTREE:
					deserialize_in_getdirtree_send(buf_local, sessfd);
					break;
			}
			free(buf_local);
		}
}
