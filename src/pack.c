#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<netdb.h>
#include<fcntl.h>
#include "pack.h"
#include <dirtree.h>
#include<errno.h>

/*----------------PACK.C---------------------------------
| Description: 	The file contains the following functions:
|		1. Serialization functions for all the in-parameters
|		2. Serialization functions for all the out-paramters
|		3. Determine the number of packets required
|		4. Send all the packets (no of packets)
|
|Related Header File: Pack.h
*---------------------------------------------------------*/


// Get the number of packets required
int get_no_of_packets(data_size) {
	int no_of_packets = 1;
	if (data_size > MAXMSGLEN){ 
		no_of_packets = data_size / MAXMSGLEN;
		no_of_packets += 1;	
	}
	return no_of_packets;
}

// Send all the packets
int send_all(int sockfd, char* buf, int no_of_packets, int data_size){
	int sent=0, total_sent = 0;
	while(no_of_packets > 0) {
		total_sent = 0;		
		char* buf_local = calloc(1,MAXMSGLEN);
		if (no_of_packets == 1){
			if(data_size == MAXMSGLEN)
				sent = data_size;
			else
				sent = data_size % MAXMSGLEN;
			//printf("To be sent %d\n", sent);
			memcpy(buf_local, buf, sent);
		}
		else							
			memcpy(buf_local, buf, MAXMSGLEN);

		while(total_sent < MAXMSGLEN){		
			sent = send(sockfd, buf_local+total_sent, MAXMSGLEN-total_sent, 0);
			//printf("Sent %d\n", sent);
			if (sent == -1)
				sent = 0; //try resending the same data
			total_sent += sent;
		}
		buf = (no_of_packets == 1) ? buf + 0 : buf + MAXMSGLEN;		
		no_of_packets -= 1;
		free(buf_local);
	}
	//total_sent = 0;
	//buf_local = calloc(1, data_size % MAXMSGLEN);
	//memcpy(buf_local, buf, data_size);
	//sent = send(sockfd, buf, data_size%MAXMSGLEN,0);
	return 0;
}


/*-------Start of Serialization In routines------------------*/

// Serialize the in-parameters for "open"
int serialize_in_open(params* p, char* buf){
	size_t offset = 0, length, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf + offset, &p->type.open_in.flags, size);
	offset += size;
		
	memcpy(buf + offset, &p->type.open_in.mode, sizeof(mode_t));
	offset += sizeof(mode_t);
	
	length = strlen(p->type.open_in.pathname);
	memcpy(buf + offset, &length, sizeof(size_t));
	
	offset += sizeof(size_t);
	memcpy(buf+offset, p->type.open_in.pathname, length);
	
	return 0;
}

// Serialize the in-parameters for "write"
int serialize_in_write(params *p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf + offset, &p->type.write_in.fd, size);
	offset += size;
		
	memcpy(buf + offset, &p->type.write_in.count, sizeof(size_t));
	offset += sizeof(size_t);

	memcpy(buf+offset, p->type.write_in.buf, p->type.write_in.count);
	offset+=p->type.write_in.count;
	
	return 0;
}

// Serialize the in-parameters for "close"
int serialize_in_close(params *p, char* buf) {
	size_t offset = 0, size = sizeof(int);

	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf + offset, &p->type.close_in.fd, size);
	offset += size;

	return 0;	
}

// Serialize the in-parameters for "read"
int serialize_in_read(params* p, char* buf){
	size_t offset = 0, size = sizeof(int);

	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf + offset, &p->type.read_in.fd, size);
	offset += size;
		
	memcpy(buf + offset, &p->type.read_in.count, sizeof(size_t));
	offset += sizeof(size_t);
	return 0;
}

// Serialize the in-parameters for "lseek"
int serialize_in_lseek(params* p, char* buf){
	size_t offset = 0, size = sizeof(int);

	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf + offset, &p->type.lseek_in.fd, size);
	offset += size;
		
	memcpy(buf + offset, &p->type.lseek_in.offset, sizeof(off_t));
	offset += sizeof(off_t);
	
	memcpy(buf + offset, &p->type.lseek_in.whence, size);
	offset += size;
	
	return 0;
}

// Serialize the in-parameters for "__xstat"
int serialize_in_xstat(params* p, char* buf){
	size_t offset = 0, length, size = sizeof(int);

	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.xstat_in.ver, size);
	offset += size;
	
	length = strlen(p->type.xstat_in.path);
	memcpy(buf+offset, &length, sizeof(size_t));
	offset += sizeof(size_t);
		
	memcpy(buf+offset, p->type.xstat_in.path, length);

	return 0;
}

// Serialize the in-parameters for "unlink"
int serialize_in_unlink(params* p, char* buf){
	size_t offset = 0, length, size = sizeof(int);

	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	length = strlen(p->type.unlink_in.pathname);
	memcpy(buf + offset, &length, sizeof(size_t));
	offset += sizeof(size_t);
	
	memcpy(buf+offset, p->type.unlink_in.pathname, length);

	return 0;
}

