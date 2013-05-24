.PHONY: clean db

da_sqlite.o:
	gcc -c da_sqlite.c -o da_sqlite.o -lsqlite3 -D_FILE_OFFSET_BITS=64

db: da_sqlite.o
	gcc da_sqlite.o -o db -lsqlite3 -D_FILE_OFFSET_BITS=64

clean:
	rm -f db *.o
