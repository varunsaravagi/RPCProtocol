#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <err.h>
#include <sys/stat.h>
#include "pack.h"
#include <errno.h>


/*----------------MYLIB.C---------------------------------------------------------
| File Mylib.c
| Description: 	The file contains the code base for the client side.
|		It has functions providing the following functionalities:
|		1. Establishing client connection with the server
|		2. Interposition functions for each of the required functions.
|
| Function Description: Each interposition function has similar structure and 
|			follows the mentioned steps:
|			1. Populate the packet structure with all the required fields
|			2. Call related serialization function.
|			3. Wait for data from the server. The data returned to the 
|			   function is deserialized.
|			4. Return the required parameter.
|
*-----------------------------------------------------------------------------------*/


int sockfd;
//extern int errno;

// The following lines declares a function pointer with the same prototype as the required library functions  
int (*orig_open)(const char *pathname, int flags, ...);  
int (*orig_close)(int fd);
ssize_t (*orig_read)(int fd, void *buf, size_t count);
ssize_t (*orig_write)(int fd, const void *buf, size_t count);
ssize_t (*orig_getdirentries)(int fd, char *buf, size_t nbytes, off_t *basep);
off_t (*orig_lseek)(int fd, off_t offset, int whence);
int (*orig_stat)(const char *path, struct stat *buf);
int (*orig_xstat)(int ver, const char *path, struct stat *buf);
int (*orig_unlink)(const char *pathname);
struct dirtreenode* (*orig_getdirtree)(const char *path);
void (*orig_freedirtree)(struct dirtreenode* dt);

// The following line declares the function for establising the client connection
int establish_client_conn();

// Replacement for standard functions begins here.
// Since each function has the same flow, comments have been written in the "open".
// If some function is deviating from the structure, the same has been mentioned in
// the function.

int open(const char *pathname, int flags, ...) {
	mode_t m=0;
	int sent = 0;
	int data_size, no_of_packets = 1;
	params data, datarcv;

	if (flags & O_CREAT) {
		va_list a;
		va_start(a, flags);
		m = va_arg(a, mode_t);
		va_end(a);
	}
	//fprintf(stderr, "Open params: path %s , flags %d, mode %d\n", pathname, flags, m);
	
	// populate params structure
	data.func_d = OPEN;
	data.type.open_in.flags = flags;
    	data.type.open_in.mode = m;
	data.type.open_in.pathname = pathname;
	
	// Get size of the packet. 3*sizeof(int) is the fixed size of the header of the packet
	data_size = 3*sizeof(int) + sizeof(data.type.open_in.flags) + sizeof(data.type.open_in.mode) + sizeof(size_t) + strlen(data.type.open_in.pathname);
	
	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;

	data.length = data_size;
	char* buf = (char*)calloc(1,data_size);
    
	//Call serialization function
	serialize_in_open(&data, buf);

	//send the full packet. Keep sending till the required packets are sent
	sent = send_all(sockfd, buf, no_of_packets, data_size);
	if (sent == -1)
			err(1,0);
	
	free(buf);	
	//Receive data back. The data received is already deserialized
	//params datarcv = receive_and_deserialize(sockfd);
	receive_and_deserialize(sockfd, &datarcv);
	errno = datarcv.type.open_out.err;
	//fprintf(stderr, "Open returned: FD %d, Errno %d\n", datarcv->type.open_out.fd, errno);
	return datarcv.type.open_out.fd;
}

ssize_t read(int fd, void *buf, size_t count) {
	//fprintf(stderr, "Read params: Fd %d, count %zu\n", fd, count);	
	
	//Check if the fd has been opened by server or not.
	//If not, call function on client side only.
	if (fd - FD_OFFSET < 0)
		return orig_read(fd,buf,count);
	
	int sent, data_size, no_of_packets;
	char* buffer;
	params data, datarcv;

	data.func_d = READ;
	data.type.read_in.fd = fd;
	data.type.read_in.count = count;

	data_size = 3*sizeof(int) + sizeof(fd) + sizeof(count);
	no_of_packets = get_no_of_packets(data_size);
	
	data.packets = no_of_packets;
	data.length = data_size;
	buffer = calloc(1,data_size);
	
	serialize_in_read(&data, buffer);
	sent = send_all(sockfd, buffer, no_of_packets, data_size);
	if (sent == -1)
		err(1,0);
	free(buffer);

	//params* datarcv = 
	receive_and_deserialize(sockfd, &datarcv);
	errno = datarcv.type.read_out.err;
	
	//data would only be present in buffer if some bytes were actually read
	if (datarcv.type.read_out.bytes_read > 0) {
		memcpy(buf, datarcv.type.read_out.buf, datarcv.type.read_out.bytes_read);
		free(datarcv.type.read_out.buf);
	}
	//fprintf(stderr, "Read returned: Read %d , Errno %d\n", datarcv->type.read_out.bytes_read, errno);	
	return datarcv.type.read_out.bytes_read;
}

