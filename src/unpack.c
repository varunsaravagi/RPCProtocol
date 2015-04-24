#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<netdb.h>
#include "pack.h"
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <dirtree.h>
#include <math.h>


/*-----------------UNPACK.C------------------------------------------------
|Description: 	The file contains the following functionalities
|		1. Receive packet from server and deserialize it
|		2. Deserialize the packet the server got, call the required
|		   function, serialize the out parameters and send to client
|
|Related Header File: Pack.h
*--------------------------------------------------------------------------*/

// Receive the parameters from server and deserialize according to the function code
void receive_and_deserialize(int sockfd, params *data) {
	//printf("Receive and deserialize\n");
	ssize_t rv, rv_total;
	size_t offset = 0, size = sizeof(int), offset_tree = 0;
	char buf[MAXMSGLEN];
	//params data;
	
	if ((rv=recv(sockfd,buf,MAXMSGLEN,0))>0){

		rv_total = rv;
		//fprintf(stderr,"Received %d\n", rv);
		while(rv_total < MAXMSGLEN){
			rv = recv(sockfd, buf+rv_total, MAXMSGLEN-rv_total, 0);
			if (rv == -1)
				rv = 0;			
			rv_total += rv;
		}
		
		int  no_of_packets=0;
		// Get function code
		memcpy(&data->func_d, buf+offset, size);
		offset += size;
		// Get no of packets
		memcpy(&no_of_packets, buf+offset, size);
		data->packets = no_of_packets;
		offset += size;
		
		memcpy(&data->length, buf+offset, size);
		offset += size;
		//printf("Length %d\n", data->length);

		//size_t length_recv = rv - 3*sizeof(int);	
		//buf_local = (char *)calloc(1,data.length);
		
		// copy out parameters
		//memcpy(buf_local, buf+offset, length_recv);
		//offset = length_recv;

		//allocate local buffer to copy the parameters. Local buffer
		//is allocated memory equal to the length of parameters expected.	
		int length_of_params = data->length - 3*sizeof(int);		
		char* buf_local = (char*)calloc(1,length_of_params);
		int length_to_copy = data->length >= MAXMSGLEN ? MAXMSGLEN-3*sizeof(int) : length_of_params; 				
		memcpy(buf_local, buf+offset, length_to_copy);
		offset = length_to_copy;
		//if >1 packets are to required, get them
		//printf("No of packets %d %u\n", no_of_packets, &no_of_packets);
		while(no_of_packets > 1) {
			memset(buf, '\0', MAXMSGLEN);
			//rv = recv(sockfd,buf,MAXMSGLEN,0);
			
			rv_total = 0;
			while(rv_total < MAXMSGLEN){
				rv = recv(sockfd, buf+rv_total, MAXMSGLEN-rv_total, 0);
				if (rv == -1)
					rv = 0;			
				rv_total += rv;
			}			
			
			length_to_copy = offset+MAXMSGLEN > length_of_params ? length_of_params - offset : MAXMSGLEN;				
			memcpy(buf_local+offset, buf, length_to_copy);

			no_of_packets -= 1;
			offset += MAXMSGLEN;
		}

		offset = 0;
		// Deserialize according to the function code
		switch(data->func_d) {
			case OPEN:
				memcpy(&data->type.open_out.fd, buf_local+offset, size);
				offset += size;
				
				memcpy(&data->type.open_out.err, buf_local+offset, size);
				break;

			case WRITE:
				memcpy(&data->type.write_out.bytes, buf_local+offset, sizeof(ssize_t));
				offset += sizeof(ssize_t);
			
				memcpy(&data->type.write_out.err, buf_local+offset, size);
				break;

			case CLOSE:
				memcpy(&data->type.close_out.ret, buf_local+offset, size);
				offset += size;

				memcpy(&data->type.close_out.err, buf_local+offset, size);
				break;

			 case READ:
				memcpy(&data->type.read_out.bytes_read, buf_local+offset, sizeof(ssize_t));
				offset+=sizeof(ssize_t);

				memcpy(&data->type.read_out.err, buf_local+offset, size);
				offset += size;
				if (data->type.read_out.bytes_read > 0){
					data->type.read_out.buf = calloc(1,data->type.read_out.bytes_read);
					memcpy(data->type.read_out.buf, buf_local+offset, data->type.read_out.bytes_read);
				}
				break;
			
			 case LSEEK:
				memcpy(&data->type.lseek_out.offset, buf_local+offset, sizeof(off_t));
				offset += sizeof(off_t);
				
				memcpy(&data->type.lseek_out.err, buf_local+offset, size);
				break;

			case XSTAT:
				memcpy(&data->type.xstat_out.ret, buf_local+offset, size);
				offset += size;

				memcpy(&data->type.xstat_out.err, buf_local+offset, size);
				offset += size;
				if (data->type.xstat_out.ret == 0){
					data->type.xstat_out.buf = calloc(1,sizeof(struct stat));
					memcpy(data->type.xstat_out.buf, buf_local+offset, sizeof(struct stat));
				}
				break;

			case UNLINK:
				memcpy(&data->type.unlink_out.ret, buf_local+offset, size);
				offset += size;
				
				memcpy(&data->type.unlink_out.err, buf_local+offset, size);
				break;

			case GETDIRENTRIES:
				memcpy(&data->type.getdirentries_out.bytes_read, buf_local+offset, sizeof(ssize_t));
				offset += sizeof(ssize_t);

				memcpy(&data->type.getdirentries_out.basep, buf_local+offset, sizeof(off_t));
				offset += sizeof(off_t);

				memcpy(&data->type.getdirentries_out.err, buf_local+offset, size);
				offset += size;
	
				if (data->type.getdirentries_out.bytes_read > 0){
					data->type.getdirentries_out.buf = calloc(1,data->type.getdirentries_out.bytes_read);
					memcpy(data->type.getdirentries_out.buf, buf_local+offset, data->type.getdirentries_out.bytes_read);
				}
				break;

			case GETDIRTREE:
				memcpy(&data->type.getdirtree_out.err, buf_local + offset, size);
				offset += size;
				if (data->type.getdirtree_out.err >= 0){
					data->type.getdirtree_out.tree = deserialize_tree(data->type.getdirtree_out.tree, buf_local+offset, &offset_tree);
					offset += offset_tree;
				}
				break;
		}
		free(buf_local);	
	}
}

