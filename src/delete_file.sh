#!/bin/bash -x

#swift -A https://192.168.88.14:8080/auth/v1.0 -U test:tester -K testing delete --all;
#swift -A https://192.168.88.14:8080/auth/v1.0 -U test:tester -K testing post abc;
swift -A https://192.168.111.106:8080/auth/v1.0 -U test:tester -K testing delete --all;
swift -A https://192.168.111.106:8080/auth/v1.0 -U test:tester -K testing post abc;

/usr/bin/sqlite3 sql.db "delete from file_attr"
/usr/bin/sqlite3 sql.db "delete from time_rec"

rm -rf ../example/rootdir/*
