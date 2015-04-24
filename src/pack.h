#define OPEN 1
#define WRITE 2
#define CLOSE 3
#define READ 4
#define LSEEK 5
#define XSTAT 6
#define UNLINK 7
#define GETDIRENTRIES 8
#define GETDIRTREE 9
#define FREEDIRTREE 10

#define MAXMSGLEN 1500
#define FD_OFFSET 65536
#include <sys/types.h>

/*-------------------PACK.H-----------------------------------
|Description: 	This is the main header file and contains:
|		1. the type declarations for all the in-out
|		structures and the packet structure.
| 		2. the function declarations (used in mylib.c, pack.c,
|		unpack.c, server.c)
|
*-------------------------------------------------------------*/

/*------------------Start of---
-----type declarations for all the in parameters----*/
typedef struct {
	int flags;
	mode_t mode;
	char* pathname;
}open_struct_in;

typedef struct {
	int fd;
	size_t count;
	void* buf;
}write_struct_in;

typedef struct {
	int fd;
}close_struct_in;

typedef struct {
	int fd;
	size_t count;
}read_struct_in;

typedef struct {
	int fd;
	off_t offset;
	int whence;
}lseek_struct_in;

typedef struct {
	int ver;
	char* path;
}xstat_struct_in;

typedef struct {
	char *pathname;
}unlink_struct_in;

typedef struct{
	int fd;
	size_t nbytes;
	off_t basep;
}getdirentries_struct_in;

typedef struct{
	char* path;
}getdirtree_struct_in;



/*------------------Start of---
-----type declarations for all the out parameters----*/
typedef struct {
	int fd;
	int err;
}open_struct_out;

typedef struct {
	ssize_t bytes;
	int err;
}write_struct_out;

typedef struct {
	int ret;
	int err;
}close_struct_out;

typedef struct {
	ssize_t bytes_read;
	int err;
	void* buf;
}read_struct_out;

typedef struct {
	off_t offset;
	int err;
}lseek_struct_out;

typedef struct {
	int ret;
	int err;
	struct stat* buf;
}xstat_struct_out;

typedef struct {
	int ret;
	int err;
}unlink_struct_out;

typedef struct {
	ssize_t bytes_read;
	off_t basep;
	int err;
	char* buf;
}getdirentries_struct_out;
	
typedef struct {
	int err;
	struct dirtreenode* tree;
}getdirtree_struct_out;

// all the above declared structures are packed in a union
typedef union {
	open_struct_in	open_in;
	write_struct_in write_in;
	close_struct_in close_in;
	read_struct_in read_in;
	lseek_struct_in lseek_in;
	xstat_struct_in xstat_in;
	unlink_struct_in unlink_in;
	getdirentries_struct_in getdirentries_in;
	getdirtree_struct_in getdirtree_in;
	
	open_struct_out open_out;
	write_struct_out write_out;
	close_struct_out close_out;
	read_struct_out read_out;
	lseek_struct_out lseek_out;
	xstat_struct_out xstat_out;
	unlink_struct_out unlink_out;
	getdirentries_struct_out getdirentries_out;
	getdirtree_struct_out getdirtree_out;
}func_type;

// Structure of the packet
typedef struct {
	int func_d;
	int packets;
	int length;
	func_type type;
}params;


/*------------Function declarations-------------------*/

//Implemented in pack.c
int get_no_of_packets(int data_size);
int send_all(int sockfd, char* buf, int no_of_packets, int data_size);
int serialize_in(params* p, void* buf);
int serialize_in_open(params* p, char* buf);
int serialize_in_write(params* p, char* buf);
int serialize_in_close(params* p, char* buf);
int serialize_in_lseek(params* p, char* buf);
int serialize_in_read(params* p, char* buf);
int serialize_in_xstat(params* p, char* buf);
int serialize_in_unlink(params* p, char* buf);
int serialize_in_getdirentries(params* p, char* buf);
int serialize_in_getdirtree(params* p, char* buf);

int serialize_out_open(params* p, char* buf);
int serialize_out_write(params* p, char* buf);
int serialize_out_close(params* p, char* buf);
int serialize_out_read(params* p, char* buf);
int serialize_out_lseek(params* p, char* buf);
int serialize_out_xstat(params* p, char* buf);
int serialize_out_unlink(params* p, char* buf);
int serialize_out_getdirentries(params* p, char* buf);

void serialize_tree(struct dirtreenode* tree, char* buffer, size_t* offset);

// implemented in unpack.c
void deserialize_in_open_send(char* buf, int sessfd);
void deserialize_in_write_send(char* buf, int sessfd);
void deserialize_in_close_send(char* buf, int sessfd);
void deserialize_in_read_send(char* buf, int sessfd);
void deserialize_in_lseek_send(char* buf, int sessfd);
void deserialize_in_xstat_send(char* buf, int sessfd);
void deserialize_in_unlink_send(char* buf, int sessfd);
void deserialize_in_getdirentries_send(char* buf, int sessfd);
void deserialize_in_getdirtree_send(char* buf, int sessfd);

void receive_and_deserialize(int sockfd, params* data);
struct dirtreenode* deserialize_tree(struct dirtreenode* treecopy, char* buffer, size_t* offset);
void gettreesize(struct dirtreenode* treecopy, size_t* size_tree);
