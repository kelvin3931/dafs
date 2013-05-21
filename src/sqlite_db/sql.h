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
sqlite3 *init_db(sqlite3 *db, char *filename);
int insert_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path);
int insert_db(char *fpath, struct stat* statbuf);
int remove_rec(sqlite3 *db, char *fpath);
int update_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path);
int update_rec_rename(sqlite3 *db, char *fpath, struct stat* statbuf,
                      char *new_path, char *path);
int get_rec(sqlite3 *db, char *fpath, struct stat* statbuf);

#endif
