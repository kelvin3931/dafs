#ifndef _SQL_H_
#define _SQL_H_

#include "../../../sqlite3/sqlite3.h"
#define MAX_LEN 1024
#define DBPATH "/home/jerry/hsm_fuse/src/sql.db"

struct rec_attr {
    struct stat file_attr;
    char filename[MAX_LEN];
    char parent[MAX_LEN];
    char full_path[MAX_LEN];
    char cache_path[MAX_LEN];
    char cloud_path[MAX_LEN];
};

void show_file_stat(struct stat *si);
sqlite3 *init_db(sqlite3 *db);
int path_translate(char *fullpath, char* filename, char* parent);
int insert_stat_to_db_value(char *fpath, char *cloud_path,
                           struct stat *statbuf, char *sql_cmd ,char *path);
int insert_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path);
int update_cloudpath(sqlite3 *db, char *path, char *cloudpath);
int update_cachepath(sqlite3 *db, char *path, char *cachepath);
int remove_rec(sqlite3 *db, char *path);
int update_stat_to_db_value(char *fpath, char *cloud_path,
                            struct stat* statbuf, char *sql_cmd, char *path);
int update_rec_rename(sqlite3 *db, char *fpath, struct stat* statbuf,
                      char *fnewpath, char *path, char *newpath);
int update_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path);
int get_db_data(sqlite3 *db, struct rec_attr *data, char *full_path);
int retrieve_common_parent(sqlite3 *db, char *allpath[MAX_LEN],
                           struct rec_attr *data);
int da_fstat(sqlite3 *db, char *full_path, struct stat *statbuf);
struct dirent *da_readdir(sqlite3 *db, char *full_path, char *allpath[],
                           int *result_count);
int update_db(sqlite3 *, char *, char *, char *);
int get_record(sqlite3 *db, char *full_path, char *query_field, char *record);
int get_state(sqlite3 *db, char *full_path);
#endif
