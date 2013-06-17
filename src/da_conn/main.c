/* Main Function */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <libconfig.h>
#include <unistd.h>
#include "curl_cloud.h"
#include <sys/stat.h>
#include <sqlite3.h>
#include "../da_sql/sql.h"

static sqlite3 *db;
char *ptr;
int size;

extern double speed_upload;
extern double total_time;
extern double transfer_time;
extern int filesize;

void usage()
{
    printf("Too few arguments!!Error!!\n");
    printf("./da_conn <upload/download/query> <filename>\n");
    printf("eg: ./da_conn -u<upload> 123.txt\n");
    printf("    ./da_conn -g<download> 123.txt\n");
    printf("    ./da_conn -q<query> /123.txt\n");
    return ;
}

int archive_upload(char *fullpath, char *token)
{
    char *cloudpath;

    char *record_cache;
    record_cache = (char*)malloc(MAX_LEN);
    get_record(db, fullpath, "cache_path", record_cache);

    cloudpath = malloc(sizeof(char)*MAX);

    upload_file(fullpath, token, record_cache, cloudpath);
    up_time_rec(db, transfer_time, filesize, fullpath, 1);
    update_cloudpath(db, fullpath, cloudpath);
    remove(record_cache);
    return 0;
}

int archive_download(char *fullpath, char *token, char *rootdir)
{
    char *down_file, *cache_path;
    down_file = (char* )malloc(MAX);
    cache_path = (char *)malloc(MAX);
    sprintf(down_file, "%s", fullpath);
    sprintf(cache_path, "%s%s", rootdir, fullpath);

    download_file(down_file, token, cache_path);

    //get_record(db, fullpath, "st_size", filesize);
    up_time_rec(db, transfer_time, filesize, fullpath, 0);

    update_cachepath(db, down_file, cache_path);
    //delete_file(down_file, token);

    return 0;
}

int main(int argc,char *argv[])
{
    char *url;
    char *filename;
    char *token ;
    char *ab_path;

    ab_path = (char* )malloc(MAX);
    filename = (char* )malloc(MAX);

#if 1
//** Read swift.cfg file
    config_t cfg;
    config_setting_t *setting = NULL;
    const char *str1;
    const char *swift_auth_url, *user, *pass, *dir, *mountdir, *rootdir;
    char *config_file_name = "/home/jerry/hsm_fuse/src/da_conn/config.cfg";
    /*Initialization */
    config_init(&cfg);

    config_read_file(&cfg, config_file_name);

    config_lookup_string(&cfg, "filename", &str1);

    /*Read the parameter group*/
    setting = config_lookup(&cfg, "swift_params");

    config_setting_length(setting);

    config_setting_lookup_string(setting, "swift-auth-url", &swift_auth_url);
    config_setting_lookup_string(setting, "user", &user);
    config_setting_lookup_string(setting, "passwd", &pass);
    config_setting_lookup_string(setting, "swift-dir", &dir);
    config_setting_lookup_string(setting, "mountdir", &mountdir);
    config_setting_lookup_string(setting, "rootdir", &rootdir);

//**
#endif

    if (argc < 2)
    {
        usage();
        exit(1);
        return 0;
    }

    //url = (char *)swift_auth_url;
    url = get_config_url();

//** get absolute path
    size = MAX;
    ptr = malloc(sizeof(char)*size);
    getcwd(ptr, size);

    if (argc > 2)
    {
        filename = argv[2];

        if ( filename[0] == '/' ) {
            sprintf(ab_path, "%s", filename);
        } else {
            sprintf(ab_path, "%s/%s", ptr, filename);
        }
        realpath(ab_path, ab_path);

//**

        //** get full_path
        int mount_len;
        mount_len = strlen(mountdir);
        strcpy (filename, ab_path + mount_len);
        //**
    }

    db = init_db(db);

    curl_global_init(CURL_GLOBAL_ALL);

    conn_swift(url);

//** get token from auth_token.txt
    token = get_token();
//**

    int opt;
    opt = getopt( argc, argv, "ugq" );
    while( opt != -1 ) {
        switch( opt ) {
            case 'u':
                archive_upload(filename, token);
                break;
            case 'g':
                archive_download(filename, token, (char *)rootdir);
                break;
            case 'q':
                get_state(db, filename);
                break;
            case ':':
                printf("Error\n");
                break;
            default:
                break;
        }
        opt = getopt( argc, argv, "ugq" );
    }

    //query_container(token);

    //create_container(token);

    //download_file(token);

    //delete_file(upload_filename, token);

    //delete_container(token);

    curl_global_cleanup();

    config_destroy(&cfg);

    return 0;
}
