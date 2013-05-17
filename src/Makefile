.PHONY: clean SUBDIRS

CC = gcc
SUBDIRS = sqlite_db da_conn
CFLAGS = -g3 -Wall -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -Lsqlite_db
LIBS = -lcurl -lconfig -lsqlite3 -lfuse
all : subdirs bbfs

oi : $(shell ./mod_path.sh oi) subdirs bbfs

ubuntu : $(shell ./mod_path.sh ubuntu) subdirs bbfs

subdirs :
	for i in $(SUBDIRS); do \
		(cd $$i && make); \
	done

bbfs : bbfs.o log.o stat.o sqlite_db/da_sqlite.o da_conn/conn_swift.o
	$(CC) -o bbfs bbfs.o log.o stat.o sqlite_db/da_sqlite.o \
		     da_conn/conn_swift.o $(CFLAGS) $(LIBS)

bbfs.o : bbfs.c log.h params.h
	$(CC) -c bbfs.c $(CFLAGS) $(LIBS)

log.o : log.c log.h params.h
	$(CC) -c log.c $(CFLAGS) $(LIBS)

stat.o: stat.c
	$(CC) -c stat.c $(CFLAGS) $(LIBS)

clean:
	rm -f bbfs *.o bbfs.log
	for i in $(SUBDIRS); do \
		(cd $$i && make clean); \
	done

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