int close(int fd) {
	//fprintf(stderr, "Close params: Fd %d\n", fd);	
	
	//Check if the fd has been opened by server or not.
	//If not, call function on client side only.
	if (fd - FD_OFFSET < 0)
		return orig_close(fd);
		
	int sent, data_size, no_of_packets;
	char* buf;
	params data, datarcv;

	data.func_d = CLOSE;
	data.type.close_in.fd = fd;
	
	data_size = 3*sizeof(int) + sizeof(data.type.close_in.fd);
	
	no_of_packets = get_no_of_packets(data_size);

	data.packets = no_of_packets;
	data.length = data_size;

	buf = calloc(1,data_size);
	
	serialize_in_close(&data, buf);
	sent = send_all(sockfd, buf, no_of_packets, data_size);
	if (sent == -1)
			err(1,0);
	
	free(buf);	

	//params* datarcv = 
	receive_and_deserialize(sockfd, &datarcv);
	errno = datarcv.type.close_out.err;
	//fprintf(stderr, "Close returned: Return %d , Errno %d\n", datarcv->type.close_out.ret, errno);	
	return datarcv.type.close_out.ret;
}

ssize_t write(int fd, const void *buf, size_t count) {
	//fprintf(stderr, "Write params: Fd %d , Count %d\n", fd, count);	
	//Check if the fd has been opened by server or not.
	//If not, call function on client side only.
	if (fd - FD_OFFSET < 0)
		return orig_write(fd,buf,count);

	int sent, data_size, no_of_packets;
	char* buffer;
	params data, datarcv;

	data.func_d = WRITE;
	data.type.write_in.fd = fd;
	data.type.write_in.count = count;
	data.type.write_in.buf = buf;

	data_size = 3*sizeof(int) + sizeof(data.func_d) + sizeof(data.type.write_in.fd) + sizeof(data.type.write_in.count) + count;
	
	data.length = data_size;
	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;

	buffer = calloc(1,data_size);
    	
	serialize_in_write(&data, buffer);
	
	sent = send_all(sockfd, buffer, no_of_packets, data_size);	
	if (sent == -1)
			err(1,0);

	free(buffer);	
	
	//params* datarcv = 
	receive_and_deserialize(sockfd, &datarcv);
	errno = datarcv.type.write_out.err;
	//fprintf(stderr, "Write returned: Written %d , Err %d\n", datarcv->type.write_out.bytes, errno);
	return datarcv.type.write_out.bytes;
}

off_t lseek(int fd, off_t offset, int whence) {
	//fprintf(stderr, "Lseek params: Fd %d , offset %d, whence %d\n", fd, offset, whence);	
	//Check if the fd has been opened by server or not.
	//If not, call function on client side only.
	if (fd - FD_OFFSET < 0)
		return orig_lseek(fd,offset,whence);

	int sent, data_size, no_of_packets;
	char* buffer;
	params data, datarcv;

	data.func_d = LSEEK;
	data.type.lseek_in.fd = fd;
	data.type.lseek_in.offset = offset;
	data.type.lseek_in.whence = whence;

	data_size = 3*sizeof(int) + sizeof(data.func_d) + sizeof(data.type.lseek_in.fd) + sizeof(data.type.lseek_in.offset) + sizeof(data.type.lseek_in.whence);
	
	data.length = data_size;
	no_of_packets = get_no_of_packets(data_size);
	data.packets = no_of_packets;

	buffer = calloc(1,data_size);
    	serialize_in_lseek(&data, buffer);
	
	sent = send_all(sockfd, buffer, no_of_packets, data_size);	
	if (sent == -1)
			err(1,0);

	free(buffer);	
	
	//params* datarcv = 
	receive_and_deserialize(sockfd, &datarcv);
	errno = datarcv.type.lseek_out.err;
	//fprintf(stderr, "Lseek returned: offset %d , Err %d\n", datarcv->type.lseek_out.offset, errno);	
	return datarcv.type.lseek_out.offset;
}

