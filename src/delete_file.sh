#!/bin/bash -x

swift -A https://192.168.88.14:8080/auth/v1.0 -U test:tester -K testing delete abc $1;
