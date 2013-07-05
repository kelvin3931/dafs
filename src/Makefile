.PHONY: clean SUBDIRS

CC = gcc
SUBDIRS = da_sql da_conn
CFLAGS = -g3 -Wall -D_FILE_OFFSET_BITS=64 -I/usr/gnu/include/fuse -Ida_sql \
		 -Ida_conn -Lda_sql
LIBS = -lcurl -lconfig -lsqlite3 -lfuse -L/usr/gnu/lib

SH_FILE = mod_path.sh
OI = oi
UBUNTU = ubuntu

all : subdirs bbfs

oi :
	./$(SH_FILE) $(OI) && make all

ubuntu :
	./$(SH_FILE) $(UBUNTU) && make all

subdirs :
	for i in $(SUBDIRS); do \
		(cd $$i && make); \
	done

bbfs : bbfs.o log.o da_sql/da_sqlite.o da_conn/conn_swift.o
	$(CC) -o bbfs bbfs.o log.o da_sql/da_sqlite.o da_sql/md5.o \
		     da_conn/conn_swift.o $(CFLAGS) $(LIBS)
	cd da_conn; make da_conn

bbfs.o : bbfs.c log.h params.h
	$(CC) -c bbfs.c $(CFLAGS) $(LIBS)

log.o : log.c log.h params.h
	$(CC) -c log.c $(CFLAGS) $(LIBS)

#stat.o: stat.c
#	$(CC) -c stat.c $(CFLAGS) $(LIBS)

clean:
	rm -f bbfs *.o bbfs.log
	for i in $(SUBDIRS); do \
		(cd $$i && make clean); \
	done

allclean: clean
	sudo rm -rf ../example/rootdir/*
	rm sql.db

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