// stat is not implemented.
int stat(const char *path, struct stat *buf) {
	//fprintf(stderr, "In stat\n");
	
	return orig_stat(path, buf);
}

int __xstat(int ver, const char *path, struct stat *buf) {
	//fprintf(stderr,"-----IN XSTAT-------\n");
	int sent = 0;
	int data_size, no_of_packets = 1;
	params data, datarcv;

	data.func_d = XSTAT;
	data.type.xstat_in.ver = ver;
	data.type.xstat_in.path = path;
	data_size = 3*sizeof(int) + sizeof(data.type.xstat_in.ver) + sizeof(size_t) + strlen(data.type.open_in.pathname);
	
	no_of_packets = get_no_of_packets(data_size);

	data.packets = no_of_packets;
	data.length = data_size;
	char* buffer = (char*)calloc(1,data_size);
    
	serialize_in_xstat(&data, buffer);
	
	sent = send_all(sockfd, buffer, no_of_packets, data_size);
	if (sent == -1)
			err(1,0);
	
	free(buffer);	
	
	//params* datarcv = 
	receive_and_deserialize(sockfd, &datarcv);
		
	errno = datarcv.type.xstat_out.err;

	if (datarcv.type.xstat_out.ret == 0) {
		memcpy(buf, datarcv.type.xstat_out.buf, sizeof(struct stat));
		free(datarcv.type.xstat_out.buf);
	}
	//fprintf(stderr, "__Xstat returned: Return %d , Errno %d\n", datarcv.type.xstat_out.ret);
	
	return datarcv.type.xstat_out.ret;
}

int unlink(const char *pathname) {
	//fprintf(stderr, "Unlink params: path %s", pathname);	
	int sent = 0;
	int data_size, no_of_packets = 1;
	params data, datarcv;
	
	data.func_d = UNLINK;
	data.type.unlink_in.pathname = pathname;
	data_size = 3*sizeof(int) + sizeof(size_t) + strlen(data.type.unlink_in.pathname);
	
	no_of_packets = get_no_of_packets(data_size);

	data.packets = no_of_packets;
	data.length = data_size;
	char* buf = (char*)calloc(1,data_size);
    
	serialize_in_unlink(&data, buf);
	sent = send_all(sockfd, buf, no_of_packets, data_size);
	if (sent == -1)
			err(1,0);
	
	free(buf);	
	
	//params* datarcv = 
	receive_and_deserialize(sockfd, &datarcv);
	
	errno = datarcv.type.unlink_out.err;
	//fprintf(stderr, "Unlink returned: Return %d , Errno %d\n", datarcv->type.unlink_out.ret, errno);
	
	return datarcv.type.unlink_out.ret;
}

ssize_t getdirentries(int fd, char *buf, size_t nbytes, off_t *basep){
	//fprintf(stderr, "Getdirentries params: Fd %d, buffer %s , nbytes %d, basep %d\n", fd, buf, nbytes, *basep);
	//Check if the fd has been opened by server or not.
	//If not, call function on client side only.
	if (fd - FD_OFFSET < 0)
		return orig_getdirentries(fd,buf,nbytes,basep);
	
	int sent = 0;
	int data_size, no_of_packets = 1;
	params data, datarcv;
	
	data.func_d = GETDIRENTRIES;
	data.type.getdirentries_in.fd = fd;
    	data.type.getdirentries_in.nbytes = nbytes;
	data.type.getdirentries_in.basep = *basep;
	
	data_size = 3*sizeof(int) + sizeof(data.type.getdirentries_in.fd) + sizeof(data.type.getdirentries_in.nbytes) + sizeof(data.type.getdirentries_in.basep);

	no_of_packets = get_no_of_packets(data_size);

	data.packets = no_of_packets;
	data.length = data_size;
	char* buffer = (char*)calloc(1,data_size);
    
	serialize_in_getdirentries(&data, buffer);
	sent = send_all(sockfd, buffer, no_of_packets, data_size);
	if (sent == -1)
			err(1,0);
	
	free(buffer);	
	
	//params* datarcv = 
	receive_and_deserialize(sockfd, &datarcv);
	*basep = datarcv.type.getdirentries_out.basep;
	errno = datarcv.type.getdirentries_out.err;
	
	//data would only be present in buffer if some bytes were actually read
	if (datarcv.type.getdirentries_out.bytes_read > 0) {
		memcpy(buf, datarcv.type.getdirentries_out.buf, datarcv.type.getdirentries_out.bytes_read);
		free(datarcv.type.getdirentries_out.buf);
	}
	//fprintf(stderr, "Getdirentries returned: Read %d , Errno %d\n", datarcv.type.getdirentries_out.bytes_read, errno);
	
	return datarcv.type.getdirentries_out.bytes_read;
}

