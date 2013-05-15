#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sqlite3.h>

int da_getattr(char *path, struct stat *si)
{
	int ret;
	ret = lstat(path, si);
	return ret;
}
