// Atomic File I/O
// RDowling 9/29/09
//
// Simple interface that guarantees that a file on close will be either
// completely changed or completely unchanged if close fails.  Basic strategy
// is to open a 2nd file, write there, and do POSIX rename upon close.
//
// Magic is only in the fopen/fclose calls.  Otherwise, use FILE* normally to
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
//
// 2/10/2010: Per TTPro 2208, the above implementation has proven
// woefully inadequate.  Cases of a game exiting without closing file
// prevents other games from doing any atomic I/O.  We must have a
// list of open handles.
//
// Brings up new failure condition: what if an already fopenAtomic
// file is fopenAtomic'd again?  In this case, we choose to fail
// (return 0) instead of be smart.  Otherwise, as many files as we
// have RAM are supported now.
//
// Add new feature to abort all known atomic files, so that we can do
// clean up from AppManager on game exit, in case games misbehave.
// 
// Also add support for append mode, and guaranteed new shadow file
// names using mktemp.

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <list>
#ifndef NOBRIO
#include <Utility.h>
#include <DebugMPI.h>
#endif

// #error need append
// #error sprinle in abortall

#include "AtomicFile.h"

#define	ATOMIC_EXT	".XXXXXX"

using namespace std;

// The information we need to maintain for atomic access
// Keep original FILE* around to support 2nd
struct atomic_info
{
	FILE *file;
	char *realName;
	char *workName;
};

static list<struct atomic_info *> atomicList;

#ifdef NOBRIO
// No BRIO
#define ATOMIC_ERR(x)		fprintf (stderr, x)
#define ATOMIC_ERR1(x,y)	fprintf (stderr, x, y)
#define ATOMIC_ERR2(x,y,z)	fprintf (stderr, x, y, z)
#define LF_USING_BRIO_NAMESPACE()
#else
// Yes BRIO: make errors go to DebugOut
#define ATOMIC_ERR(x)		debug.DebugOut(kDbgLvlImportant, x)
#define ATOMIC_ERR1(x,y)	debug.DebugOut(kDbgLvlImportant, x, y)
#define ATOMIC_ERR2(x,y,z)	debug.DebugOut(kDbgLvlImportant, x, y, z)
#endif

LF_USING_BRIO_NAMESPACE()

// Find an already open atomic file given its File *, or return NULL
static struct atomic_info *find_by_fp (FILE *fp)
{
	list<struct atomic_info *>::iterator p;
	for (p=atomicList.begin(); p!= atomicList.end(); p++)
		if ((*p)->file == fp)
			return *p;
	return NULL;
}

// Find an already open atomic file given its realName, or return NULL
static struct atomic_info *find_by_realName (const char *realName)
{
	list<struct atomic_info *>::iterator p;
	for (p=atomicList.begin(); p!= atomicList.end(); p++)
		if (!strcmp ((*p)->realName, realName))
			return *p;
	return NULL;
}

#ifdef DEBUG
// Helper
void show_atomics ()
{
	list<struct atomic_info *>::iterator p;
	int i=0;
	printf ("---\n");
	for (p=atomicList.begin(); p!= atomicList.end(); p++)
	{
		printf ("i=%d realName=%s", i, (*p)->realName ? (*p)->realName : "NULL");
		printf ("workName=%s fp=%p\n", (*p)->workName ? (*p)->workName : "NULL", (*p)->file);
		i++;
	}
}
#else
#define show_atomics()
#endif

static int fabortGuts (struct atomic_info *atomicOpen)
{
#ifndef NOBRIO
	CDebugMPI debug(kGroupPlatform);
#endif
	if (!atomicOpen)
	{
		ATOMIC_ERR ("fabortGuts: NULL atomicOpen\n");
		return -1;
	}

	// Close and remove working file, free atomicOpen data
	FILE *fp = atomicOpen->file;
	if (!fp)
	{
		ATOMIC_ERR1 ("fabortGuts: fp is null for %x!\n", (int) atomicOpen);
		return -1;
	}

	if (fclose (fp))
	{
		ATOMIC_ERR1 ("fabortGuts: fclose failed for fp=0x%08x!\n", (int) fp);
		return -1;
	}
	if (atomicOpen->workName)
	{
		int res = unlink (atomicOpen->workName);
		if (res)
			ATOMIC_ERR1 ("fabortGuts: could not remove %s\n", atomicOpen->workName);
		free (atomicOpen->workName);
	}
	if (atomicOpen->realName)
		free (atomicOpen->realName);
	return 0;
}


