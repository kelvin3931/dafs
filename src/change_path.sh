#!/bin/bash -x
old_user=$1
new_user=$2

sed -i "s/\/home\/$old_user\/hsm_fuse/\/home\/$new_user\/hsm_fuse/g" \
        da_conn/config.cfg da_conn/curl_cloud.h da_sql/sql.h
