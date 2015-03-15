#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

#include <sys/io.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "trace.h"
#define MY_TRACE_PREFIX "fast_gpio_common"

#define UIO_DIR "/sys/class/uio/"
#define UIO_MAX_NAME_LEN 256

int fastGpioFindUioByName(const char *name)
{
	const struct dirent *ent;
	int uio_num;
	size_t name_len;

	FILE *nameFile;
	DIR *dp;
	char thisname[UIO_MAX_NAME_LEN];
	char *filename;
	char *cr;

	dp = opendir(UIO_DIR);
	if (dp == NULL) {
		trace_error("No uio devices available\n");
		return -ENODEV;
	}

	while (ent = readdir(dp), ent != NULL) {
		if (strncmp(ent->d_name, "uio", sizeof("uio") - 1) == 0) {
			trace_debug("Found UIO entry at %s%s\n",
				    UIO_DIR, ent->d_name);
			sscanf(ent->d_name, "uio%d", &uio_num);
			filename = malloc(sizeof(UIO_DIR)
					  + strlen(ent->d_name)
					  + 5);
			if (filename == NULL) {
				closedir(dp);
				return -ENOMEM;
			}
			sprintf(filename, "%s%s/name",
				UIO_DIR,
				ent->d_name);

			nameFile = fopen(filename, "r");
			if (!nameFile) {
				free(filename);
				continue;
			}
			free(filename);

			memset(thisname, 0, sizeof(thisname));
			fread(thisname, UIO_MAX_NAME_LEN, 1, nameFile);
			if (!thisname) {
				fclose(nameFile);
				continue;
			}
			fclose(nameFile);

			/* thisname might contain a carriage return, remove if so */
			cr = strpbrk(thisname, "\r\n");
			if (cr)
				*cr = '\0';

			if (strcmp(name, thisname) == 0) {
				trace_debug("Match found for UIO device '%s'\n",
					    name);
				closedir(dp);
				return uio_num;
			}
		}
	}
	trace_error("No match found for UIO device '%s'\n", name);
	closedir(dp);
	return -ENODEV;
}


int fastGpioGetInfo(const int uio_num,
		    const int index,
		    char *path_fmt)
{
	int ret = 0;
	FILE *fp;
	char *temp = malloc(strlen(path_fmt) + 8);
	if (temp == NULL)
		return -ENOMEM;

	sprintf(temp, path_fmt, uio_num, index);
	fp = fopen(temp, "r");
	if (fp == NULL) {
		trace_error("failed to open %s\n", temp);
		ret = -errno;
		goto error_free;
	}
	fscanf(fp, "%i", &ret);

    error_free:
	if (NULL != fp)
		fclose(fp);
	free(temp);

	return ret;
}
