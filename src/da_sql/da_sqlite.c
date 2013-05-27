#include <stdio.h>
#include <sys/stat.h>
#include "../../../sqlite3/sqlite3.h"
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../log.h"
#include "sql.h"
#include "../da_conn/curl_cloud.h"
#include <langinfo.h>

void show_file_stat(struct stat *si);
sqlite3 *init_db(sqlite3 *db);
int insert_rec(sqlite3 *db, char *fpath, struct stat* si, char *path);
int insert_stat_to_db_value(char *fpath, char *cloud_path,
                            struct stat* statbuf, char *sql_cmd, char *path);
int remove_rec(sqlite3 *db, char *path);
int update_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path);
int update_rec_rename(sqlite3 *db, char *fpath, struct stat* statbuf,
                      char *new_fpath, char* path, char *new_path);
int update_stat_to_db_value(char *fpath, char *cloud_path,
                            struct stat* statbuf, char *sql_cmd, char *path);
int get_rec(sqlite3 *db, char *fpath, struct stat* attr);
struct dirent *da_readdir(sqlite3 *db, char *full_path);

struct rec_attr {
    struct stat file_attr;
    char *filename;
    char *parent;
    char *full_path;
    char *cache_path;
    char *cloud_path;
};

struct rec_attr *db_data;
char *sql_cmd;
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
                         //"st_atim VARCHAR(255),"
                         //"st_mtim VARCHAR(255),"
                         //"st_ctim VARCHAR(255),"
                         "st_atim DATETIME,"
                         "st_mtim DATETIME,"
                         "st_ctim DATETIME,"
                         "st_blksize INTEGER NOT NULL,"
                         "st_blocks INTEGER NOT NULL,"
                         //"st_fstype VARCHAR(255),"
                         "filename VARCHAR(255) ,"
                         "parent VARCHAR(255) ,"
                         "full_path VARCHAR(255) PRIMARY KEY,"
                         "cache_path VARCHAR(255) ,"
                         "cloud_path VARCHAR(255));";

int time_t_to_str(struct timestruc_t *time, char *datestr)
{
    struct tm     *tm;
    tm = localtime(time);
    strftime(datestr, sizeof(datestr), "%Y-%m-%d %H-%M-%S", tm);
}

void show_db_data(struct rec_attr *data)
{
    char tm_atime[256], tm_mtime[256], tm_ctime[256];

    time_t_to_str(&(((struct rec_attr *)data)->file_attr.st_atime), tm_atime);
    time_t_to_str(&(((struct rec_attr *)data)->file_attr.st_mtime), tm_mtime);
    time_t_to_str(&(((struct rec_attr *)data)->file_attr.st_ctime), tm_ctime);

    printf ("st_dev=%d\n", ((struct rec_attr *)data)->file_attr.st_dev);
    printf ("st_ino=%d\n", ((struct rec_attr *)data)->file_attr.st_ino);
    printf ("st_mode=%d\n", ((struct rec_attr *)data)->file_attr.st_mode);
    printf ("st_nlink=%d\n", ((struct rec_attr *)data)->file_attr.st_nlink);
    printf ("st_uid=%d\n", ((struct rec_attr *)data)->file_attr.st_uid);
    printf ("st_gid=%d\n", ((struct rec_attr *)data)->file_attr.st_gid);
    printf ("st_rdev=%d\n", ((struct rec_attr *)data)->file_attr.st_rdev);
    printf ("st_size=%d\n", ((struct rec_attr *)data)->file_attr.st_size);
    printf ("st_atime=%s\n", tm_atime);
    printf ("st_mtime=%s\n", tm_mtime);
    printf ("st_ctime=%s\n", tm_ctime);
    printf ("st_atime=%s\n", ctime(&(((struct rec_attr *)data)->file_attr.st_atime)));
    printf ("st_mtime=%s\n", ctime(&(((struct rec_attr *)data)->file_attr.st_mtime)));
    printf ("st_ctime=%s\n", ctime(&(((struct rec_attr *)data)->file_attr.st_ctime)));
    printf ("st_blksize=%d\n", ((struct rec_attr *)data)->file_attr.st_blksize);
    printf ("st_blocks=%d\n", ((struct rec_attr *)data)->file_attr.st_blocks);
    printf ("filename=%s\n", ((struct rec_attr *)data)->filename);
    printf ("parent=%s\n", ((struct rec_attr *)data)->parent);
    printf ("full_path=%s\n", ((struct rec_attr *)data)->full_path);
    printf ("cache_path=%s\n", ((struct rec_attr *)data)->cache_path);
    printf ("cloud_path=%s\n", ((struct rec_attr *)data)->cloud_path);
}

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

