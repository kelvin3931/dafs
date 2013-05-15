bbfs : bbfs.o log.o stat.o
	gcc -g3 -Wall -o bbfs bbfs.o log.o stat.o -L/root/libfuse/amd64 -lfuse

bbfs.o : bbfs.c log.h params.h
	gcc -g -Wall -I/root/libfuse/include -D_FILE_OFFSET_BITS=64 -c bbfs.c

log.o : log.c log.h params.h
	gcc -g -Wall -I/root/libfuse/include -D_FILE_OFFSET_BITS=64 -c log.c

stat.o: stat.c
	gcc -g -Wall -D_FILE_OFFSET_BITS=64 -c stat.c

clean:
	rm -f bbfs *.o

dist:
	rm -rf fuse-tutorial/
	mkdir fuse-tutorial/
	cp ../*.html fuse-tutorial/
	mkdir fuse-tutorial/example/
	mkdir fuse-tutorial/example/mountdir/
	mkdir fuse-tutorial/example/rootdir/
	echo "a bogus file" > fuse-tutorial/example/rootdir/bogus.txt
	mkdir fuse-tutorial/src
	cp Makefile COPYING COPYING.LIB *.c *.h fuse-tutorial/src/
	tar cvzf ../../fuse-tutorial.tgz fuse-tutorial/
	rm -rf fuse-tutorial/
