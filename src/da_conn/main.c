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
#include "sql.h"

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
    printf("eg: ./da_conn -u <upload> 123.txt\n");
    printf("    ./da_conn -g <download> 123.txt\n");
    printf("    ./da_conn -q <query> /123.txt\n");
    return ;
}

int archive_upload(char *fullpath)
{
    char *cloudpath;
    char *record_cache;
    record_cache = (char*)malloc(MAX_LEN);
    get_record(db, fullpath, "cache_path", record_cache);

    cloudpath = malloc(sizeof(char)*MAX);

    upload_file(fullpath, record_cache, cloudpath);
    up_time_rec(db, transfer_time, filesize, fullpath, 1);

    update_cloudpath(db, fullpath, cloudpath);
    remove(record_cache);
    return 0;
}

int archive_download(char *fullpath, char *rootdir)
{
    char *down_file, *cache_path, *record_fsize;
    down_file = (char* )malloc(MAX);
    cache_path = (char *)malloc(MAX);
    record_fsize = (char *)malloc(MAX);
    sprintf(down_file, "%s", fullpath);
    sprintf(cache_path, "%s%s", rootdir, fullpath);

    download_file(down_file, cache_path);

    get_record(db, fullpath, "st_size", record_fsize);
    filesize = atoi(record_fsize);
    up_time_rec(db, transfer_time, filesize, fullpath, 0);

    update_cachepath(db, down_file, cache_path);

    return 0;
}

int archive_query(sqlite3 *db,char *filename)
{
    int state;
    state = get_state(db, filename);
    if (state == -1)
        printf("No File.\n");
    else if (state == 0)
        printf("Local File.\n");
    else if (state == 1)
        printf("Archived File.\n");
    else if (state == 2)
        printf("Cached File.\n");
    return 0;
}

int archive_query_all(sqlite3 *db,char *parent)
{
    char path_of_all[MAX_LEN][MAX_LEN];
    char **getpath;
    int result_num, i;

    getpath = (char **)malloc(MAX_LEN * MAX_LEN * sizeof(char*));
    result_num = get_all_state(db, parent, getpath);

    for(i=0;i < result_num ; i++)
    {
        sprintf(path_of_all[i], "%s", getpath[i]);
    }

    i = (strcmp(path_of_all[0], "/") == 0) ? 3:0;

    /*printf("                           FileName  | File State             ");
    printf("\n       -----------------------------------------------------\n");
    */
    printf("File State        Filename\
            \n--------------    ----------------\n");
    for(;i<result_num;i++)
    {
        if (get_state(db, path_of_all[i]) == 0)
        {
            //printf("%35s  | Local File\n", strrchr(path_of_all[i], '/') + 1);
            printf("Local File        %s\n", strrchr(path_of_all[i], '/') + 1);
        }
        if (get_state(db, path_of_all[i]) == 1)
        {
            //printf("%35s  | Archived File\n", strrchr(path_of_all[i], '/') + 1);
            printf("Archived File     %s\n", strrchr(path_of_all[i], '/') + 1);
        }
        if (get_state(db, path_of_all[i]) == 2)
        {
            //printf("%35s  | Cached File\n", strrchr(path_of_all[i], '/') + 1);
            printf("Cached File       %s\n", strrchr(path_of_all[i], '/') + 1);
        }
    }
    return 0;
}

int archive_remove(sqlite3 *db, char *fullpath)
{
    delete_file(fullpath);
    return 0;
}

int main(int argc,char *argv[])
{
    char *url;
    char *filename;
    char *parent;
    //char *token ;
    char *ab_path;

    ab_path = (char* )malloc(MAX);
    filename = (char* )malloc(MAX);
    parent = (char* )malloc(MAX);

#if 1
//** Read swift.cfg file
    config_t cfg;
    config_setting_t *setting = NULL;
    const char *str1;
    const char *swift_auth_url, *user, *pass, *dir, *mountdir, *rootdir;
    char *config_file_name = CONFIG_PATH;
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

    url = get_config_url();

//** get absolute path
    int mount_len;
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

        if ((strcmp(filename, ".") != 0))
            realpath(ab_path, ab_path);

//**

        //** get full_path
        mount_len = strlen(mountdir);
        strcpy (filename, ab_path + mount_len);
        //**
    }

    if (argc >= 2)
    {
        mount_len = strlen(mountdir);
        sprintf (parent, "%s/", ptr + mount_len);
    }

    db = init_db(db);

    curl_global_init(CURL_GLOBAL_ALL);

    conn_swift(url);

//** get token from auth_token.txt
    //token = get_token();
//**

    int opt;
    char *opt_string;
    opt_string = (char *)malloc(sizeof(char) * MAX);
    opt_string = "ugqlr";
    opt = getopt( argc, argv, opt_string );
    while( opt != -1 ) {
        switch( opt ) {
            case 'u':
                archive_upload(filename);
                break;
            case 'g':
                archive_download(filename, (char *)rootdir);
                break;
            case 'q':
                archive_query(db, filename);
                break;
            case 'l':
                archive_query_all(db, parent);
                break;
            case 'r':
                archive_remove(db, filename);
                break;
            case ':':
                fprintf(stderr,"Error\n");
                break;
            default:
                usage();
                break;
        }
        opt = getopt( argc, argv, opt_string );
    }

    //query_container(token);

    //create_container(token);

    //delete_container(token);

    //delete_file(upload_filename);

    curl_global_cleanup();

    config_destroy(&cfg);

    return 0;
}