struct dirtreenode* getdirtree(const char *path){
	//fprintf(stderr, "Getdirtree params: path %s\n", path);	
	int sent = 0;
	int data_size, no_of_packets = 1;
	params data, datarcv;
	
	data.func_d = GETDIRTREE;
	data.type.getdirtree_in.path = path;
	
	data_size = 3*sizeof(int) + sizeof(size_t) + strlen(data.type.getdirtree_in.path);
	
	no_of_packets = get_no_of_packets(data_size);

	data.packets = no_of_packets;
	data.length = data_size;
	char* buf = (char*)calloc(1,data_size);
    
	serialize_in_getdirtree(&data, buf);
	sent = send_all(sockfd, buf, no_of_packets, data_size);
	if (sent == -1)
			err(1,0);
	
	free(buf);	

	
	//params* datarcv = 
	receive_and_deserialize(sockfd, &datarcv);
	
	errno = datarcv.type.getdirtree_out.err;
	return datarcv.type.getdirtree_out.tree;
}

void freedirtree(struct dirtreenode* dt) {
	//fprintf(stderr, "Freedirtree params\n");
	
	return orig_freedirtree(dt);
}

// This function is automatically called when program is started
void _init(void) {
	// set function pointer orig_open to point to the original open function
	orig_open = dlsym(RTLD_NEXT, "open");
	orig_close = dlsym(RTLD_NEXT, "close");
	orig_read = dlsym(RTLD_NEXT, "read");
	orig_write = dlsym(RTLD_NEXT, "write");
	orig_lseek = dlsym(RTLD_NEXT, "lseek");
	orig_stat = dlsym(RTLD_NEXT, "stat");
	orig_xstat = dlsym(RTLD_NEXT, "__xstat");
	orig_unlink = dlsym(RTLD_NEXT, "unlink");
	orig_getdirentries = dlsym(RTLD_NEXT, "getdirentries");
	orig_getdirtree = dlsym(RTLD_NEXT, "getdirtree");
	orig_freedirtree = dlsym(RTLD_NEXT, "freedirtree");
	sockfd = establish_client_conn();	
}

// This function establishes the connection with the server.
int establish_client_conn() {
	char *serverip;
	char *serverport;
	unsigned short port;
	int sockfd, rv;
	struct sockaddr_in srv;
	
	// Get environment variable indicating the ip address of the server
	serverip = getenv("server15440");
	if (serverip);
	else {
		serverip = "127.0.0.1";
	}
	
	// Get environment variable indicating the port of the server
	serverport = getenv("serverport15440");
	if (serverport);
	else {
		serverport = "15440";
	}
	port = (unsigned short)atoi(serverport);
	
	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	// TCP/IP socket
	if (sockfd<0) 
		err(1, 0);				// in case of error
	memset(&srv, 0, sizeof(srv));			// clear it first
	srv.sin_family = AF_INET;			// IP family
	srv.sin_addr.s_addr = inet_addr(serverip);	// IP address of server
	srv.sin_port = htons(port);			// server port

	rv = connect(sockfd, (struct sockaddr*)&srv, sizeof(struct sockaddr));
	if (rv<0) 
		err(1,0);
	return sockfd;
}

//This function is automatically called when exiting the program. Close the socket here.
void _fini(void) {
	orig_close(sockfd);
}

