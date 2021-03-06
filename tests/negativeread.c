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
	int i;
	if (argc<2){ 
		printf("need to specify file\n");
		exit(1);
}
	//if ( stat(argv[1], &st) < 0 ) {
	//	printf("Couldn't stat %s\n", argv[1]);
	//	exit(1);
	//}
	//printf("file is %d bytes long\n", (int) st.st_size);
	
	//Normal Read operation
/*	
	fd = open( argv[1], O_RDONLY );
	if (fd<0) {
	//	printf("Couldn't open %s\n", argv[1]);
	//	exit(1);
	}
	
	rv = read(fd, buf, 10);
	if (rv < 0 ){
		printf("Fd %d, Read %d\n", fd, rv);
		close(fd);	
	}
	
	fd = open( argv[2], O_RDONLY);
	rv = read(fd+2, buf, 10);
	if (rv < 0 ){
		printf("Fd %d, Read %d\n", fd, rv);
		close(fd);	
	}
	close(fd);
*/

	//Negative fd to Read
	fd = open( argv[1], O_RDONLY );
	if (fd<0) {
		printf("Couldn't open %s\n", argv[1]);
		exit(1);
	}
	
	fd = -1;
	rv = read( fd, buf, 10 );
	
	if (rv>0) {
		buf[rv]=0;
		printf ("Read %d bytes: %s\n", rv, buf);
	}
	
	close(fd);


	return 0;
}

