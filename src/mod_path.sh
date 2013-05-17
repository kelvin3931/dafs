#!/bin/bash -x
os=$1

usage()
{
    echo "Modify path. Please Use 'oi' or 'ubuntu'."
}

if [[ $os == 'oi' ]]; then
    sed -i 's/\/root\/hsm_fuse\/src/\/home\/jerry\/hsm_fuse\/src/g' sqlite_db/sql.h da_conn/curl_cloud.h
elif [[ $os == 'ubuntu' ]]; then
    sed -i 's/\/home\/jerry\/hsm_fuse\/src/\/root\/hsm_fuse\/src/g' sqlite_db/sql.h da_conn/curl_cloud.h
else
    usage
fi
