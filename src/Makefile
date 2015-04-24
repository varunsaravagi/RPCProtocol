all: mylib.so server

#pack.o: pack.c
#	gcc -Wall -fPIC -DPIC -c pack.c 

#libpack.so: pack.o
#	ld -shared -o ../lib/libpack.so pack.o -ldl

mylib.o: mylib.c
	gcc -g -Wall -fPIC -DPIC -c mylib.c -I../include/ -L../lib/ #-ldirtree

pack.o: pack.c
	gcc -g -Wall -fPIC -DPIC -c pack.c -I../include/

unpack.o: unpack.c
	gcc -g -Wall -fPIC -DPIC -c unpack.c -I../include/ -L../lib/ -ldirtree

mylib.so: mylib.o pack.o unpack.o
	ld -shared -o mylib.so mylib.o pack.o unpack.o -ldl

server: server.c unpack.c pack.c
	gcc -g -o server server.c pack.c unpack.c -I../include/ -L../lib/ -ldirtree

clean:
	rm -f *.o *.so