/*------------Start of Deserialization-in routines---------------*/

// Deserialize and send the packet for "open"
void deserialize_in_open_send(char* buf, int sessfd){
	size_t offset = 0, size = sizeof(int), length;
	int flags, fd, sent, data_size, no_of_packets;	
	char* pathname;
	char* buffer;
	params data;

	mode_t mode;
	
	memcpy(&flags, buf + offset, size);
	offset += size;
	
	memcpy(&mode, buf+offset, sizeof(mode_t));
	offset+=sizeof(mode_t);	
	
	memcpy(&length, buf+offset, sizeof(size_t));
	offset+=sizeof(size_t);	
	
	pathname = (char *)malloc(length+1);
	memcpy(pathname, buf+offset, length);
	pathname[length] = 0;
	
	//call open
	fd = open(pathname,flags,mode);
	//add offset to file descriptor to prevent conflict b/w client and server offsets	
	if(fd>0)	
		fd += FD_OFFSET;
	
	data.type.open_out.fd = fd;
	data.type.open_out.err = errno;
	data_size = 3*sizeof(int) + sizeof(data.type.open_out.fd) + sizeof(errno);
	data.length = data_size;
	data.func_d = OPEN;

	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;

	buffer = calloc(1,data_size);
	
	//serialize the out parameters
	serialize_out_open(&data, buffer);
	
	//send the packet
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	if (sent == -1)
		err(1,0);
	free(pathname);
	free(buffer);
}