// Used to close an atomic update file, discarding any changes.
// Use this when you discover and error after calling fopenAtomic, and don't
// wish to disturb the original file.
// Return 0 for OK, -1 for failure
int fabortAtomic (FILE *fp)
{
#ifndef NOBRIO
	CDebugMPI debug(kGroupPlatform);
#endif
	show_atomics ();
	struct atomic_info *atomicOpen = find_by_fp (fp);
	if (!atomicOpen)
	{
		ATOMIC_ERR1 ("fabortAtomic(0x%08x), this FILE not open in atomic mode!\n", (int) fp);
		return -1;
	}
	int res = fabortGuts (atomicOpen);
	if (res)
		return res;

	// Remove from list
	atomicList.remove (atomicOpen);
	delete atomicOpen;
	
	ATOMIC_ERR1 ("fabortAtomic(0x%08x) returning 0\n", (unsigned) fp);
	return 0;
}

// Close all atomic update files, discarding all changes in all files.
// Use this to clear out the system under an error condition where all
// activity is suspect.
// Return 0 for OK, -1 for failure
int fabortAllAtomic ()
{
#ifndef NOBRIO
	CDebugMPI debug(kGroupPlatform);
#endif
	show_atomics ();
	list<struct atomic_info *>::iterator p, q;
	int i = 0;
	for (p=atomicList.begin(); p!= atomicList.end(); p=q)
	{
		struct atomic_info *atomicOpen = *p;
		ATOMIC_ERR2 ("fabortAllAtomic: aborting i=%d realName=%s\n", i++, (*p)->realName ? (*p)->realName : "NULL");
		ATOMIC_ERR2 ("fabortAllAtomic:          workName=%s fp=0x%08x\n", (*p)->workName ? (*p)->workName : "NULL", (int) (*p)->file);
		if (!atomicOpen)
		{
			ATOMIC_ERR ("fabortAllAtomic: impossible condition: NULL entry in atomicList\n");
			return -1;
		}
		if (!atomicOpen->file)
		{
			ATOMIC_ERR1 ("fabortAllAtomic: impossible condition: NULL FILE in atomicList realName=%s\n", atomicOpen->realName);
			return -1;
		}
		// Reuse normal abort guts, without delete
		int res = fabortGuts (atomicOpen);
		if (res)
			return res;
		// Do delete safely wrt iterators
		q = p;
		q++;
		atomicList.erase (p);
		delete atomicOpen;
	}
	ATOMIC_ERR1 ("fabortAllAtomic: aborted %d files\n", i);
	show_atomics ();
	return 0;
}

