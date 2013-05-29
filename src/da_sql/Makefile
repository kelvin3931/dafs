.PHONY: clean db

da_sqlite.o: da_sqlite.c
	gcc -g -c da_sqlite.c -o da_sqlite.o -D_FILE_OFFSET_BITS=64 /home/jerry/sqlite3/sqlite3.o

db: da_sqlite.o
	gcc -g da_sqlite.o -o db -D_FILE_OFFSET_BITS=64 /home/jerry/sqlite3/sqlite3.o

clean:
	rm -f db *.o