// Deserialize and send the packet for "write"
void deserialize_in_write_send(char* buf, int sessfd){	
	//printf("Deserialize write\n");
	int fd, sent, data_size, no_of_packets;	
   	size_t count, size = sizeof(int), offset = 0;
	void* content;
	char* buffer;
	params data;
		
	memcpy(&fd, buf+offset, size);
	offset += size;
	//subtract offset to get the original created fd
	if (fd > 0)	
		fd -= FD_OFFSET;

	memcpy(&count, buf+offset, sizeof(size_t));
	offset+=sizeof(size_t);	
	
	content = calloc(1,count);

	memcpy(content, buf+offset, count);
	
	data.func_d = WRITE;
	
	//call write	
	data.type.write_out.bytes  = write(fd,content,count);
	
	data.type.write_out.err = errno;
	data_size = 3*sizeof(int) + sizeof(data.type.write_out.bytes) + sizeof(errno);
	data.length = data_size;

	buffer = calloc(1,data_size);
	//printf("Data size %d\n", data_size);
	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;
	//printf("No of packets %d %d\n", data.packets, no_of_packets);
	//serialize out parameters
	serialize_out_write(&data, buffer);
	
	//send the packet
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	if (sent == -1)
		err(1,0);
	//printf("Sent\n");
	free(content);
	free(buffer);
}

// Deserialize and send the packet for "close"
void deserialize_in_close_send(char* buf, int sessfd){
	size_t offset = 0, size = sizeof(int);
	int fd, sent, data_size, no_of_packets;	
	char* buffer;
	params data;
	
	memcpy(&fd, buf + offset, size);
	offset += size;
	if (fd>0)	
		fd -= FD_OFFSET;
	
	//call close	
	data.type.close_out.ret = close(fd);
	data.type.close_out.err = errno;
	
	data_size = 3*sizeof(int) + sizeof(data.type.close_out.ret) + sizeof(errno);
	no_of_packets = get_no_of_packets(data_size);
	data.func_d = CLOSE;
	data.length = data_size;
	data.packets = no_of_packets;

	buffer = malloc(data_size);
	
	// serialize out parameters
	serialize_out_close(&data, buffer);
	
	// send the packet
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	if (sent == -1)
		err(1,0);	
	free(buffer);
}

// Deserialize and send the packet for "read"
void deserialize_in_read_send(char* buf, int sessfd){	
	//printf("Deserialize read\n");
	size_t offset = 0, size = sizeof(int), count;
	int fd, sent, data_size, no_of_packets;	
	void* content;
	char* buffer;
	params data;
		
	memcpy(&fd, buf+offset, size);
	offset += size;
	if (fd > 0)	
		fd -= FD_OFFSET;	
	
	memcpy(&count, buf+offset, sizeof(size_t));
	offset+=sizeof(size_t);	

	data.func_d = READ;
	content = calloc(1,count);
	
	//call read	
	data.type.read_out.bytes_read = read(fd,content,count);
	
	data.type.read_out.err = errno;
	data_size = 3*sizeof(int) + sizeof(data.type.read_out.bytes_read) + sizeof(errno);
	
	// only allocate memory for buffer read if there is no error in read
	if (data.type.read_out.bytes_read > 0) {
		data.type.read_out.buf = calloc(1,data.type.read_out.bytes_read);
		memcpy(data.type.read_out.buf, content, data.type.read_out.bytes_read);
		data_size += data.type.read_out.bytes_read;
	}

	data.length = data_size;
	
	buffer = calloc(1,data_size);

	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;
	
	//serialize out parameters
	serialize_out_read(&data, buffer);

	if(data.type.read_out.bytes_read > 0)
		free(data.type.read_out.buf);
	
	// send the packet
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	//printf("Sent\n");
	if (sent == -1)
			err(1,0);
	free(content);
	free(buffer);
}

// Deserialize and send the packet for "lseek"
void deserialize_in_lseek_send(char* buf, int sessfd){	
	size_t offset = 0, size = sizeof(int);
	int fd, sent, data_size, no_of_packets, whence;	
   	off_t l_offset;
	char* buffer;
	params data;
		
	memcpy(&fd, buf+offset, size);
	offset += size;
	if (fd > 0)	
		fd -= FD_OFFSET;

	memcpy(&l_offset, buf+offset, sizeof(off_t));
	offset+=sizeof(off_t);	
	
	memcpy(&whence, buf+offset, size);
	offset+=size;

	data.func_d = LSEEK;
	
	//call lseek
	data.type.lseek_out.offset = lseek(fd,l_offset,whence);

	data.type.lseek_out.err = errno;
	data_size = 3*sizeof(int) + sizeof(data.type.lseek_out.offset) + sizeof(errno);
	data.length = data_size;
	
	buffer = calloc(1,data_size);
	
	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;
	
	//serialize out parameters
	serialize_out_lseek(&data, buffer);

	//send the packet
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	if (sent == -1)
			err(1,0);
	free(buffer);
}

