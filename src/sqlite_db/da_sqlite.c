#include <stdio.h>
#include <sys/stat.h>
#include <sqlite3.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include "sql.h"

void show_file_stat(struct stat *si);
sqlite3 *init_db(sqlite3 *db, char *filename);
int insert_rec(sqlite3 *db, char *fpath, struct stat* si);
int insert_stat_to_db_value(char *fpath, char *cloud_path,
                            struct stat* statbuf, char *sql_cmd);
int remove_rec(sqlite3 *db, char *fpath);
int remove_stat_db_value(char *fpath, char *cloud_path, char *sql_cmd);
int update_rec(sqlite3 *db, char *fpath, struct stat* statbuf);
int update_rec_rename(sqlite3 *db, char *fpath, struct stat* statbuf,
                      char *new_path);
int update_stat_to_db_value(char *fpath, char *cloud_path,
                            struct stat* statbuf, char *sql_cmd);
int get_rec(sqlite3 *db, char *fpath, struct stat* attr);

struct rec_attr {
    struct stat file_attr;
    char *path;
    char *cloud_path;
    int archived;
};
char *errMsg = NULL;
static char *createsql = "CREATE TABLE file_attr("
                         "st_dev INTEGER NOT NULL,"
                         "st_ino INTEGER NOT NULL,"
                         "st_mode INTEGER NOT NULL,"
                         "st_nlink INTEGER NOT NULL,"
                         "st_uid INTEGER NOT NULL,"
                         "st_gid INTEGER NOT NULL,"
                         "st_rdev INTEGER NOT NULL,"
                         "st_size INTEGER NOT NULL,"
                         "st_atim VARCHAR(255),"
                         "st_mtim VARCHAR(255),"
                         "st_ctim VARCHAR(255),"
                         "st_blksize INTEGER NOT NULL,"
                         "st_blocks INTEGER NOT NULL,"
                         //"st_fstype VARCHAR(255),"
                         "path VARCHAR(255) PRIMARY KEY,"
                         "cloud_path VARCHAR(255),"
                         "archived BOOLEAN NOT NULL);";

void show_file_stat(struct stat *si)
{
    printf("DEV:                      %ld\n", (long) si->st_dev);
    printf("RDEV:                     %ld\n", (long) si->st_rdev);
    printf("I-node number:            %ld\n", (long) si->st_ino);
    printf("Mode:                     %lo(oct)\n",
            (unsigned long) si->st_mode);
    printf("Link count:               %ld\n", (long) si->st_nlink);
    printf("Ownership:                UID=%ld   GID=%ld\n",
            (long) si->st_uid, (long) si->st_gid);
    printf("Preferred I/O block size: %ld bytes\n",
            (long) si->st_blksize);
    printf("File size:                %lld bytes\n",
            (long long) si->st_size);
    printf("Blocks allocated:         %lld\n",
            (long long) si->st_blocks);
    printf("Last status change:       %s", ctime(&si->st_ctime));
    printf("Last file access:         %s", ctime(&si->st_atime));
    printf("Last file modification:   %s", ctime(&si->st_mtime));
}

sqlite3 *init_db(sqlite3 *db, char *filename)
{
    char *err_msg = NULL;

    if (filename == NULL)
    {
        fprintf(stderr, "No such filename.\n");
    }
    else
    {
        //verify if filename is valid
        printf("filename is valid.\n");
    }

    if (sqlite3_open_v2( DBPATH, &db, SQLITE_OPEN_READWRITE
                     | SQLITE_OPEN_CREATE, NULL) == SQLITE_OK){
        if ( sqlite3_exec(db, createsql, 0, 0, &err_msg) == SQLITE_OK)
        {
	        fprintf(stderr, "Create table OK.\n");
        }
        else
        {
            fprintf(stderr, "Create table is fail or table is already created.\n");
        }
    }
    else {
        fprintf(stderr, "Open database is fail.\n");
        sqlite3_close(db);
        return 0;
    }

    return db;
}

int insert_stat_to_db_value(char *fpath, char *cloud_path, struct stat* statbuf, char *sql_cmd)
{
    int ret;
    ret = lstat(fpath, statbuf);
    sprintf(sql_cmd, "INSERT INTO file_attr VALUES( %ld, %ld, %lo, %ld, %ld, \
                      %ld, %ld, %lld, '%s', '%s', '%s', %ld, %lld, '%s', '%s',\
                      %d);", (long)statbuf->st_dev, (long)statbuf->st_ino,
                      (unsigned long)statbuf->st_mode, (long)statbuf->st_nlink,
                      (long)statbuf->st_uid, (long)statbuf->st_gid,
                      (long)statbuf->st_rdev, (long long)statbuf->st_size,
                      ctime(&statbuf->st_atime), ctime(&statbuf->st_mtime),
                      ctime(&statbuf->st_ctime), (long)statbuf->st_blksize,
                      (long long)statbuf->st_blocks, fpath, CLOUD_PATH,
                      ARCHIVED);
    return ret;
}

