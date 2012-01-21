#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <linux/input.h>

#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

//----------------------------------------------------------------------------
// find and open an input device by name
//----------------------------------------------------------------------------

// Fill char dev[20] and return 0 or -1 for failure.
int find_input_device(const char *input_name, char *dev)
{
	struct dirent *dp;
	char name[32+1];
	DIR *dir;
	int fd, i;

	dir = opendir("/dev/input/");
	if (!dir)
		return -1;

	while ((dp = readdir(dir)) != NULL) {
		if (dp->d_name && !strncmp(dp->d_name, "event", 5)) {
			sprintf(dev, "/dev/input/%s", dp->d_name);
			fd = open(dev, O_RDONLY);
			if (fd < 0)
				continue;
			int i=ioctl(fd, EVIOCGNAME(32), name);
			close (fd);
			if (i < 0)
				continue;
			if (strstr(name, input_name)) {
				closedir(dir);
				return 0;
			}
		}
	}
	closedir(dir);
	return -1;
}

int open_input_device(const char *input_name)
{
	char dev[20];
	if (find_input_device (input_name, dev) < 0)
		return -1;
	return open(dev, O_RDONLY);
}

LF_END_BRIO_NAMESPACE()

