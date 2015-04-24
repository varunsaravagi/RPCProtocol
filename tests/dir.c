#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <dirtree.h>

int main(int argc, char**argv) {
	struct dirtreenode* dir = NULL;
	const char *path = "../";
	int fd, rv;
	char *buf;
	struct stat st;

	if (argc<2) {
		//printf("need to specify path\n");
		//exit(1);
	}
	
	//printf("file is %d bytes long\n", (int) st.st_size);
	
	dir = getdirtree(path);
	printf("Number of subdirs %d\n", dir->num_subdirs);	
	return 0;
}
