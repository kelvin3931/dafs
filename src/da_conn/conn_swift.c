/** Connect with swift backend
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <libconfig.h>
#include "curl_cloud.h"

#ifdef DEBUG
#define debug(...) printf(__VA_ARGS__)
#endif

#define RETRY_TIMEOUT 10

//** Function Prototype
char *get_config_url();
char *get_token();
static int progress(void *p, double dltotal, double dlnow,
                    double ultotal, double ulnow);
static int test_CURL(CURL *curl, CURLcode res);
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
static size_t write_file(void *ptr, size_t size, size_t nmemb, void *stream);
size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
CURL *create_curl();
int conn_swift(char *url);
int query_container(char *token);
int upload_file(char *file, char *token, char *fpath);
int delete_file(char *file, char *token);
int download_file(char *file, char *token);
int create_container(char *token);
int delete_container(char *token);
//**

//** global variable
long http_response_number ;
char *temp_container_url;
CURL *curl;
CURLcode res;
struct curl_slist *headers;
char storage_token[MAX];
//**

struct myprogress {
    double lastruntime;
    CURL *curl;
};

char *get_config_url()
{
    config_t cfg;
    config_setting_t *setting = NULL;
    const char *str1;

    char *url;
    const char *swift_auth_url, *user, *pass, *dir;
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

    url = (char *)swift_auth_url;
    return url;
}

char *get_token()
{
    FILE *fp;
    char *temp ;
    char *token ;

    temp = (char *)malloc(MAX);

    fp = fopen(TOKEN_PATH, "r");
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

    return token;
}

static int progress(void *p, double dltotal, double dlnow,
                    double ultotal, double ulnow)
{
    struct myprogress *myp = (struct myprogress *)p;
    CURL *curl = myp->curl;
    double curtime = 0;

    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

    /* under certain circumstances it may be desirable for certain functionality
     * to only run every N seconds, in order to do this the transaction time can
     * be used */
    if((curtime - myp->lastruntime) >= 3) {
        myp->lastruntime = curtime;
        fprintf(stderr, "TOTAL TIME: %f \r\n", curtime);
    }

    fprintf(stderr, "UP: %g of %g  DOWN: %g of %g\r\n",
            ulnow, ultotal, dlnow, dltotal);

    if(dlnow > 6000)
        return 1;
    return 0;
}

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t retcode;
    /* in real-world cases, this would probably get this data differently
     * as this fread() stuff is exactly what the library already would do
     * by default internally */
    retcode = fread(ptr, size, nmemb, stream);
    return retcode;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    char *str;
    char *token_content;

    str = (char *)malloc(MAX);
    memcpy(str, ptr, size * nmemb);

    // will point to start of 'X-Auth-Token' header
    token_content = (char *)malloc(MAX);
    token_content = strstr((char*)str,"X-Auth-Token:");
    //** output of the token
    if (token_content != NULL)
    {
        fwrite(ptr, size, nmemb, (FILE *)stream);
    }
    //**

#if 0
    char *header = (char *)alloca(size * nmemb + 1);
    char *head = (char *)alloca(size * nmemb + 1);
    char *value = (char *)alloca(size * nmemb + 1);
    memcpy(header, (char *)ptr, size * nmemb);
    header[size * nmemb] = '\0';
    if (sscanf(header, "%[^:]: %[^\r\n]", head, value) == 2)
    {
        if (!strncasecmp(head, "x-auth-token", size * nmemb))
            strncpy(storage_token, value, sizeof(storage_token));
    }
#endif

    return size * nmemb;
}

