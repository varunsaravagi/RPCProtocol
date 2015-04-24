#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

int main(int argc, char**argv) {
	int fd;

	if (argc<2) {
		printf("need to specify file\n");
		exit(1);
	}
	
	fd = open(argv[1], O_RDONLY);
	if (fd<0) {
		printf("Couldn't open %s\n", argv[1]);
		exit(1);
	}
	
	close(fd);
	close(fd);
	return 0;
}
