#ifndef _CURL_H_
#define _CURL_H_

#include <curl/curl.h>
#define MAX 80
#define CONFIG_PATH "/home/jerry/hsm_fuse/src/da_conn/config.cfg"
#define TOKEN_PATH "/home/jerry/hsm_fuse/src/da_conn/auth_token.txt"

char *get_config_url();
char *get_token();
int conn_swift(char *url);
int query_container(char *token);
int upload_file(char *file, char *token);
int delete_file(char *file, char *token);
int download_file(char *token);
int create_container(char *token);
int delete_container(char *token);

#endif
