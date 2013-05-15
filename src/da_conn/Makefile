.PHONY: clean
CC = gcc
CFLAGS = -lcurl -g3 -Wall -lconfig -lsqlite3

HEADER = curl_cloud.h

da_conn: conn_swift main
	$(CC) main.o conn_swift.o $(CFLAGS) -o da_conn

conn_swift: $(HEADER)
	$(CC) -c conn_swift.c -o conn_swift.o $(CFLAGS)

main: $(HEADER)
	$(CC) -c main.c -o main.o $(CFLAGS)

clean:
	rm *.o da_conn
