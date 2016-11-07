#!/bin/bash -x

mkdir -p ../example/rootdir ../example/mountdir;
fusermount -u ../example/mountdir;
make clean; make oi;
./bbfs ../example/rootdir ../example/mountdir;
swift -A https://192.168.111.106:8080/auth/v1.0 -U test:tester -K testing list abc;
#swift -A https://192.168.88.14:8080/auth/v1.0 -U test:tester -K testing list abc;