// Helper
static int copy_file(const char *srcPath, const char *dstPath)
{
#ifndef NOBRIO
	CDebugMPI debug(kGroupPlatform);
#endif
	int ret = 0; // Assume success
	int fin, fout;
	fin = open (srcPath, O_RDONLY);
	if (fin < 0)
	{
		ATOMIC_ERR1 ("copy_file: Unable to open %s for read\n", srcPath);
		return -1;
	}
	fout = open (dstPath, O_CREAT|O_TRUNC|O_WRONLY, 0666); // mode=0666 comes from fopen's default
	if (fout < 0)
	{
		ATOMIC_ERR1 ("copy_file: Unable to open %s for read\n", dstPath);
		return -1;
	}

	int cnt;
	char buff[512];

	while (cnt = read (fin, buff, sizeof(buff)))
	{
		int wcnt = write(fout, buff, cnt);
		if (cnt != wcnt)
		{
			ret = -1;
			ATOMIC_ERR2 ("copy_file: Write %d returned %d\n", cnt, wcnt);
			break;
		}
	}
	if (close (fin))
	{
		ATOMIC_ERR1 ("copy_file: Close %s failed!\n", srcPath);
		ret = -1;
	}
	if (close (fout))
	{
		ATOMIC_ERR1 ("copy_file: Close %s failed!\n", dstPath);
		ret = -1;
	}
		
	if (!ret)
		ATOMIC_ERR2 ("copy_file: %s -> %s success\n", srcPath, dstPath);
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
// Open a file for atomic writing.  Use like fopen.
// Actually opens file with derivative name.
FILE *fopenAtomic(const char *path, const char *mode)
{
#ifndef NOBRIO
	CDebugMPI debug(kGroupPlatform);
#endif

	// Search for previously opened file by same name
	struct atomic_info *atomicOpen = find_by_realName (path);
	if (atomicOpen)
	{
		// Not cool.
		ATOMIC_ERR1 ("fopenAtomic(%s): Can't fopenAtomic same file twice!  Close other.\n", path);
		return NULL;
	}

	// Make sure there is only writing or appending requested
	const char *s;
	int append=0;
	for (s=mode; *s; s++)
	{
		if (tolower(*s) == 'r')
		{
			ATOMIC_ERR1 ("fopenAtomic(%s): Can't use with read modes!\n", path);
			return NULL;
		}
		if (tolower(*s) == 'a')
		{
			append=1;
		}
	}

	// Make a record
	atomicOpen = new struct atomic_info;
	if (!atomicOpen)
	{
		ATOMIC_ERR1 ("fopenAtomic(%s): Unable to 'new struct atomic_info'\n", path);
		return NULL;
	}

	// Make up a new name based on old name, and save both
	atomicOpen->realName = strdup (path);
	if (strlen (atomicOpen->realName) == 0)
	{
		ATOMIC_ERR1 ("fopenAtomic(%s): strdup failed us!\n", path);
		free (atomicOpen->realName);
		delete atomicOpen;
		return NULL;
	}
	atomicOpen->workName = (char *)malloc (1+strlen(path)+strlen(ATOMIC_EXT));
	strcpy (atomicOpen->workName, path);
	strcat (atomicOpen->workName, ATOMIC_EXT);
	mkstemp (atomicOpen->workName);
	if (strlen (atomicOpen->workName) == 0)
	{
		ATOMIC_ERR1 ("fopenAtomic(%s): mkstemp failed us!\n", path);
		free (atomicOpen->workName);
		free (atomicOpen->realName);
		delete atomicOpen;
		return NULL;
	}

	// Handle append mode by copying original file
	if (append)
	{
		int fd = open (atomicOpen->realName, O_RDONLY);
		if (fd >= 0)
		{
			close (fd);
			if (copy_file (atomicOpen->realName, atomicOpen->workName))
			{
				ATOMIC_ERR1 ("fopenAtomic(%s): copy_file failed!\n", path);
				goto failure;
			}
			ATOMIC_ERR1 ("fopenAtomic(%s): copy done\n", path);
		}
		else
		{
			ATOMIC_ERR1 ("fopenAtomic(%s): Nothing to Append\n", path);
		}
	}

	// Try to open working file
	atomicOpen->file = fopen (atomicOpen->workName, mode);
	if (!atomicOpen->file)
	{
		ATOMIC_ERR2 ("fopenAtomic(%s): fopen mode='%s' failed!\n", atomicOpen->workName, mode);
		goto failure;
	}

	// Save in list
	atomicList.push_back (atomicOpen);

	// Success
	ATOMIC_ERR2 ("fopenAtomic(%s) returning 0x%08x\n", path, (unsigned) atomicOpen->file);
	return atomicOpen->file;

	// Failure; clean up
 failure:
	free (atomicOpen->realName);
	free (atomicOpen->workName);
	atomicOpen->realName = atomicOpen->workName = NULL;
	delete atomicOpen;
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// Close file opened by fopenAtomic and either update real file or leave it
// alone.  Use like fclose
int fcloseAtomic(FILE *fp)
{
#ifndef NOBRIO
	CDebugMPI debug(kGroupPlatform);
#endif
	// Make sure it was fopenAtomic'd
	struct atomic_info *atomicOpen = find_by_fp (fp);
	if (!atomicOpen)
	{
		ATOMIC_ERR1 ("fcloseAtomic(0x%08x), this FILE not open in atomic mode!\n", (int) fp);
		return -1;
	}
	// Do atomic dance
	int res = fflush (fp);
	if (res)
	{
		ATOMIC_ERR1 ("fcloseAtomic fflush failed; returning %d\n", res);
		return res;
	}
	int fd = fileno (fp);
	res = fdatasync (fd);
	if (res)
	{
		ATOMIC_ERR1 ("fcloseAtomic fdatasync failed; returning %d\n", res);
		return res;
	}
	res = fclose (fp);
	if (res)
	{
		ATOMIC_ERR1 ("fcloseAtomic fclose failed; returning %d\n", res);
		return res;
	}
	// POSIX atomic magic
	res = rename (atomicOpen->workName, atomicOpen->realName);

	// At this point, there is no going back, so free structures
	free (atomicOpen->realName);
	free (atomicOpen->workName);
	// Remove from list
	atomicList.remove (atomicOpen);
	delete atomicOpen;

	ATOMIC_ERR1 ("fcloseAtomic returning %d\n", res);
	return res;
}
