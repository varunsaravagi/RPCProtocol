#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

int main(int argc, char**argv) {
	int fd, rv;
	char buf[100];
	struct stat st;
	off_t offset;
	ssize_t bytes;
	if (argc<2) printf("need to specify file\n");
	
	//if ( stat(argv[1], &st) < 0 ) {
	//	printf("Couldn't stat %s\n", argv[1]);
	//	exit(1);
	//}
	//printf("file is %d bytes long\n", (int) st.st_size);
	
	//lseek read write operation
	//SEEK_SET and SEEK_CUR

	fd = open( argv[1], O_RDWR );
	if (fd<0) {
		printf("Couldn't open %s\n", argv[1]);
		exit(1);
	}
	
	offset = lseek(fd, 10, SEEK_SET);
	printf("\n---FROM TEST---\nOffset set %d bytes\n---END---\n", offset);
	rv = read(fd, buf, 10);
	
	if (rv>0) {
		buf[rv]=0;
		printf ("---FROM TEST---\nRead %d bytes: %s\n---END---\n", rv, buf);
	}
	
	char* content = "This is from lseek write\n";
	bytes = write(fd, content, 15);
	printf("---FROM TEST---\nWritten %d bytes\n---END---\n", bytes);

	offset = lseek(fd, 10, SEEK_CUR);
	printf("\n---FROM TEST---\nOffset set %d bytes\n---END---\n", offset);
	rv = read(fd, buf, 10);
	
	if (rv>0) {
		buf[rv]=0;
		printf ("---FROM TEST---\nRead %d bytes: %s\n---END---\n", rv, buf);
	}
	
	close(fd);

	return 0;
}

