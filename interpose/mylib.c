#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>

// The following line declares a function pointer with the same prototype as the open function.  
int (*orig_open)(const char *pathname, int flags, ...);  // mode_t mode is needed when flags includes O_CREAT
int (*orig_close)(int fd);
ssize_t (*orig_read)(int fd, void *buf, size_t count);
ssize_t (*orig_write)(int fd, const void *buf, size_t count);
ssize_t (*orig_getdirentries)(int fd, char *buf, size_t nbytes, off_t *basep);

// This is our replacement for the open function from libc.
int open(const char *pathname, int flags, ...) {
	mode_t m=0;
	if (flags & O_CREAT) {
		va_list a;
		va_start(a, flags);
		m = va_arg(a, mode_t);
		va_end(a);
	}
	// we just print a message, then call through to the original open function (from libc)
	fprintf(stderr, "mylib: open called for path %s\n", pathname);
	return orig_open(pathname, flags, m);
}

ssize_t read(int fd, void *buf, size_t count) {
	fprintf(stderr, "mylib: read called for fd %d\n", fd);
	return orig_read(fd, buf, count);
}

int close(int fd) {
	fprintf(stderr, "mylib: close called\n");
	return orig_close(fd);
}

ssize_t write(int fd, const void *buf, size_t count) {
	fprintf(stderr, "mylib: write called for fd %d\n", fd);
	return orig_write(fd, buf, count);
}

ssize_t getdirentries(int fd, char *buf, size_t nbytes, off_t *basep){
	fprintf(stderr, "mylib: getdirentries called for content %s\n", buf);
	return orig_getdirentries(fd, buf, nbytes, basep);
}
// This function is automatically called when program is started
void _init(void) {
	// set function pointer orig_open to point to the original open function
	orig_open = dlsym(RTLD_NEXT, "open");
	orig_close = dlsym(RTLD_NEXT, "close");
	orig_read = dlsym(RTLD_NEXT, "read");
	orig_write = dlsym(RTLD_NEXT, "write");
	//orig_lseek = dlsym(RTLD_NEXT, "lseek");
	//orig_stat = dlsym(RTLD_NEXT, "stat");
	//orig_unlink = dlsym(RTLD_NEXT, "unlink");
	orig_getdirentries = dlsym(RTLD_NEXT, "getdirentries");
	fprintf(stderr, "Init mylib\n");
}