static size_t write_file(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

/* test curl perform execute success or fail */
static int test_CURL(CURL *curl, CURLcode res)
{
    clock_t start,end;
    double diff_time;

    if(res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        start = time(0);
        res = curl_easy_perform(curl);
        end = time(0);
        diff_time = end - start;
        if (diff_time < RETRY_TIMEOUT)
            printf("over!%lf\n",diff_time);
    }
    else
        fprintf(stderr, "connect correct!!\n");
    return 0;
}

/* Connect with swift and Get swift AUTH_TOKEN ID. */
int conn_swift(char *url)
{
//** Put body on file
    static const char *fd = TOKEN_PATH;
    FILE *fd_src;
    fd_src = fopen(fd, "w");
//**

    headers = NULL;
    headers = curl_slist_append(headers, "X-Storage-User: test:tester");
    headers = curl_slist_append(headers, "X-Storage-Pass: testing");

    curl = curl_easy_init();
    curl_easy_reset(curl);

    if(curl) {

        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, RETRY_TIMEOUT);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        /* write the page body to this file handle. CURLOPT_FILE is also known
         * as CURLOPT_WRITEDATA*/
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd_src);
        //curl_easy_setopt(curl, CURLOPT_FILE, fd_src);

        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

        curl_easy_setopt(curl, CURLOPT_CONV_FROM_UTF8_FUNCTION, 1L);

        res = curl_easy_perform(curl);

        test_CURL(curl, res);

//** get response number
        //curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_response_number);
        //printf ("HTTP response number:%ld\n",http_response_number);
//**

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    fclose(fd_src);

    return 0;
}

int query_container(char *token)
{
//** URL and File_name string concatenation
    char *container_url;
    temp_container_url = SWIFT_CONTAINER_URL;
    container_url = (char* )malloc(50);
    strcpy(container_url, temp_container_url);
//**

    headers = NULL;
    headers = curl_slist_append(headers, token);

    curl = curl_easy_init();
    curl_easy_reset(curl);

    if(curl) {

        curl_easy_setopt(curl, CURLOPT_URL, container_url);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, RETRY_TIMEOUT);

        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

        res = curl_easy_perform(curl);

        test_CURL(curl, res);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return 0;
}

int upload_file(char *file, char *token, char *fpath)
{
    struct myprogress prog;
    FILE *hd_src;
    struct stat file_info;

    double speed_upload, total_time;
    curl_off_t fsize;

//** URL and File_name string concatenation
    char *container_url;
    char *file_name;
    CURL *curl;
    container_url = (char* )malloc(MAX);
    file_name = (char* )malloc(MAX);
    temp_container_url = SWIFT_CONTAINER_URL;
    strcpy(file_name, file);
    strcpy(container_url, temp_container_url);
    strcat(container_url, file);
//**
    hd_src = fopen(fpath, "r");

    if (hd_src)
        printf("file open success!!\n");
    else
    {
        printf("file open fail!!\n");
        return 1;
    }

    headers = NULL;
    /* build a list of commands to pass to libcurl */
    headers = curl_slist_append(headers, token);

    //fstat(hd_src, &file_info);
    if(fstat(fileno(hd_src), &file_info) != 0)
    {
        return 1; /* can't continue */
    }

    fsize = (curl_off_t)file_info.st_size;
    printf("Local file size: %" CURL_FORMAT_CURL_OFF_T " bytes.\n", fsize);

    curl = curl_easy_init();
    curl_easy_reset(curl);

    if(curl) {

        curl_easy_setopt(curl, CURLOPT_URL, container_url);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* we want to use our own read function */
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

        /* now specify which file to upload */
        curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

        /* HTTP PUT please */
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);

        /* HTTP PUT please */
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        /* provide the size of the upload, we specicially typecast the value
         * to curl_off_t since we must be sure to use the correct data size */
        curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
                        (curl_off_t)file_info.st_size);

        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, RETRY_TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, RETRY_TIMEOUT);

        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
        /* pass the struct pointer into the progress function */
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

        res = curl_easy_perform(curl);

        test_CURL(curl, res);

        //** get response number
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_response_number);
        if (http_response_number == 404)
        {
            printf ("Bucket does not exist.\n");
            assert(NULL);
        }
        //**

        /* now extract transfer info */
        curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &speed_upload);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);

        printf("\nSpeed: %.3f bytes/sec during %.3f seconds\n",
                                      speed_upload, total_time);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    fclose(hd_src);

    return 0;
}

