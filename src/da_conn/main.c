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
    return ;
}

int archive_upload(char *upload_filename, char *token)
{
    char *cloudpath;
    char *fpath;
    char *file;
    fpath = (char* )malloc(MAX);
    file = (char* )malloc(MAX);
    size = MAX;
    ptr = malloc(sizeof(char)*size);
    getcwd(ptr, size);

    sprintf(fpath, "%s/%s", ptr, upload_filename);
    sprintf(file, "/%s", upload_filename);
    cloudpath = malloc(sizeof(char)*MAX);
    upload_file(file, token, fpath, cloudpath);
    update_cachepath(db, file, cloudpath);
    remove(fpath);
    return 0;
}

int archive_download(char *download_filename, char *token)
{
    char *fpath;
    char *file;
    fpath = (char* )malloc(MAX);
    file = (char* )malloc(MAX);
    size = MAX;
    ptr = malloc(sizeof(char)*size);
    getcwd(ptr, size);

    sprintf(fpath, "%s/%s", ptr, download_filename);
    sprintf(file, "/%s", download_filename);

    download_file(file, token, fpath);
    update_cloudpath(db, file, fpath);
    delete_file(file, token);
    return 0;
}

int main(int argc,char *argv[])
{

    config_t cfg;
    config_setting_t *setting = NULL;
    const char *str1;

    char *url;
    char *filename;
    //char full_path;
    //char fpath[MAX] = {}, file[MAX] = {};
//**
    FILE *fp;
    char *temp ;
    char *token ;
//**
#if 0
//** Read swift.cfg file
    const char *swift_auth_url, *user, *pass, *dir;
    char *config_file_name = "config.cfg";
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

    printf("%s\n%s\n%s\n%s\n", swift_auth_url, user, pass, dir);
#endif

//**

    if (argc < 3)
    {
        usage();
        exit(1);
        return 0;
    }

    //url = (char *)swift_auth_url;
    url = get_config_url();
    filename = argv[2];

    db = init_db(db);

    curl_global_init(CURL_GLOBAL_ALL);

    conn_swift(url);

//** get token from auth_token.txt
    token = get_token();
//**

    //query_container(token);

    //create_container(token);

    if (strcmp(argv[1], "up") == 0)
        archive_upload(filename, token);
    else if (strcmp(argv[1], "down") == 0)
        archive_download(filename, token);

    //download_file(token);

    //delete_file(upload_filename, token);

    //delete_container(token);

    curl_global_cleanup();

    //config_destroy(&cfg);

    return 0;
}