// Deserialize and send the packet for "__xstat"
void deserialize_in_xstat_send(char* buf, int sessfd){
	size_t offset = 0, size = sizeof(int), length;
	int ver, ret, sent, data_size, no_of_packets;	
	char* path;
	char* buffer;
	struct stat st;
	params data;
	
	memcpy(&ver, buf + offset, size);
	offset += size;
	
	memcpy(&length, buf+offset, sizeof(size_t));
	offset+=sizeof(size_t);	
	
	path = (char *)calloc(1,length);
	memcpy(path, buf+offset, length);
	//path[length]=0;

	// call __xstat	
	ret = __xstat(ver, path, &st);
	data.type.xstat_out.ret = ret;
	data.type.xstat_out.err = errno;
	data_size = 3*sizeof(int) + sizeof(data.type.xstat_out.ret) + sizeof(data.type.xstat_out.err);
	if (ret == 0){
		data.type.xstat_out.buf = malloc(sizeof(struct stat));
		memcpy(data.type.xstat_out.buf, &st, sizeof(struct stat));
		data_size += sizeof(struct stat);
	}

	data.length = data_size;
	data.func_d = XSTAT;

	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;

	buffer = calloc(1,data_size);
	
	//serialize the out parameters
	serialize_out_xstat(&data, buffer);
	
	//send packet
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	if (sent == -1)
		err(1,0);
	free(path);
	if (ret==0)
		free(data.type.xstat_out.buf);
	free(buffer);
}

// Deserialize and send the packet for "unlink"
void deserialize_in_unlink_send(char* buf, int sessfd){
	size_t offset = 0, length;
	int ret, sent, data_size, no_of_packets;	
	char* pathname;
	char* buffer;
	params data;
	
	memcpy(&length, buf+offset, sizeof(size_t));
	offset+=sizeof(size_t);	
	
	pathname = (char *)malloc(length+1);
	memcpy(pathname, buf+offset, length);
	pathname[length+1]=0;
	
	// call unlink
	ret = unlink(pathname);
	data.type.unlink_out.ret = ret;
	data.type.unlink_out.err = errno;

	data_size = 3*sizeof(int) + sizeof(data.type.unlink_out.ret) + sizeof(errno);
	data.length = data_size;
	data.func_d = UNLINK;

	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;

	buffer = calloc(1,data_size);

	//serialize the out parameters
	serialize_out_unlink(&data, buffer);
	
	//send the packet
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	if (sent == -1)
		err(1,0);
	free(pathname);
	free(buffer);
}

// Deserialize and send the packet for "getdirentries"
void deserialize_in_getdirentries_send(char* buf, int sessfd){	
	size_t offset = 0, size = sizeof(int), nbytes; 
	int fd, sent, data_size, no_of_packets;	
	off_t basep;
	char* content;
	char* buffer;
	params data;

	memcpy(&fd, buf+offset, size);
	offset += size;
	if (fd > 0)	
		fd -= FD_OFFSET;	

	memcpy(&nbytes, buf+offset, sizeof(size_t));
	offset+=sizeof(size_t);	
	
	memcpy(&basep, buf+offset, sizeof(off_t));
	offset+=sizeof(off_t);	
	
	content = calloc(1,nbytes);

	data.func_d = GETDIRENTRIES;

	//call getdirentries	
	data.type.getdirentries_out.bytes_read  = getdirentries(fd, content, nbytes, &basep);
	data.type.getdirentries_out.err = errno;
	
	data_size = 3*sizeof(int) + sizeof(data.type.getdirentries_out.bytes_read) + sizeof(errno) + sizeof(off_t);
	data.length = data_size;
	
	// only allocate memory for buffer read if bytes have been read
	if (data.type.getdirentries_out.bytes_read > 0) {
		data.type.getdirentries_out.buf = calloc(1,data.type.getdirentries_out.bytes_read);
		memcpy(data.type.getdirentries_out.buf, content, data.type.getdirentries_out.bytes_read);
		data_size += data.type.getdirentries_out.bytes_read;
		data.length = data_size;
	}
	
	buffer = calloc(1,data_size);
	
	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;
	
	//serialize out parameters
	serialize_out_getdirentries(&data, buffer);
	
	//send the packet
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	if (sent == -1)
			err(1,0);
	free(content);
	if (data.type.getdirentries_out.bytes_read > 0)
		free(data.type.getdirentries_out.buf);
	free(buffer);
}

