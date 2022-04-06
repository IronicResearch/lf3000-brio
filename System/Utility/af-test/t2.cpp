#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

#include "AtomicFile.h"

// Try an atomic and non-atomic file write, side by side in infinite loop.
// Interrupt with ^C or power off, and check results

#define PRE   "/LF/Bulk/"
// #define PRE   "/tmp/"
#define FILE1 PRE "safe-file"
#define FILE2 PRE "risky-file"

void readint (FILE *f, int *pX)
{
	if (f)
	{
		int res = fscanf (f, "%d", pX);
		if (res != 1)
			*pX = 0;
		fclose (f);
	}
	else
		*pX = 0;
}

int main (int c, char **v)
{
	int res;
	FILE *f;
	int x;
	//while (1)
	{
		// Atomic
		f = fopen (FILE1, "r");
		readint (f, &x);
		f = fopenAtomic (FILE1, "w");
		if (!f)
		{
			fprintf (stderr, "Can't open %s for write", FILE1);
			exit (1);
		}
		fprintf (f, "%d\n", 1+x);
		res = fcloseAtomic (f);
		if (res)
		{
			fprintf (stderr, "Can't close %s for write", FILE1);
			exit (1);
		}

		// Non-Atomic
		f = fopen (FILE2, "r");
		readint (f, &x);
		f = fopen (FILE2, "w");
		if (!f)
		{
			fprintf (stderr, "Can't open %s for write", FILE2);
			exit (1);
		}
		fprintf (f, "%d\n", 1+x);
		res = fclose (f);
		if (res)
		{
			fprintf (stderr, "Can't close %s for write", FILE2);
			exit (1);
		}
	}
}
