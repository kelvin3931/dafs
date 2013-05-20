/* Main Function */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <libconfig.h>
#include "curl_cloud.h"

void usage()
{
    printf("Too few arguments!!Error!!\n");
    printf("./conn <UPLOAD_FILENAME>\n");
    printf("eg: ./conn 123.txt\n");
    abort();
}

int main(int argc,char *argv[])
{

    config_t cfg;
    config_setting_t *setting = NULL;
    const char *str1;

    char *url;
    char *upload_filename;

//**
    FILE *fp;
    char *temp ;
    char *token ;
//**

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

#if 0
    printf("%s\n%s\n%s\n%s\n", swift_auth_url, user, pass, dir);
#endif

//**

    if (argc < 2)
    {
        usage();
    }

    url = (char *)swift_auth_url;
    upload_filename = argv[1];

    curl_global_init(CURL_GLOBAL_ALL);

    conn_swift(url);

//** get token from auth_token.txt
    temp = (char *)malloc(MAX);
    fp = fopen("auth_token.txt", "r");
    if (fp)
    {
        printf("OK!!\n");
        while(!feof(fp))
        {
            if(fgets(temp, MAX, fp) != NULL)
            {
                printf("%s",temp);
            }
        }
    }
    else
        printf("Error\n");
    fclose(fp);
    token = strtok(temp, "\n,\r");

    /* write token to config file
    setting = config_setting_add(setting, "X-Auth-Token", CONFIG_TYPE_STRING);
    config_setting_set_string (setting, token);
    config_write_file(&cfg, "token_config.cfg");
    */
//**


    query_container(token);

    create_container(token);

    upload_file(upload_filename, token, "/root/hsm_fuse/src/da_conn/123.txt");

    //download_file(token);

    //delete_file(upload_filename, token);

    //delete_container(token);

    curl_global_cleanup();

    config_destroy(&cfg);

    return 0;
}