int insert_rec(sqlite3 *db, char *fpath, struct stat* statbuf)
{
    char *sql_cmd;
    char *errMsg = NULL;
	sql_cmd = (char*)malloc(MAX_LEN);
    insert_stat_to_db_value(fpath, NULL, statbuf, sql_cmd);
    sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    return 0;
}

int remove_stat_db_value(char *fpath, char *cloud_path, char *sql_cmd)
{
    sprintf(sql_cmd, "DELETE FROM file_attr where path = '%s';",fpath);
    return 0;
}

int remove_rec(sqlite3 *db, char *fpath)
{
    char *sql_cmd;
    char *errMsg = NULL;
	sql_cmd = (char*)malloc(MAX_LEN);
    sprintf(sql_cmd, "DELETE FROM file_attr where path = '%s';",fpath);
    sqlite3_exec(db, sql_cmd, 0 , 0, &errMsg );
    return 0;
}

int update_stat_to_db_value(char *fpath, char *cloud_path, struct stat* statbuf, char *sql_cmd)
{
    int ret;
    ret = lstat(fpath, statbuf);
    sprintf(sql_cmd, "UPDATE file_attr SET st_dev=%ld, st_mode=%lo, \
                      st_nlink=%ld, st_uid=%ld, st_gid=%ld, st_rdev=%ld, \
                      st_size=%lld, st_atim='%s', st_mtim='%s', st_ctim='%s', \
                      st_blksize=%ld, st_blocks=%lld, cloud_path='%s', \
                      archived=%d where path = '%s';", (long)statbuf->st_dev,
                      (unsigned long)statbuf->st_mode, (long)statbuf->st_nlink,
                      (long)statbuf->st_uid, (long)statbuf->st_gid,
                      (long)statbuf->st_rdev, (long long)statbuf->st_size,
                      ctime(&statbuf->st_atime), ctime(&statbuf->st_mtime),
                      ctime(&statbuf->st_ctime), (long)statbuf->st_blksize,
                      (long long)statbuf->st_blocks, CLOUD_PATH, ARCHIVED,
                      fpath);
    return ret;
}

int update_rec_rename(sqlite3 *db, char *fpath, struct stat* statbuf,
                      char *new_path)
{
    char *sql_cmd;
    char *errMsg = NULL;
	sql_cmd = (char*)malloc(MAX_LEN);
    lstat(new_path, statbuf);
    sprintf(sql_cmd, "UPDATE file_attr SET st_dev=%ld, st_mode=%lo, \
                      st_nlink=%ld, st_uid=%ld, st_gid=%ld, st_rdev=%ld, \
                      st_size=%lld, st_atim='%s', st_mtim='%s', st_ctim='%s', \
                      st_blksize=%ld, st_blocks=%lld, path = '%s', \
                      cloud_path='%s', archived=%d where path = '%s';",
                      (long)statbuf->st_dev,(unsigned long)statbuf->st_mode,
                      (long)statbuf->st_nlink,(long)statbuf->st_uid,
                      (long)statbuf->st_gid, (long)statbuf->st_rdev,
                      (long long)statbuf->st_size, ctime(&statbuf->st_atime),
                      ctime(&statbuf->st_mtime), ctime(&statbuf->st_ctime),
                      (long)statbuf->st_blksize, (long long)statbuf->st_blocks,
                      new_path, CLOUD_PATH, ARCHIVED, fpath);
	sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    return 0;
}

int update_rec(sqlite3 *db, char *fpath, struct stat* statbuf)
{
    char *sql_cmd;
    char *errMsg = NULL;
	sql_cmd = (char*)malloc(MAX_LEN);
    update_stat_to_db_value(fpath, NULL, statbuf, sql_cmd);
	sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    return 0;
}

int get_rec(sqlite3 *db, char *fpath, struct stat* attr)
{
    char *sql_cmd;
	sql_cmd = (char*)malloc(MAX_LEN);
    sprintf(sql_cmd, "SELECT * FROM file_attr;",fpath);
    sqlite3_exec(db, sql_cmd, 0 , 0, &errMsg );
    return 0;
}

#if 0
int main(int argc, char *argv[])
{
    int rc;
    char **result;
    int rows,cols, i , j;
    struct stat* si;
    sqlite3 *db;
	//struct rec_attr si;

	//sql_cmd = (char*)malloc(100);

	si = (struct stat*)malloc(sizeof(struct stat));

    //if (stat(argv[1], si) == -1) {
    if (stat(my_path, si) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    show_file_stat(si);

    //db = init_db(db, argv[1]);

    //db = init_db(db, my_path);

    insert_rec(db, my_path, si);

    //insert_rec(db, argv[1], si);

    //remove_rec(db, argv[1]);

    //update_rec(db, argv[1], si);

    //get_rec(db, argv[1], si);

	//system ("sqlite3 sqlite3.db \"SELECT * FROM file_attr;\"");

    sqlite3_free(errMsg);

    sqlite3_close(db);
    return 0;
}
#endif
