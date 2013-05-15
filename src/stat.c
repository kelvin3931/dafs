#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

int da_getattr(char *path, struct stat *si)
{
	int ret;
	ret = lstat(path, si);
	return ret;
}


