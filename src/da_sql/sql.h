#ifndef _SQL_H_
#define _SQL_H_

#define MAX_LEN 1024
//#define CLOUD_PATH "https://192.168.88.14:8080/abc/"
//#define ARCHIVED 0
#define DBPATH "/home/jerry/hsm_fuse/src/sql.db"

#define FULL_PATH "/fullpath"
#define FILENAME "123.txt"
#define PARENT "/parent"

void show_file_stat(struct stat *si);
sqlite3 *init_db(sqlite3 *db);
int insert_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path);
int insert_db(char *fpath, struct stat* statbuf);
int remove_rec(sqlite3 *db, char *path);
int update_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path);
int update_rec_rename(sqlite3 *db, char *fpath, struct stat* statbuf,
                      char *fnewpath, char *path, char *newpath);
int get_rec(sqlite3 *db, char *fpath, struct stat* statbuf);
struct dirent *da_readdir(sqlite3 *db, char *full_path, char *allpath[], int *result_count);
int da_fstat(sqlite3 *db, char *full_path, struct stat *statbuf);
#endif
