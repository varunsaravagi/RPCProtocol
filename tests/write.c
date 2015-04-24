#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

int main(int argc, char**argv) {
	int fd; ssize_t rv;
	char *buf;
	struct stat st;

	if (argc<2) {
		printf("need to specify file\n");
		exit(1);
	}

	if (argc < 3) {
		printf("need to specify string to write\n");
		exit(1);
	}
	
	if (xstat(1, argv[1], &st) < 0 ) {
		printf("Couldn't stat %s\n", argv[1]);
		exit(1);
	}
	printf("file is %d bytes long\n", (int) st.st_size);
	
	printf("Test 1: Write to read only\n");
	fd = open(argv[1], O_RDONLY);
	if (fd<0) {
		//printf("Couldn't open %s\n", argv[1]);
		//exit(1);
	}
	
	//rv = read( fd, buf, 10 );
	//if (rv>0) {
	//	buf[rv]=0;
	//	printf ("Read %d bytes: %s\n", rv, buf);
	//}
	buf = argv[2];
	
	rv = write(fd, buf, 20);
	//rv = -1;
	if (rv>0) {
		printf("Written %d bytes \n", rv);
	}

	close(fd);

	return 0;
}
