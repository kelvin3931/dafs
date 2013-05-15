.PHONY: clean db

da_sqlite.o:
	gcc -c da_sqlite.c -o da_sqlite.o -lsqlite3

db: da_sqlite.o
	gcc da_sqlite.o -o db -lsqlite3

clean:
	rm -f db *.o