// Serialize the in-parameters for "getdirentries"
int serialize_in_getdirentries(params *p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf + offset, &p->type.getdirentries_in.fd, size);
	offset += size;
		
	memcpy(buf + offset, &p->type.getdirentries_in.nbytes, sizeof(size_t));
	offset += sizeof(size_t);
	
	memcpy(buf + offset, &p->type.getdirentries_in.basep, sizeof(off_t));
	offset += sizeof(off_t);

	return 0;
}

// Serialize the in-parameters for "getdirtree"
int serialize_in_getdirtree(params* p, char* buf){
	size_t offset = 0, length, size = sizeof(int);

	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	
	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	length = strlen(p->type.getdirtree_in.path);
	memcpy(buf + offset, &length, sizeof(size_t));
	offset += sizeof(size_t);
	
	memcpy(buf+offset, p->type.getdirtree_in.path, length);
	
	return 0;
}

/*-------------Start of Serialization out routines--------------*/

// Serialize the out-parameters for "open"
int serialize_out_open(params* p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;

	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.open_out.fd, size);
	offset += size;

	memcpy(buf + offset, &p->type.open_out.err, size);
	offset+=size;

	return 0;
}

// Serialize the out-parameters for "write"
int serialize_out_write(params* p, char* buf) {
	size_t offset = 0, size = sizeof(int);

	memcpy(buf+offset, &p->func_d, size);
	offset += size;
	//printf("Function code %d\n", func_d);
	//printf("Packet %u, no of packet %d\n", &p->packets, p->packets);
	memcpy(buf+offset, &p->packets, size);
	offset += size;
	
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.write_out.bytes, sizeof(ssize_t));
	offset += sizeof(ssize_t);

	memcpy(buf + offset, &p->type.write_out.err, size);
	offset+=size;

	return 0;
}

// Serialize the out-parameters for "close"
int serialize_out_close(params* p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;

	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.close_out.ret, size);
	offset += size;

	memcpy(buf + offset, &p->type.close_out.err, size);
	offset+=size;

	return 0;
}

// Serialize the out-parameters for "read"
int serialize_out_read(params* p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;

	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.read_out.bytes_read, sizeof(ssize_t));
	offset += sizeof(ssize_t);

	memcpy(buf + offset, &p->type.read_out.err, size);
	offset+=size;
	if (p->type.read_out.bytes_read > 0){
		memcpy(buf+offset, p->type.read_out.buf, p->type.read_out.bytes_read);
		offset+=p->type.read_out.bytes_read;
	}
	return 0;
}

// Serialize the out-parameters for "lseek"
int serialize_out_lseek(params* p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;

	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.lseek_out.offset, sizeof(off_t));
	offset += sizeof(off_t);

	memcpy(buf + offset, &p->type.lseek_out.err, size);
	offset+=size;

	return 0;
}

// Serialize the out-parameters for "__xstat"
int serialize_out_xstat(params* p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;

	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.xstat_out.ret, size);
	offset += size;

	memcpy(buf + offset, &p->type.xstat_out.err, size);
	offset+=size;
	
	if(p->type.xstat_out.ret == 0){
		memcpy(buf+offset, p->type.xstat_out.buf, sizeof(struct stat));
		offset+=sizeof(struct stat);
	}

	return 0;
}

// Serialize the out-parameters for "unlink"
int serialize_out_unlink(params* p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;

	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.unlink_out.ret, size);
	offset += size;

	memcpy(buf + offset, &p->type.unlink_out.err, size);
	offset+=size;
	
	return 0;
}

// Serialize the out-parameters for "getdirentries"
int serialize_out_getdirentries(params* p, char* buf) {
	size_t offset = 0, size = sizeof(int);
	
	memcpy(buf+offset, &p->func_d, size);
	offset += size;

	memcpy(buf+offset, &p->packets, size);
	offset += size;
   
	memcpy(buf+offset, &p->length, size);
	offset += size;

	memcpy(buf+offset, &p->type.getdirentries_out.bytes_read, sizeof(ssize_t));
	offset += sizeof(ssize_t);
	
	memcpy(buf+offset, &p->type.getdirentries_out.basep, sizeof(off_t));
	offset += sizeof(off_t);
	
	memcpy(buf + offset, &p->type.getdirentries_out.err, size);
	offset+=size;

	if(p->type.getdirentries_out.bytes_read >= 0){
		memcpy(buf+offset, p->type.getdirentries_out.buf, p->type.getdirentries_out.bytes_read);
		offset+=p->type.getdirentries_out.bytes_read;
	}

	return 0;
}

/*-------------------This routine will serialize the tree structure
----------returned by getdirtree recursively-----------------*/
void serialize_tree(struct dirtreenode* tree, char* buffer_tree, size_t *offset){
	size_t length;
	int i;
	
	length = strlen(tree->name);
	memcpy(buffer_tree+ *offset, &length, sizeof(size_t));
	*offset += sizeof(size_t);
	
	memcpy(buffer_tree+ *offset, tree->name, length);
	*offset+=length;
	
	memcpy(buffer_tree+ *offset, &tree->num_subdirs, sizeof(int));
	*offset+=sizeof(int);
	for(i=0;i<tree->num_subdirs;i++){
		serialize_tree((tree->subdirs[i]),buffer_tree, offset);
	}
}

/*-----------------END OF FILE------------------------*/
