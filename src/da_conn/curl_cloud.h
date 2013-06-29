#ifndef _CURL_H_
#define _CURL_H_

#include <curl/curl.h>
#define MAX 80
#define CONFIG_PATH "/home/jerry/hsm_fuse/src/da_conn/config.cfg"
#define TOKEN_PATH "/home/jerry/hsm_fuse/src/da_conn/auth_token.txt"

//#define SWIFT_CONTAINER_URL "https://192.168.88.14:8080/v1/AUTH_test/abc"
//#define SWIFT_NEW_CONTAINER "https://192.168.88.14:8080/v1/AUTH_test/abc"
//#define SWIFT_DOWNLOAD_URL "https://192.168.88.14:8080/v1/AUTH_test/abc/conn_swift.c"
#define SWIFT_CONTAINER_URL "https://192.168.111.94:8080/v1/AUTH_test/abc"
#define SWIFT_NEW_CONTAINER "https://192.168.111.94:8080/v1/AUTH_test/abc"
//#define SWIFT_DOWNLOAD_URL "https://192.168.111.94:8080/v1/AUTH_test/abc/conn_swift.c"


char *get_config_url();
char *get_token();
int conn_swift(char *url);
int query_container(char *token);
int upload_file(char *file, char *token, char *fpath, char *container_url);
int delete_file(char *file, char *token);
int download_file(char *file, char *token, char *fpath);
int create_container(char *token);
int delete_container(char *token);
size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);

#endif
