.PHONY: clean db

CC = gcc

da_sqlite.o: da_sqlite.c md5.o
	$(CC) -g -c da_sqlite.c -o da_sqlite.o -lsqlite3 -D_FILE_OFFSET_BITS=64 \
	-lfuse -I/usr/gnu/include/fuse -I../da_conn -L/usr/gnu/lib md5.o

db: da_sqlite.o
	$(CC) -g da_sqlite.o -o db -lsqlite3 -D_FILE_OFFSET_BITS=64 -lfuse -I/usr/include/fuse

md5.o:
	$(CC) -c md5.c -o md5.o

clean:
	rm -f db *.o