sqlite3 *init_db(sqlite3 *db)
{
    if (sqlite3_open_v2( DBPATH, &db, SQLITE_OPEN_READWRITE
                     | SQLITE_OPEN_CREATE, NULL) == SQLITE_OK){
        if ( sqlite3_exec(db, createsql, 0, 0, &errMsg) == SQLITE_OK)
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

int path_translate(char *fullpath, char* filename, char* parent)
{
    int fullpath_len, filename_len, parent_len;
    strcpy(filename, strrchr(fullpath, '/') + 1);

    fullpath_len = strlen(fullpath);
    filename_len = strlen(filename);
    parent_len = fullpath_len - filename_len;

    strncpy(parent, fullpath, parent_len);
    parent[parent_len] = '\0';
    //log_msg("Translating: parent %s, file %s, full %s\n",parent, filename, fullpath );
    return 0;
}

int insert_stat_to_db_value(char *fpath, char *cloud_path,
                           struct stat* statbuf, char *sql_cmd ,char *path)
{
    int ret;
    char *filename, *parent;
	filename = (char*)malloc(MAX_LEN);
	parent = (char*)malloc(MAX_LEN);
    //log_msg("parent %s, file %s, full %s\n",&parent, &filename, path );
    path_translate(path, filename, parent);
    //log_msg("parent %s, file %s, full %s\n",&parent, &filename, path );
    ret = lstat(fpath, statbuf);

    struct tm *a_tm, *m_tm, *c_tm;
    char a_datestring[256];
    char m_datestring[256];
    char c_datestring[256];
    a_tm = localtime(&(statbuf->st_atime));
    //strftime(a_datestring, sizeof(a_datestring), nl_langinfo(D_T_FMT), a_tm);
    strftime(a_datestring, sizeof(a_datestring), "%Y-%m-%d %H-%M-%S", a_tm);
    m_tm = localtime(&(statbuf->st_mtime));
    strftime(m_datestring, sizeof(m_datestring), "%Y-%m-%d %H-%M-%S", m_tm);
    c_tm = localtime(&(statbuf->st_ctime));
    strftime(c_datestring, sizeof(c_datestring), "%Y-%m-%d %H-%M-%S", c_tm);

    sprintf(sql_cmd, "INSERT INTO file_attr VALUES( %ld, %ld, %lo, %ld, %ld, \
                      %ld, %ld, %lld, '%s', '%s', '%s', %ld, %lld, '%s', '%s',\
                      '%s', '%s', '%s' );", (long)statbuf->st_dev,
                      (long)statbuf->st_ino,
                      (unsigned long)statbuf->st_mode, (long)statbuf->st_nlink,
                      (long)statbuf->st_uid, (long)statbuf->st_gid,
                      (long)statbuf->st_rdev, (long long)statbuf->st_size,
                      //ctime(&statbuf->st_atime), ctime(&statbuf->st_mtime),
                      //ctime(&statbuf->st_ctime), (long)statbuf->st_blksize,
                      a_datestring, m_datestring,
                      c_datestring, (long)statbuf->st_blksize,
                      (long long)statbuf->st_blocks, filename, parent, path, fpath, cloud_path);
    return ret;
}

int insert_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path)
{
    char *container_url;
	sql_cmd = (char*)malloc(MAX_LEN);
    container_url = (char* )malloc(MAX_LEN);
    sprintf(container_url, "%s%s", SWIFT_CONTAINER_URL, path);
    insert_stat_to_db_value(fpath, container_url, statbuf, sql_cmd, path);
    sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    return 0;
}

int remove_rec(sqlite3 *db, char *path)
{
	sql_cmd = (char*)malloc(MAX_LEN);
    sprintf(sql_cmd, "DELETE FROM file_attr where full_path = '%s';",path);
    sqlite3_exec(db, sql_cmd, 0 , 0, &errMsg );
    return 0;
}

int update_stat_to_db_value(char *fpath, char *cloud_path, struct stat* statbuf,
                            char *sql_cmd, char *path)
{
    int ret;

    char *filename, *parent;
	filename = (char*)malloc(MAX_LEN);
	parent = (char*)malloc(MAX_LEN);
    path_translate(path, filename, parent);

    ret = lstat(fpath, statbuf);
    sprintf(sql_cmd, "UPDATE file_attr SET st_dev=%ld, st_ino=%ld, st_mode=%lo,\
                      st_nlink=%ld, st_uid=%ld, st_gid=%ld, st_rdev=%ld, \
                      st_size=%lld, st_atim='%s', st_mtim='%s', st_ctim='%s', \
                      st_blksize=%ld, st_blocks=%lld, filename='%s', \
                      parent='%s', cache_path='%s', \
                      cloud_path='%s' where full_path = '%s';",
                      (long)statbuf->st_dev, (long)statbuf->st_ino,
                      (unsigned long)statbuf->st_mode, (long)statbuf->st_nlink,
                      (long)statbuf->st_uid, (long)statbuf->st_gid,
                      (long)statbuf->st_rdev, (long long)statbuf->st_size,
                      //ctime(&statbuf->st_atime), ctime(&statbuf->st_mtime),
                      //ctime(&statbuf->st_ctime), (long)statbuf->st_blksize,
                      localtime(&statbuf->st_atime), localtime(&statbuf->st_mtime),
                      localtime(&statbuf->st_ctime), (long)statbuf->st_blksize,
                      (long long)statbuf->st_blocks, filename, parent,
                      fpath, cloud_path, path);
    return ret;
}

int update_rec_rename(sqlite3 *db, char *fpath, struct stat* statbuf,
                      char *new_fpath, char *path, char *new_path)
{
    char *container_url;
	char *filename, *parent;
	filename = (char*)malloc(MAX_LEN);
	parent = (char*)malloc(MAX_LEN);
	sql_cmd = (char*)malloc(MAX_LEN);
    container_url = (char* )malloc(MAX_LEN);
    sprintf(container_url, "%s%s", SWIFT_CONTAINER_URL, path);

    path_translate(new_path, filename, parent);

    lstat(new_fpath, statbuf);
    sprintf(sql_cmd, "UPDATE file_attr SET st_dev=%ld, st_ino=%ld, st_mode=%lo, \
                      st_nlink=%ld, st_uid=%ld, st_gid=%ld, st_rdev=%ld, \
                      st_size=%lld, st_atim='%s', st_mtim='%s', st_ctim='%s', \
                      st_blksize=%ld, st_blocks=%lld, filename='%s', \
                      parent='%s', full_path = '%s', cache_path = '%s', \
                      cloud_path='%s' where full_path = '%s';",
                      (long)statbuf->st_dev, (long)statbuf->st_ino,
                      (unsigned long)statbuf->st_mode,
                      (long)statbuf->st_nlink,(long)statbuf->st_uid,
                      (long)statbuf->st_gid, (long)statbuf->st_rdev,
                      //(long long)statbuf->st_size, ctime(&statbuf->st_atime),
                      //ctime(&statbuf->st_mtime), ctime(&statbuf->st_ctime),
                      (long long)statbuf->st_size, localtime(&statbuf->st_atime),
                      localtime(&statbuf->st_mtime), localtime(&statbuf->st_ctime),
                      (long)statbuf->st_blksize, (long long)statbuf->st_blocks,
                      filename, parent, new_path, new_fpath, container_url,
                      path);
	sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    return 0;
}

int update_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path)
{
    char *container_url;
	sql_cmd = (char*)malloc(MAX_LEN);
    container_url = (char* )malloc(MAX_LEN);
    sprintf(container_url, "%s%s", SWIFT_CONTAINER_URL, path);
    update_stat_to_db_value(fpath, container_url, statbuf, sql_cmd, path);
	sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    return 0;
}

