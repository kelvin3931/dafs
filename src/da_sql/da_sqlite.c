#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "../log.h"
#include "sql.h"
#include "../da_conn/curl_cloud.h"
#include "../../../sqlite3/sqlite3.h"


struct tm *a_tm, *m_tm, *c_tm;
char a_datestring[MAX_LEN];
char m_datestring[MAX_LEN];
char c_datestring[MAX_LEN];
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

static char *table = "file_attr";
struct db_col (
  char col_name[MAX_LEN];
  char col_value[MAX_LEN];
);

void show_db_data(struct rec_attr *data)
{
    log_msg("st_dev=%d\n", (int)data->file_attr.st_dev);
    log_msg("st_ino=%d\n", (int)data->file_attr.st_ino);
    log_msg("st_mode=%d\n", (int)data->file_attr.st_mode);
    log_msg("st_nlink=%d\n", (int)data->file_attr.st_nlink);
    log_msg("st_uid=%d\n", data->file_attr.st_uid);
    log_msg("st_gid=%d\n", data->file_attr.st_gid);
    log_msg("st_rdev=%d\n", (int)data->file_attr.st_rdev);
    log_msg("st_size=%d\n", (int)data->file_attr.st_size);
    log_msg("st_atime=%s\n", ctime(&(data->file_attr.st_atime)));
    log_msg("st_mtime=%s\n", ctime(&(data->file_attr.st_mtime)));
    log_msg("st_ctime=%s\n", ctime(&(data->file_attr.st_ctime)));
    log_msg("st_blksize=%d\n", (int)data->file_attr.st_blksize);
    log_msg("st_blocks=%d\n", (int)data->file_attr.st_blocks);
    log_msg("filename=%s\n", data->filename);
    log_msg("parent=%s\n", data->parent);
    log_msg("full_path=%s\n", data->full_path);
    log_msg("cache_path=%s\n", data->cache_path);
    log_msg("cloud_path=%s\n", data->cloud_path);

    return;
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
    return 0;
}

int get_datestring(struct stat *statbuf)
{
    a_tm = localtime(&(statbuf->st_atime));
    strftime(a_datestring, sizeof(a_datestring), "%Y-%m-%d %H:%M:%S", a_tm);
    m_tm = localtime(&(statbuf->st_mtime));
    strftime(m_datestring, sizeof(m_datestring), "%Y-%m-%d %H:%M:%S", m_tm);
    c_tm = localtime(&(statbuf->st_ctime));
    strftime(c_datestring, sizeof(c_datestring), "%Y-%m-%d %H:%M:%S", c_tm);
    return 0;
}

int time_to_str(struct timestruct *time, char *time_str)
{
    struct tm tm_time = localtime(time);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_time);
}

int insert_stat_to_db_value(char *fpath, char *cloud_path,
                           struct stat *statbuf, char *sql_cmd ,char *path)
{
    char *filename, *parent;
  	filename = (char*)malloc(MAX_LEN);
  	parent = (char*)malloc(MAX_LEN);
    path_translate(path, filename, parent);
    lstat(fpath, statbuf);