// Deserialize and send the packet for "getdirtree"
void deserialize_in_getdirtree_send(char* buf, int sessfd){
	size_t offset = 0, offset_tree = 0, length, size_tree = 0, size = sizeof(int);
	int sent, data_size, func_d, no_of_packets;
	char* pathname;
	char* buffer;
	
	memcpy(&length, buf+offset, sizeof(size_t));
	offset+=sizeof(size_t);	
	
	pathname = calloc(1,length+1);
	memcpy(pathname, buf+offset, length);
	
	//call getdirtree	
	struct dirtreenode *tree  = getdirtree(pathname);		

	//linearize the tree structure
	char* buffer_tree = NULL;
	if(tree != NULL){
		gettreesize(tree, &size_tree);
		buffer_tree = calloc(1,size_tree);
		serialize_tree(tree, buffer_tree, &offset_tree);
	}
	
	data_size = 3*sizeof(int) + sizeof(errno) + size_tree;
	func_d = GETDIRTREE;
	no_of_packets = get_no_of_packets(data_size);
	
	buffer = calloc(1,data_size);
	offset = 0;
	
	memcpy(buffer+offset, &func_d, size);
	offset += size;

	memcpy(buffer+offset, &no_of_packets, size);
	offset += size;
	
	memcpy(buffer+offset, &data_size, size);
	offset += size;
	
	memcpy(buffer+offset, &errno, size);
	offset += size;

	if (tree != NULL){
		memcpy(buffer+offset, buffer_tree, size_tree);
		offset += size_tree;
	}
	
	// send the packer	
	sent = send_all(sessfd, buffer, no_of_packets, data_size);
	if (sent == -1)
		err(1,0);
	free(pathname);
	if (tree!= NULL){
		free(buffer_tree);
		freedirtree(tree);
	}
	free(buffer);
}
/*------------------End of Deserializing-In routines---------------*/

// This function takes the serialized input and returns the dirtreenode*
struct dirtreenode* deserialize_tree(struct dirtreenode *treecopy, char* buffer, size_t *offset){
	treecopy = calloc(1, sizeof(struct dirtreenode));
	size_t length;
	int i;

	memcpy(&length, buffer+ *offset, sizeof(size_t));
	*offset+=sizeof(size_t);
	
	treecopy->name = calloc(1,length+1);
	
	memcpy(treecopy->name, buffer+ *offset, length);
	treecopy->name[length+1]=0;
	*offset+=length;
	
	memcpy(&treecopy->num_subdirs, buffer+ *offset, sizeof(int));
	*offset += sizeof(int);

	//allocate memory to the subdirs** structure		
	if (treecopy->num_subdirs > 0){
		treecopy->subdirs = calloc(1, treecopy->num_subdirs * sizeof(struct dirtreenode*));
	}
	//recursively build the tree
	for (i=0; i< treecopy->num_subdirs; i++){
		treecopy->subdirs[i] = deserialize_tree((treecopy->subdirs[i]), buffer, offset);
	}

	return treecopy;
}

// This function returns the total size of the tree
void gettreesize(struct dirtreenode* treecopy, size_t* size_tree){
	int i;
	size_t length = strlen(treecopy->name);
	*size_tree += sizeof(size_t) + length + sizeof(int);
	 
	for(i = 0; i < treecopy->num_subdirs;i++){
		gettreesize((treecopy->subdirs[i]), size_tree);
	}
}