int get_rec(sqlite3 *db, char *fpath, struct stat* attr)
{
	sql_cmd = (char*)malloc(MAX_LEN);
    sprintf(sql_cmd, "SELECT * FROM file_attr;");
    sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    return 0;
}

time_t str_to_time_t(char *datetime)
{
    //  0   1   2  3  4 5
    // 2013-01-02 11:12:56
    struct tm time;
    char tmp[5];
    int i, i_tmp = 0;
    int part = 0;
    for ( i = 0; i < strlen(datetime); i++) {
        if (datetime[i] != '-' || datetime[i] != ':' || datetime[i] != ' ') {
            tmp[i_tmp] = datetime[i];
            i_tmp++;
        } else {
            tmp[i_tmp] = '\0';
            if ( part == 0 )
                time.tm_year = atoi(tmp)-1900;
            else if ( part == 1 )
                time.tm_mon = atoi(tmp);
            else if ( part == 2 )
                time.tm_mday = atoi(tmp);
            else if ( part == 3 )
                time.tm_hour = atoi(tmp);
            else if ( part == 4 )
                time.tm_min = atoi(tmp);
            else if ( part == 5 )
                time.tm_sec = atoi(tmp);
            i_tmp = 0;
            part++;
        }
    }
    time.tm_wday = 0;
    time.tm_yday = 0;
    time.tm_isdst = 0;
    return mktime(&time);
}

