#!/bin/bash -x

mkdir ../example; mkdir ../example/rootdir; mkdir ../example/mountdir;
sudo /usr/gnu/bin/fusermount -u ../example/mountdir;
make clean; make oi;
sudo ./bbfs ../example/rootdir ../example/mountdir;
swift -A https://192.168.88.14:8080/auth/v1.0 -U test:tester -K testing list abc;
