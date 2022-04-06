#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>

#include "AtomicFile.h"

// Cases to test:
// Success on new file
// Success on old file
// fail, reading
// fail, appending
// fail, can't find directory
// fail, can't write to file
// fail, aborted write
// fail, 2nd file already exists
// fail, mismatch fopenAtomic and fclose
// fail, mismatch fopen and fcloseAtomic

#define FILE1 "/tmp/myfile"
#define FILE1a "/tmp/myfile1"
#define FILE2 "/temp/myfile"
#define FILE3 "/etc/myfile"
#define FILE4 "/tmp/nowrite"

static int nsuccess = 0;

void fail (const char *fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	fprintf (stdout, "Failure: ");
	vfprintf (stdout, fmt, ap);
	va_end (ap);
	perror ("why:");
	fabortAllAtomic();
	exit (1);
}

void succeed (const char *fmt, ...)
{
	va_list ap;
	va_start (ap, fmt);
	fprintf (stdout, "Success: ");
	vfprintf (stdout, fmt, ap);
	va_end (ap);
	// system ("cat " FILE1);
	nsuccess++;
}

#define TEN 10

int main (int c, char **v)
{
	FILE *fs[TEN];
	int i, res;
	unlink (FILE1);
	// Success on new file
	FILE *f = fopenAtomic (FILE1, "wt");
	if (!f)
		fail ("Can't open for write\n");
	
	fprintf (f, "this is my test, line 1\n");
	res = fcloseAtomic (f);
	if (res)
		fail ("fcloseAtomic res=%d\n", res);
	succeed ("new file\n");

	// Success on old file
	f = fopenAtomic (FILE1, "w");
	if (!f)
		fail ("Can't open for write\n");
	
	fprintf (f, "this is my test, try 2 on line 1\n");
	res = fcloseAtomic (f);
	if (res)
		fail ("fcloseAtomic res=%d\n", res);
	unlink (FILE1);
	succeed ("old file\n");

	// Success, can open two files
	f = fopenAtomic (FILE1, "w");
	if (!f)
		fail ("Can't open for write #1\n");
	FILE *f2 = fopenAtomic (FILE1a, "w");
	if (!f2)
		fail ("Can't open for write #2\n");
	res = fcloseAtomic (f);
	if (res)
		fail ("fcloseAtomic #1 res=%d\n", res);
	res = fcloseAtomic (f2);
	if (res)
		fail ("fcloseAtomic #2 res=%d\n", res);
	unlink (FILE1a);
	unlink (FILE1);
	succeed ("two files at once\n");

	// fail, can't open twice same file
	f = fopenAtomic (FILE1, "w");
	if (!f)
		fail ("Can't open for write #1\n");
	f2 = fopenAtomic (FILE1, "w");
	if (f2)
		fail ("Should not allow open again\n");
	res = fcloseAtomic (f);
	if (res)
		fail ("fcloseAtomic #1 res=%d\n", res);
	unlink (FILE1);
	succeed ("two files of same file at once\n");
	// fail, reading
	f = fopenAtomic (FILE1, "rw");
	if (f)
		fail ("Was able to open for read\n");
	succeed ("Read disabled correctly in open\n");

	// succeed, appending to new file
	unlink (FILE1);
	f = fopenAtomic (FILE1, "a");
	if (!f)
		fail ("Was not able to open for append!\n");
	fprintf (f, "foo\n");
	res = fcloseAtomic (f);
	if (res)
		fail ("fcloseAtomic append failed", res);
	system ("cat " FILE1);
	succeed ("Append to new file works\n");

	// succeed, appending to existing file
	f = fopenAtomic (FILE1, "a");
	if (!f)
		fail ("Was not able to open for append!\n");
	fprintf (f, "bar\n");
	res = fcloseAtomic (f);
	if (res)
		fail ("fcloseAtomic append failed", res);
	system ("cat " FILE1);
	unlink (FILE1);
	succeed ("Append to existing works\n");

	// fail, can't find directory
	f = fopenAtomic (FILE2, "wb");
	if (f)
		fail ("can't find directory failed\n");
	succeed ("Open correctly failed to find directory\n");

	// fail, can't write this file because of directory
	f = fopenAtomic (FILE3, "w");
	if (f)
		fail ("can't write this file failed\n");
	succeed ("Open correctly failed to allow write to file in wp directory\n");

	// fail, can't write this file because write protected itself
	chmod (FILE4, 0666);
	unlink (FILE4);
	creat (FILE4, 0400);
	f = fopenAtomic (FILE4, "w");
	if (!f)
		fail ("can't open for write\n");
	succeed ("open succeeded on 2nd file\n");
	fprintf (f, "try to write...\n");
	fcloseAtomic (f);
	if (res)
		fail ("fcloseAtomic res=%d\n", res);
	succeed ("Close did not fail, even though file was unwritable\n"
		 "Rename only needs write access to directory!\n");
	chmod (FILE4, 0666);
	unlink (FILE4);
	// fail, aborted write
	f = fopenAtomic (FILE1, "wt");
	if (!f)
		fail ("Can't open for write\n");
	fprintf (f, "this is my test, line 1\n");
	res = fabortAtomic (f);
	if (res)
		fail ("fabortAtomic res=%d\n", res);
	succeed ("Abort worked\n");

	// Stress test opening and closing in/out of order
	for (i=0; i<TEN; i++)
	{
		char buf[128];
		sprintf (buf, "%s.%d", FILE1, i);
		fs[i] = fopenAtomic (buf, "w");
		if (!fs[i])
			fail ("Can't open for write\n");
		fprintf (fs[i], "this is file %d\n", i);
	}
	for (i=0; i<TEN; i+=2)
	{
		res = fcloseAtomic (fs[i]);
		if (res)
			fail ("Can't close\n");
	}
	for (i=1; i<TEN; i+=2)
	{
		res = fcloseAtomic (fs[i]);
		if (res)
			fail ("Can't close\n");
	}
	for (i=0; i<TEN; i++)
	{
		char buf[128];
		sprintf (buf, "%s.%d", FILE1, i);
		unlink (buf);
	}
	succeed ("Stress test opening and closing in/out of order\n");

	// Stress test fabortAllAtomic
	for (i=0; i<TEN; i++)
	{
		char buf[128];
		sprintf (buf, "%s.%d", FILE1, i);
		fs[i] = fopenAtomic (buf, "w");
		if (!fs[i])
			fail ("Can't open for write\n");
		fprintf (fs[i], "this is file %d\n", i);
	}
	// Close a couple by hand
	res = fcloseAtomic (fs[1]); fs[1] = NULL;
	if (res)
		fail ("fcloseAtomic res=%d\n", res);
	res = fcloseAtomic (fs[3]); fs[3] = NULL;
	if (res)
		fail ("fcloseAtomic res=%d\n", res);
	res = fabortAtomic (fs[5]); fs[5] = NULL;
	if (res)
		fail ("fabortAtomic res=%d\n", res);
	res = fabortAllAtomic ();
	if (res)
		fail ("fabortAllAtomic res=%d\n", res);
	for (i=0; i<TEN; i++)
	{
		res = fcloseAtomic (fs[i]);
		if (!res)
			fail ("fabortAllAtomic res=%d\n", res);
	}
	unlink (FILE1 ".1");
	unlink (FILE1 ".3");
	succeed ("Stress test fabortAllAtomic: working\n");

	//////////////////////////////////////////////////
	fprintf (stdout, "\n%d successful tests\n", nsuccess);
	return 0;
}