static int callback(void *data, int col_count, char **col_value, char **col_name)
{
    int i;
    //db_data = (struct rec_attr *)malloc(sizeof(struct rec_attr));
    //for(i=0; i<col_count; i++){
    //    printf("%s = %s\n", col_name[i], col_value[i] ? col_value[i] : "NULL");
    //}
    //printf("\n");

    ((struct rec_attr *)data)->file_attr.st_dev = atoi(col_value[0]);
    ((struct rec_attr *)data)->file_attr.st_ino = atoi(col_value[1]);
    ((struct rec_attr *)data)->file_attr.st_mode = atoi(col_value[2]);
    ((struct rec_attr *)data)->file_attr.st_nlink = atoi(col_value[3]);
    ((struct rec_attr *)data)->file_attr.st_uid = atoi(col_value[4]);
    ((struct rec_attr *)data)->file_attr.st_gid = atoi(col_value[5]);
    ((struct rec_attr *)data)->file_attr.st_rdev = atoi(col_value[6]);
    ((struct rec_attr *)data)->file_attr.st_size = atoi(col_value[7]);
    ((struct rec_attr *)data)->file_attr.st_atime = str_to_time_t(col_value[8]);
    ((struct rec_attr *)data)->file_attr.st_mtime = str_to_time_t(col_value[9]);
    ((struct rec_attr *)data)->file_attr.st_ctime = str_to_time_t(col_value[10]);
    ((struct rec_attr *)data)->file_attr.st_blksize = atoi(col_value[11]);
    ((struct rec_attr *)data)->file_attr.st_blocks = atoi(col_value[12]);
    ((struct rec_attr *)data)->filename = col_value[13];
    ((struct rec_attr *)data)->parent = col_value[14];
    ((struct rec_attr *)data)->full_path = col_value[15];
    ((struct rec_attr *)data)->cache_path = col_value[16];
    ((struct rec_attr *)data)->cloud_path = col_value[17];
    show_db_data(data);

    return 0;
}

struct dirent *da_readdir(sqlite3 *db, char *full_path)
{
    int row = 0, column = 0, i, j, nCol, rc;
    struct rec_attr *data=malloc(sizeof(struct rec_attr));
    char **Result;
    sqlite3_stmt *stmt;
    struct dirent *de;
    de = (struct dirent *)malloc(sizeof(struct dirent));
	sql_cmd = (char*)malloc(MAX_LEN);

    sprintf(sql_cmd, "select * from file_attr where full_path='%s';", full_path);
    rc = sqlite3_exec(db, sql_cmd, callback, data, &errMsg);
    show_db_data(data);

    /*sqlite3_get_table( db, sql_cmd, &Result, &row, &column, &errMsg );
    for(i=0; i<(row+1); i++)
    {
        for(j=0; j<column; j++)
        {
            //log_msg("Result[%d] = %s\n", i, Result[i][j]);
            printf( "Result[%d] = %s\n", j , Result[j] );
        }
    }
    sqlite3_free_table(Result);
    */
    return de;
}
#if 1
int main(int argc, char *argv[])
{
    sqlite3 *db;
    db_data = (struct rec_attr *)malloc(sizeof(struct rec_attr));
/*
    struct stat* si;
	si = (struct stat*)malloc(sizeof(struct stat));
    if (stat(argv[1], si) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    show_file_stat(si);
*/
    db = init_db(db);

    da_readdir(db, "/hello");

    //insert_rec(db, my_path, si);

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
