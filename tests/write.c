#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>

int main(int argc, char**argv) {
	int fd, fd1, rv;
	char buf[1000000];
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
	
	fd = open( argv[1], O_RDONLY );
	fd1 = open( argv[2], O_RDWR);
	if (fd<0) {
		printf("Couldn't open %s\n", argv[1]);
		exit(1);
	}
	if (fd1<0) {
		printf("Couldn't open %s\n", argv[1]);
		exit(1);
	}

	for(i=0; i<3;i++){
		rv = read( fd, buf, 1000000 );
		if (rv>0) {
			buf[rv]=0;
			//printf("Read %d bytes \n %s\n", rv, buf);
			write(fd1, buf, 1000000);		
		}
		
	}
	
	close(fd);
	close(fd1);
/*
	//Negative fd to Read
	fd = open( argv[1], O_RDONLY );
	if (fd<0) {
		printf("Couldn't open %s\n", argv[1]);
		exit(1);
	}
	
	fd = fd+2;
	rv = read( fd, buf, 10 );
	
	if (rv>0) {
		buf[rv]=0;
		printf ("Read %d bytes: %s\n", rv, buf);
	}
	
	close(fd);

*/
	return 0;
}

