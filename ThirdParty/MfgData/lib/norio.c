/* Raw NOR I/O in C
 * RDowling 6/23/2009
 * Not very pretty
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <mtd/mtd-user.h>
#include <string.h>

#define PAGE_SIZE 4096
#define FILENAME_LEN	16
#define PROC_MTD_LINE_LEN	80
#define MFG_DATA_FMT	"MfgData%d"

// Find and return absolute path to /dev/mtdX corresponding to MfgDataN
// Return 0 for success and fill buf[]
// Return 1 for failure
int get_nor_data_filename (char *buf, int partition)
{
	char needle[FILENAME_LEN];
	char line[PROC_MTD_LINE_LEN];
	FILE *fin = fopen ("/proc/mtd", "r");
	if (!fin)
	{
		perror ("get_nor_data_filename filed opening /proc/mtd");
		return 1;
	}
	sprintf (needle, MFG_DATA_FMT, partition & 1);
	while (!feof(fin))
	{
		fgets (line, PAGE_SIZE, fin);
		if (strstr (line, needle))
		{
			// Found it, now pull out mtd
			char c = line[3];
			sprintf (buf, "/dev/mtd%c", c);
			fclose (fin);
			return 0;
		}
	}
	fclose (fin);
	fprintf (stderr, "get_nor_data_filename: Could not find %s in /proc/mtd\n", needle);
	return 1;
}

// Return 0 for success and fill buf[]
// Return 1 for failure
int read_mfg_sector (unsigned char *buf, int partition)
{
	char path[FILENAME_LEN];
	int fd, len;
	if (get_nor_data_filename (path, partition))
	{
		fprintf (stderr, "read_mfg_sector: get_nor_data_filename failed\n");
		return 1;
	}
	fprintf (stderr, "read_mfg_sector: reading from %s\n", path);
	fd = open (path, O_RDONLY);
	if (!fd)
	{
		perror ("read_mfg_sector");
		return 1;
	}

	// Read
	len = read (fd, buf, PAGE_SIZE);
	if (len < PAGE_SIZE)
	{
		close (fd);
		fprintf (stderr, "read_mfg_sector: only could read %d bytes of %d\n", len, PAGE_SIZE);
		return 1;
	}
	close (fd);
	return 0;
}

// Return 0 for success and fill buf[]
// Return 1 for failure
int write_mfg_sector (unsigned char *buf, int partition)
{
	struct erase_info_user erase;
	char path[FILENAME_LEN];
	int fd, len;

	// Open file
	if (get_nor_data_filename (path, partition))
	{
		fprintf (stderr, "write_mfg_sector: get_nor_data_filename failed\n");
		return 1;
	}
	fprintf (stderr, "read_mfg_sector: reading from %s\n", path);
	fd = open (path, O_WRONLY);
	if (!fd)
	{
		perror ("write_mfg_sector");
		return 1;
	}
	
	// Erase
	erase.start =0;
	erase.length = PAGE_SIZE;
	if (ioctl (fd, MEMERASE, &erase) < 0)
	{
		close (fd);
		perror ("write_mfg_sector");
		return 1;
	}

	// Write
	len = write (fd, buf, PAGE_SIZE);
	if (len < PAGE_SIZE)
	{
		close (fd);
		fprintf (stderr, "write_mfg_sector: only could write %d bytes of %d\n", len, PAGE_SIZE);
		return 1;
	}
	close (fd);
	return 0;
}

#ifdef TEST
main ()
{
	unsigned char iobuf[PAGE_SIZE];
	// Simple test: copy sector 0 to sector 1
	read_mfg_sector (iobuf, 0);
	write_mfg_sector (iobuf, 1);
}
#endif
