#!/bin/bash
os=$1

if [ $1 == 'oi' ]; then
    sed -i 's/\/root\/hsm_fuse\/src/\/home\/jerry\/hsm_fuse\/src/g' sqlite_db/sql.h da_conn/curl_cloud.h
fi

if [ $1 == 'ubuntu' ]; then
    sed -i 's/\/home\/jerry\/hsm_fuse\/src/\/root\/hsm_fuse\/src/g' sqlite_db/sql.h da_conn/curl_cloud.h
fi
