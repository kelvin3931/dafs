.PHONY: clean db

da_sqlite.o: da_sqlite.c
	gcc -g -c da_sqlite.c -o da_sqlite.o -lsqlite3 -D_FILE_OFFSET_BITS=64 -lfuse -I/usr/include/fuse

db: da_sqlite.o
	gcc -g da_sqlite.o -o db -lsqlite3 -D_FILE_OFFSET_BITS=64 -lfuse -I/usr/include/fuse

clean:
	rm -f db *.o
