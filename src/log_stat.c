#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <time.h>
#include <langinfo.h>

#include "log.h"

int get_file_stat(char *path, struct stat *si)
{
	int ret;
	struct tm     *tm;
    	char          datestring[256];
	ret = lstat(path, si);

	log_msg(" DEV = %lld(%d) \n", si->st_dev, &(si->st_dev));
        log_msg(" PAD1[3](%d) \n", &(si->st_pad1));
        log_msg(" INODE = %lld(%d) \n", si->st_ino, &(si->st_ino));
        log_msg(" MODE = 0%o(%d) \n", si->st_mode, &(si->st_mode));
        log_msg(" LINK = %d(%d) \n", si->st_nlink , &(si->st_nlink));
        log_msg(" UID = %d(%d) \n", si->st_uid, &(si->st_uid));
        log_msg(" GID = %d(%d) \n", si->st_gid, &(si->st_gid));
        log_msg(" RDEV = %lld(%d) \n", si->st_rdev, &(si->st_rdev));
        log_msg(" PAD2[2](%d) \n", &(si->st_pad2));
        log_msg(" SIZE = %9jd(%d) \n", (intmax_t)si->st_size, &(si->st_size));
        //log_msg(" PAD3(%d) \n", &(si->st_pad3));
        tm = localtime(&(si->st_atime));
        strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
        log_msg(" %s(%d)\n", datestring, &(si->st_atime));
        tm = localtime(&(si->st_ctime));
        strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
        log_msg(" %s(%d)\n", datestring, &(si->st_ctime));
        tm = localtime(&(si->st_mtime));
        strftime(datestring, sizeof(datestring), nl_langinfo(D_T_FMT), tm);
        log_msg(" %s(%d)\n", datestring, &(si->st_mtime));
        log_msg(" BLKSIZE = %d(%d) \n", si->st_blksize, &(si->st_blksize));
        log_msg(" BLOCKS = %d(%d) \n", si->st_blocks, &(si->st_blocks));
        log_msg(" FSTYPE(%d) \n", &(si->st_fstype));
        log_msg(" PAD4[8](%d) \n", &(si->st_pad4));

	log_msg(" -----------------------------%d \n", si);
	log_stat_new(si);
	log_msg(" Memory address = %d \n", si);

	return ret;
}