int delete_file(char *file, char *token)
{
//** URL and File_name string concatenation
    char *container_url;
    char *file_name;
    container_url = (char* )malloc(MAX);
    file_name = (char* )malloc(MAX);
    temp_container_url = SWIFT_CONTAINER_URL;
    strcpy(file_name, file);
    strcpy(container_url, temp_container_url);
    strcat(container_url, file_name);
//**

    headers = NULL;
    /* build a list of commands to pass to libcurl */
    headers = curl_slist_append(headers, token);

    curl = curl_easy_init();
    curl_easy_reset(curl);

    if(curl) {

        curl_easy_setopt(curl, CURLOPT_URL, container_url);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, RETRY_TIMEOUT);

        /* HTTP DELETE please */
        curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,"DELETE");

        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

        res = curl_easy_perform(curl);

        test_CURL(curl, res);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return 0;
}

int download_file(char *file, char *token)
{
    struct myprogress prog;
    double speed_download, total_time;

//** URL and File_name string concatenation
    char *container_url;
    char *file_name;
    container_url = (char* )malloc(MAX);
    file_name = (char* )malloc(MAX);
    temp_container_url = SWIFT_CONTAINER_URL;
    strcpy(file_name, file);
    strcpy(container_url, temp_container_url);
    strcat(container_url, file);
//**

    headers = NULL;
    headers = curl_slist_append(headers, token);

    static const char *filename = "/home/jerry/hsm_fuse/src/output";
    FILE *output_file;

    /* open the file */
    output_file = fopen(filename, "wb");

    //temp_container_url = SWIFT_DOWNLOAD_URL;

    curl = curl_easy_init();
    curl_easy_reset(curl);

    if(curl) {

        //curl_easy_setopt(curl, CURLOPT_URL, temp_container_url);
        curl_easy_setopt(curl, CURLOPT_URL, container_url);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, RETRY_TIMEOUT);

        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);

        /* write the page body to this file handle. CURLOPT_FILE is also known
         * as CURLOPT_WRITEDATA*/
        curl_easy_setopt(curl, CURLOPT_FILE, output_file);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress);
        /* pass the struct pointer into the progress function */
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

        res = curl_easy_perform(curl);

        test_CURL(curl, res);

        /* now extract transfer info */
        curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed_download);
        curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total_time);
        printf("\nSpeed: %.3f bytes/sec during %.3f seconds\n", speed_download,
                                                                total_time);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    //fclose(output_file);

    return 0;
}

int create_container(char *token)
{
//** URL and File_name string concatenation
    char *container_url;
    temp_container_url = SWIFT_NEW_CONTAINER;
    container_url = (char* )malloc(50);
    strcpy(container_url, temp_container_url);
//**

    headers = NULL;
    /* build a list of commands to pass to libcurl */
    headers = curl_slist_append(headers, token);

    curl = curl_easy_init();
    curl_easy_reset(curl);

    if(curl) {

        curl_easy_setopt(curl, CURLOPT_URL, container_url);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, RETRY_TIMEOUT);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* HTTP PUT please */
        curl_easy_setopt(curl, CURLOPT_PUT, 1L);

        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

        res = curl_easy_perform(curl);

        test_CURL(curl, res);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return 0;
}

int delete_container(char *token)
{
//** URL and File_name string concatenation
    char *container_url;
    container_url = (char* )malloc(50);
    temp_container_url = SWIFT_NEW_CONTAINER;
    strcpy(container_url, temp_container_url);
//**

    headers = NULL;
    /* build a list of commands to pass to libcurl */
    headers = curl_slist_append(headers, token);

    curl = curl_easy_init();
    curl_easy_reset(curl);

    if(curl) {

        curl_easy_setopt(curl, CURLOPT_URL, container_url);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, RETRY_TIMEOUT);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        /* HTTP DELETE please */
        curl_easy_setopt(curl,CURLOPT_CUSTOMREQUEST,"DELETE");

        curl_easy_setopt(curl, CURLOPT_HEADER, 1L);

        res = curl_easy_perform(curl);

        test_CURL(curl, res);

        /* always cleanup */
        curl_easy_cleanup(curl);
    }

    return 0;
}
