// Atomic File I/O
// RDowling 9/29/09
//
// Simple interface that guarantees that a file on close will be either
// completely changed or completely unchanged if close fails.  Basic strategy
// is to open a 2nd file, write there, and do POSIC rename upon close.
//
// Magic is only in the fopen/fclose calls.  Otherwise, use FILE* normaly to
// write.  
//
//
// Caveats:
// * Error messages are sent to stderr right now
// * fopenAtomic generates a derivative file name with ATOMIC_EXT appended.
//   If that file already exists, it will get clobbered.
// * Only supports writing to files, not read or append.
// * At most MAX_ATOMIC=1 files can be open at once for atomic write.
// * fabortAtomic call allow for reverting changes instead of fcloseAtomic.

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <unistd.h>

#include "AtomicFile.h"

#define MAX_ATOMIC	1
#define	ATOMIC_EXT	".atomic"
#define ATOMIC_UNUSED	{ -1 }

#define ATOMIC_ERR(x)		fprintf (stderr, x);
#define ATOMIC_ERR1(x,y)	fprintf (stderr, x, y);

// The information we need to maintain for atomic access
// Keep original FILE* around to support 2nd
static struct atomic_info
{
	int fd;
	FILE *file;
	char *realName;
	char *workName;
} atomicOpen = ATOMIC_UNUSED;


// Used to close an atomic update file, discarding any changes.
// Use this when you discover and error after calling fopenAtomic, and don't
// wish to disturb the original file.
// Return 0 for OK, -1 for failure
int fabortAtomic (FILE *fp)
{
	int fd = fileno (fp);
	if (atomicOpen.fd != fd && atomicOpen.fd >= 0)
	{
		ATOMIC_ERR1 ("fabortAtomic: fd=%d not open in atomic mode!\n", fd);
		return -1;
	}
	// Currently open?
	if (atomicOpen.fd >= 0)
	{
		// Close and remove working file, free atomicOpen data
		fclose (fp);
		if (atomicOpen.workName)
		{
			int res = unlink (atomicOpen.workName);
			if (res)
				ATOMIC_ERR1 ("fabortAtomic: could not remove %s", atomicOpen.workName);
			free (atomicOpen.workName);
		}
		if (atomicOpen.realName)
			free (atomicOpen.realName);
	}
	// Regardless, reset to unused state
	atomicOpen = (struct atomic_info)ATOMIC_UNUSED;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// Open a file for atomic writing.  Use like fopen.
// Actually opens file with derivative name.
FILE *fopenAtomic(const char *path, const char *mode)
{
	// Make sure there is only writing requested
	const char *s;
	for (s=mode; *s; s++)
	{
		if (tolower(*s) == 'a' || tolower(*s) == 'r')
		{
			ATOMIC_ERR ("fopenAtomic: Can't use with append or read modes!\n");
			return NULL;
		}
	}

	// Make sure there is room in atomicOpen structure
	if (atomicOpen.fd != -1)
	{
		ATOMIC_ERR ("fopenAtomic: Ran out of atomic file handles!\n");
		return NULL;
	}

	// Make up a new name based on old name, and save both
	// Don't try very hard on name
	// FIXME: test and iterate name until it works or we get tired
	atomicOpen.realName = strdup (path);
	atomicOpen.workName = (char *)malloc (1+strlen(path)+strlen(ATOMIC_EXT));
	strcpy (atomicOpen.workName, path);
	strcat (atomicOpen.workName, ATOMIC_EXT);

	// Try to open working file
	FILE *fout = fopen (atomicOpen.workName, mode);
	if (!fout)
	{
		// Failed
		free (atomicOpen.realName);
		free (atomicOpen.workName);
		atomicOpen.realName = atomicOpen.workName = NULL;
		return fout;
	}

	// We fd also for fsync call later
	int fd = fileno (fout);

	// Save in opens list
	atomicOpen.fd = fd;
	atomicOpen.file = fout;

	return fout;
}

//////////////////////////////////////////////////////////////////////////////
// Close file opened by fopenAtomic and either update real file or leave it
// alone.  Use like fclose
int fcloseAtomic(FILE *fp)
{
	// Make sure it was fopenAtomic'd
	int fd = fileno (fp);
	if (atomicOpen.fd != fd)
	{
		// Failed
		if (atomicOpen.fd == -1)
		{
			ATOMIC_ERR ("fcloseAtomic: No atomic files open!\n");
			return -1;
		}
		ATOMIC_ERR1 ("fcloseAtomic: fd=%d not open in atomic mode!\n", fd);
		return -1;
	}

	// Do atomic dance
	int res = fdatasync (fd);
	if (res)
		return res;
	res = fclose (fp);
	if (res)
		return res;
	sync (); // Gotta be exactly here to work.  Otherwise 1% failure rate
	// POSIX atomic magic
	res = rename (atomicOpen.workName, atomicOpen.realName);

	// At this point, there is no going back, so free structures
	free (atomicOpen.realName);
	free (atomicOpen.workName);
	atomicOpen = (struct atomic_info)ATOMIC_UNUSED;

	return res;
}
