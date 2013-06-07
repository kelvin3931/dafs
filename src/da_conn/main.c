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

void usage()
{
    printf("Too few arguments!!Error!!\n");
    printf("./conn <up/down> <filename>\n");
    printf("eg: ./conn up 123.txt\n");
    printf("    ./conn down 123.txt\n");
    printf("    ./conn query /123.txt\n");
    return ;
}

int archive_upload(char *upload_filename, char *token, char *ab_path)
{
    char *cloudpath;

    char *record_cache;
    record_cache = (char*)malloc(MAX_LEN);
    get_record(db, upload_filename, "cache_path", record_cache);

    cloudpath = malloc(sizeof(char)*MAX);
    upload_file(upload_filename, token, ab_path, cloudpath);
    update_cloudpath(db, upload_filename, cloudpath);
    remove(ab_path);
    return 0;
}

int archive_download(char *download_filename, char *token, char *ab_path)
{
    download_file(download_filename, token, ab_path);
    //update_cachepath(db, download_filename, ab_path);
    //delete_file(download_filename, token);
    return 0;
}

int main(int argc,char *argv[])
{
    char *url;
    char *filename;
    char *token ;
    char *ab_path, *check_file;

    ab_path = (char* )malloc(MAX);
    check_file = (char* )malloc(MAX);
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

    printf("%s\n%s\n%s\n%s\n", swift_auth_url, user, pass, dir);
//**
#endif

    if (argc < 3)
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
    sprintf(check_file, "/%s", argv[2]);
    strcpy(filename, strrchr(check_file, '/'));
    sprintf(ab_path, "%s%s", ptr, filename);
    printf(" filename:%s\n", filename);
//**

    db = init_db(db);

    //curl_global_init(CURL_GLOBAL_ALL);

    conn_swift(url);

//** get token from auth_token.txt
    token = get_token();
//**

    //query_container(token);

    //create_container(token);

    if (strcmp(argv[1], "up") == 0)
        archive_upload(filename, token, ab_path);
    else if (strcmp(argv[1], "down") == 0)
        archive_download(filename, token, ab_path);
    else if (strcmp(argv[1], "query") == 0)
    {
        if (strncmp(mountdir, ab_path, strlen(mountdir)) == 0)
        {
            //sprintf(query_file, "/%s", filename);
            get_state(db, filename);
        }
        else
            exit(1);
    }

    //download_file(token);

    //delete_file(upload_filename, token);

    //delete_container(token);

    //curl_global_cleanup();

    //config_destroy(&cfg);

    return 0;
}
