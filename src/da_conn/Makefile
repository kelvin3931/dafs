.PHONY: clean
CC = gcc
#CFLAGS = -lcurl -g3 -Wall -lconfig -lsqlite3 -lfuse -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse \
		 -L/usr/gnu/lib -I../da_sql
CFLAGS = -lcurl -g3 -Wall -lconfig -lsqlite3 -lfuse -D_FILE_OFFSET_BITS=64 \
		 -I/usr/include/fuse/ -L/lib -I../da_sql

HEADER = curl_cloud.h

conn_swift: $(HEADER)
	$(CC) -c conn_swift.c -o conn_swift.o $(CFLAGS)

da_conn: conn_swift main $(HEADER)
	$(CC) main.o conn_swift.o  $(CFLAGS) -o da_conn ../da_sql/da_sqlite.o \
	../da_sql/md5.o ../da_sql/sha256.o

main: $(HEADER)
	$(CC) -c main.c -o main.o $(CFLAGS)

clean:
	rm *.o da_conn