    get_datestring(statbuf);
    sprintf(sql_cmd, "INSERT INTO file_attr VALUES( %ld, %ld, %ld, %ld, %ld, \
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
    return 0;
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

int update_path(char *sql_cmd, char *path, struct db_col *update_cols, int count)
{
    int i, isOk = false;
    char *updates;
    if ( count > 0 ) {
        updates = malloc(sizeof(char)*MAX_LEN);
        for ( i = 0; i < count; i++) {
            sprintf(updates, "%s, %s='%s'", updates, update_cols[i].col_name, update_cols[i].col_value);
        }
        updates = updates + 2;
        sprintf(sql_cmd, "UPDATE %s SET %s where full_path = '%s';",
                      table, updates, path);
        updates = updates - 2;
        isOk = true;
        free(updates);
    }
    return isOk;
}

int update_cachepath(sqlite3 *db, char *path)
{
	sql_cmd = (char*)malloc(MAX_LEN);
    int count = 1;
    struct db_col cols[count];

    sprintf(cols[0].col_name, "%s", "cache_path");
    sprintf(cols[0].col_value, "%s", "''");
    
    if ( update_path(sql_cmd, path, cols, count) ) {
    //sprintf(sql_cmd, "UPDATE file_attr SET cache_path='' where full_path = '%s';",
    //                  path);
        sqlite3_exec(db, sql_cmd, 0 , 0, &errMsg);
    }
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

    get_datestring(statbuf);

    sprintf(sql_cmd, "UPDATE file_attr SET st_dev=%ld, st_ino=%ld, st_mode=%ld,\
                      st_nlink=%ld, st_uid=%ld, st_gid=%ld, st_rdev=%ld, \
                      st_size=%lld, st_atim='%s', st_mtim='%s', st_ctim='%s', \
                      st_blksize=%ld, st_blocks=%lld, filename='%s', \
                      parent='%s', cache_path='%s', \
                      cloud_path='%s' where full_path = '%s';",
                      statbuf->st_dev, (long)statbuf->st_ino,
                      (unsigned long)statbuf->st_mode, (long)statbuf->st_nlink,
                      (long)statbuf->st_uid, (long)statbuf->st_gid,
                      (long)statbuf->st_rdev, (long long)statbuf->st_size,
                      //ctime(&statbuf->st_atime), ctime(&statbuf->st_mtime),
                      //ctime(&statbuf->st_ctime), (long)statbuf->st_blksize,
                      a_datestring, m_datestring,
                      c_datestring, (long)statbuf->st_blksize,
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
    get_datestring(statbuf);

    sprintf(sql_cmd, "UPDATE file_attr SET st_dev=%ld, st_ino=%ld, st_mode=%ld, \
                      st_nlink=%ld, st_uid=%ld, st_gid=%ld, st_rdev=%ld, \
                      st_size=%lld, st_atim='%s', st_mtim='%s', st_ctim='%s', \
                      st_blksize=%ld, st_blocks=%lld, filename='%s', \
                      parent='%s', full_path = '%s', cache_path = '%s', \
                      cloud_path='%s' where full_path = '%s';",
                      statbuf->st_dev, (long)statbuf->st_ino,
                      (unsigned long)statbuf->st_mode,
                      (long)statbuf->st_nlink,(long)statbuf->st_uid,
                      (long)statbuf->st_gid, (long)statbuf->st_rdev,
                      //(long long)statbuf->st_size, ctime(&statbuf->st_atime),
                      //ctime(&statbuf->st_mtime), ctime(&statbuf->st_ctime),
                      (long long)statbuf->st_size, a_datestring,
                      m_datestring, c_datestring,
                      (long)statbuf->st_blksize, (long long)statbuf->st_blocks,
                      filename, parent, new_path, new_fpath, container_url,
                      path);
	sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    return 0;
}

int assign_cols(struct db_col* cols, struct stat* statbuf)
{
    char datestring[MAX_LEN];
    cols[0].col_name = "st_dev";
    sprintf(cols[0].col_value, "%ld", statbuf->st_dev);
    cols[1].col_name = "st_ino";
    sprintf(cols[1].col_value, "%ld", (long)statbuf->st_ino);
    cols[2].col_name = "st_mode";
    sprintf(cols[2].col_value, "%ld", (unsigned long)statbuf->st_mode);
    cols[3].col_name = "st_nlink";
    sprintf(cols[3].col_value, "%ld", (long)statbuf->st_nlink);
    cols[4].col_name = "st_uid";
    sprintf(cols[4].col_value, "%ld", (long)statbuf->st_uid);
    cols[5].col_name = "st_gid";
    sprintf(cols[5].col_value, "%ld", (long)statbuf->st_gid);
    cols[6].col_name = "st_rdev";
    sprintf(cols[6].col_value, "%ld", (long)statbuf->st_rdev);
    cols[7].col_name = "st_size";
    sprintf(cols[7].col_value, "%lld", (long long)statbuf->st_dev);
    time_to_str(&(statbuf->st_atime), datestring);
    cols[8].col_name = "st_atim";
    sprintf(cols[8].col_value, "%s", datestring);
    time_to_str(&(statbuf->st_mtime), datestring);
    cols[9].col_name = "st_mtim";
    sprintf(cols[9].col_value, "%s", datestring);
    time_to_str(&(statbuf->st_ctime), datestring);
    cols[10].col_name = "st_ctim";
    sprintf(cols[10].col_value, "%s", datestring);
    cols[11].col_name = "st_blksize";
    sprintf(cols[11].col_value, "%ld", (long)statbuf->st_blksize);
    cols[12].col_name = "st_blocks";
    sprintf(cols[12].col_value, "%lld", (long long)statbuf->st_blocks);
    db_cols[13].col_name = "filename";
    db_cols[14].col_name = "parent";
    db_cols[15].col_name = "full_path";
    db_cols[16].col_name = "cache_path";
    db_cols[17].col_name = "cloud_path";
}

int update_rec(sqlite3 *db, char *fpath, struct stat* statbuf, char *path)
{
    int count = 18;
    char *container_url, *filename, *parent;
	sql_cmd = (char*)malloc(MAX_LEN);
    container_url = (char* )malloc(MAX_LEN);
    filename = (char*)malloc(MAX_LEN);
    parent = (char*)malloc(MAX_LEN);
    struct db_col *db_cols = malloc(sizeof(struct db_col)*count);
    
    sprintf(container_url, "%s%s", SWIFT_CONTAINER_URL, path);
    path_translate(path, filename, parent);

    ret = lstat(fpath, statbuf);

    assign_cols(db_cols, statbuf);
    sprintf(db_cols[13].col_value, "%s", filename);
    sprintf(db_cols[14].col_value, "%s", parent);
    sprintf(db_cols[15].col_value, "%s", path);
    sprintf(db_cols[16].col_value, "%s", fpath);
    sprintf(db_cols[17].col_value, "%s", container_url);

    update_path(sql_cmd, path, db_cols, count);
    /*
    update_stat_to_db_value(fpath, container_url, statbuf, sql_cmd, path);
    */
	sqlite3_exec(db, sql_cmd, 0, 0, &errMsg);
    
    free(sql_cmd);
    free(container_url);
    free(filename);
    free(parent);
    free(db_cols);

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
    char tmp[5]="\0";
    int i, i_tmp = 0;
    int part = 0;
    for ( i = 0; i < strlen(datetime); i++) {
        if (datetime[i] != '-' && datetime[i] != ':' && datetime[i] != ' ' ) {
            tmp[i_tmp] = datetime[i];
            i_tmp++;
        } else {
            tmp[i_tmp] = '\0';
            if ( part == 0 )
                time.tm_year = atoi(tmp)-1900;
            else if ( part == 1 )
                time.tm_mon = atoi(tmp)-1;
            else if ( part == 2 )
                time.tm_mday = atoi(tmp);
            else if ( part == 3 )
                time.tm_hour = atoi(tmp);
            else if ( part == 4 )
                time.tm_min = atoi(tmp);
            i_tmp = 0;
            part++;
        }
    }
    time.tm_sec = atoi(tmp);
    time.tm_wday = 0;
    time.tm_yday = 0;
    time.tm_isdst = 0;
    return mktime(&time);
}

int get_db_data(sqlite3 *db, struct rec_attr *data, char *full_path)
{
    int row = 0, column = 0, i=0, j=0;
    char **Result;
	  sql_cmd = (char*)malloc(MAX_LEN);

    sprintf(sql_cmd, "select * from file_attr where full_path='%s';", full_path);
    sqlite3_get_table( db, sql_cmd, &Result, &row, &column, &errMsg );


    data->file_attr.st_dev = atoi(Result[(i+1)*column]);
    data->file_attr.st_ino = atoi(Result[(i+1)*column+1]);
    data->file_attr.st_mode = atoi(Result[(i+1)*column+2]);
    data->file_attr.st_nlink = atoi(Result[(i+1)*column+3]);
    data->file_attr.st_uid = atoi(Result[(i+1)*column+4]);
    data->file_attr.st_gid = atoi(Result[(i+1)*column+5]);
    data->file_attr.st_rdev = atoi(Result[(i+1)*column+6]);
    data->file_attr.st_size = atoi(Result[(i+1)*column+7]);
    data->file_attr.st_atime = str_to_time_t(Result[(i+1)*column+8]);
    data->file_attr.st_mtime = str_to_time_t(Result[(i+1)*column+9]);
    data->file_attr.st_ctime = str_to_time_t(Result[(i+1)*column+10]);
    data->file_attr.st_blksize = atoi(Result[(i+1)*column+11]);
    data->file_attr.st_blocks = atoi(Result[(i+1)*column+12]);
    sprintf(data->filename, "%s", Result[(i+1)*column+13]);
    sprintf(data->parent, "%s", Result[(i+1)*column+14]);
    sprintf(data->full_path, "%s", Result[(i+1)*column+15]);
    sprintf(data->cache_path, "%s", Result[(i+1)*column+16]);
    sprintf(data->cloud_path, "%s", Result[(i+1)*column+17]);
    sqlite3_free_table(Result);

    return 0;
}

int retrieve_common_parent(sqlite3 *db, char *allpath[MAX_LEN], struct rec_attr *data)
{
    int row = 0, column = 0, i=0, j=0;
    char **Result;
	sql_cmd = (char*)malloc(MAX_LEN);

    if ( (strcmp(data->parent, "/") == 0) && (strcmp(data->full_path, "/") == 0)) {
        sprintf(sql_cmd, "select filename from file_attr where parent='%s';", data->parent);
        log_msg("root_parent = %s\n", data->parent);
    }
    else
    {
        sprintf(sql_cmd, "select filename from file_attr where parent='%s/';", data->full_path);
        log_msg("sub_parent = %s\n", data->parent);
    }
    log_msg(sql_cmd);

    sqlite3_get_table( db, sql_cmd, &Result, &row, &column, &errMsg );
    for(i=0; i<row; i++)
    {
        for(j=0; j<column; j++)
        {
            log_msg( "Result[%d][%d] = %s\n", i , j, Result[(i+1)*column+j] );
            allpath[i] = Result[(i+1)*column+j];
            log_msg( "allpath[%d] = %s\n", i , allpath[i] );
        }
    }
    sqlite3_free_table(Result);
    return i;
}

int da_fstat(sqlite3 *db, char *full_path, struct stat *statbuf)
{
    int row = 0, column = 0, i=0, j=0;
    char **Result;
/*
    char filename[MAX_LEN], parent[MAX_LEN];
    path_translate(full_path, filename, parent);
    if ( filename == "." && parent != "/" ) {
        strncpy(full_path, parent, len(parent)-1);
        full_path[len(parent)] = '\0';
    } else if ( filename == ".." && parent != "/" ) {
        strncpy(full_path, parent, len(parent)-1);
        path_translate(full_path, filename, parent);
        strncpy(full_path, parent, len(parent)-1);
        full_path[len(parent)] = '\0';
    }
*/
	sql_cmd = (char*)malloc(MAX_LEN);

    sprintf(sql_cmd, "select * from file_attr where full_path='%s';", full_path);
    sqlite3_get_table( db, sql_cmd, &Result, &row, &column, &errMsg );

    log_msg("%s\n", sql_cmd);
    log_msg("row %d\n", row);
    log_msg("err %s\n", errMsg);
    if ( row == 1 ) {
        statbuf->st_dev = atoi(Result[(i+1)*column]);
        statbuf->st_ino = atoi(Result[(i+1)*column+1]);
        statbuf->st_mode = atoi(Result[(i+1)*column+2]);
        statbuf->st_nlink = atoi(Result[(i+1)*column+3]);
        statbuf->st_uid = atoi(Result[(i+1)*column+4]);
        statbuf->st_gid = atoi(Result[(i+1)*column+5]);
        statbuf->st_rdev = atoi(Result[(i+1)*column+6]);
        statbuf->st_size = atoi(Result[(i+1)*column+7]);
        statbuf->st_atime = str_to_time_t(Result[(i+1)*column+8]);
        statbuf->st_mtime = str_to_time_t(Result[(i+1)*column+9]);
        statbuf->st_ctime = str_to_time_t(Result[(i+1)*column+10]);
        statbuf->st_blksize = atoi(Result[(i+1)*column+11]);
        statbuf->st_blocks = atoi(Result[(i+1)*column+12]);
    }

    log_msg("statbuf %d\n", statbuf->st_blksize);
    sqlite3_free_table(Result);
    if ( row == 1 )
        return 0;
    else
        return -1;
}

struct dirent *da_readdir(sqlite3 *db, char *full_path, char *allpath[], int *result_count)
{
    char filename[MAX_LEN], parent[MAX_LEN];
    struct rec_attr *data=malloc(sizeof(struct rec_attr));
    struct dirent *de;
    de = (struct dirent *)malloc(sizeof(struct dirent));

    path_translate(full_path, filename, parent);
    if ( filename == "." && parent != "/" ) { 
        strncpy(full_path, parent, len(parent)-1);
        full_path[len(parent)] = '\0';
    } else if ( filename == ".." && parent != "/" ) { 
        strncpy(full_path, parent, len(parent)-1);
        path_translate(full_path, filename, parent);
        strncpy(full_path, parent, len(parent)-1);
        full_path[len(parent)] = '\0';
    }

    log_msg("get_db_data %s\n", full_path);
    get_db_data(db, data, full_path);
    log_msg("show_db_data\n");
    show_db_data(data);

    strcpy(de->d_name,data->filename);
    de->d_ino = data->file_attr.st_ino;
    de->d_reclen = 16;
    de->d_off = 1;

    if(S_ISREG(data->file_attr.st_mode))
        log_msg("%s file\n", de->d_name);
    else
        log_msg("%s directory\n", de->d_name);

    *result_count = retrieve_common_parent(db, allpath, data);

    return de;
}
