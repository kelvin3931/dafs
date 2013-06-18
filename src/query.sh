#!/bin/bash -x

/usr/bin/sqlite3 sql.db ".dump";
#/usr/bin/sqlite3 sql.db "select * from file_attr";
#/usr/bin/sqlite3 sql.db "select * from time_rec";
swift -A https://192.168.88.14:8080/auth/v1.0 -U test:tester -K testing list abc;
