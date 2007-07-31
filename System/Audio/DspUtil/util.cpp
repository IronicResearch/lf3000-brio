// *************************************************************** 
// util.cpp:		Miscellaneous audio-dsp related functions
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "util.h"

// **********************************************************************************
// PrintTimeval:	Print struct stuff to console
// ********************************************************************************** 
	void   PrintTimeval(struct timeval *tv)
{
printf("tv:  sec=%d usec=%d -> %g seconds\n",  tv->tv_sec, tv->tv_usec,
	((double) tv->tv_sec) + 0.000001*(double) tv->tv_usec);
}	// ---- end PrintTimeval() ---- 

// **********************************************************************************
// SecondsFromTimeval:	Return time in seconds	
// ********************************************************************************** 
	double   
SecondsFromTimeval(struct timeval *tv)
{
double time = ((double) tv->tv_sec) + 0.000001*(double) tv->tv_usec;
return (time);
}	// ---- end SecondsFromTimeval() ---- 

// **********************************************************************************
// SecondsFromTimevalDiff:	Return time in seconds from two timeval structs	
// ********************************************************************************** 
	double   
SecondsFromTimevalDiff(struct timeval *startTv, struct timeval *endTv)
{
double endTime   = ((double)  endTv->tv_sec) + 0.000001*(double)  endTv->tv_usec;
double startTime = ((double)startTv->tv_sec) + 0.000001*(double)startTv->tv_usec;
return (endTime - startTime);
}	// ---- end SecondsFromTimevalDiff() ---- 

// **********************************************************************************
// AddTimeval:	Add timeval structs using integer-only arithmetic
//			Result is in argument a = a + b
// ********************************************************************************** 
	void   
AddTimeval(struct timeval *a, struct timeval *b)
{
a->tv_sec  += b->tv_sec;
a->tv_usec += b->tv_usec;

// Check and compensate for any overflow in microsecond addition
if (a->tv_usec >= 1000000)
	{
	a->tv_sec  += 1;
	a->tv_usec -= 1000000;
	}
}	// ---- end AddTimeval() ---- 

//**********************************************************************************
// SubTimeval:	Subtract timeval structs using integer-only arithmetic
//			Result is in argument a = b - a
//********************************************************************************** 
	void   
SubTimeval(struct timeval *a, struct timeval *b)
{
a->tv_sec  = b->tv_sec  - a->tv_sec;
a->tv_usec = b->tv_usec - a->tv_usec;

// Check and compensate for any overflow in microsecond addition
if (a->tv_usec < 0)
	{
	a->tv_sec  -= 1;
	a->tv_usec += 1000000;
	}
}	// ---- end SubTimeval() ---- 

// **********************************************************************************
// AddTimevalDiff:	Add timeval structs using integer-only arithmetic
//			Result is in argument y += (b-a)
// ********************************************************************************** 
	void   
AddTimevalDiff(struct timeval *y, struct timeval *a, struct timeval *b)
{
struct timeval diff;

SetTimeval(&diff, a);
SubTimeval(&diff, b);
AddTimeval(y, &diff);
}	// ---- end AddTimevalDiff() ---- 

// **********************************************************************************
// ClearTimeval:	Set timeval structs to zero
// ********************************************************************************** 
	void   
ClearTimeval(struct timeval *x)
{
x->tv_sec  = 0;
x->tv_usec = 0;
}	// ---- end ClearTimeval() ---- 

// **********************************************************************************
// SetTimeval:	Set timeval structs y = x
// ********************************************************************************** 
	void   
SetTimeval(struct timeval *y, struct timeval *x)
{
y->tv_sec  = x->tv_sec;
y->tv_usec = x->tv_usec;
}	// ---- end SetTimeval() ---- 

// **********************************************************************************
// Stricmp:	Compare two strings with case insensitivity.
//			Return 0 if strings are the same.	
// ********************************************************************************** 
	int   
Stricmp(char *inS1, char *inS2)
{
char s1[1000], s2[1000];
long i, j;
long returnValue = 1;

if ((!inS1) || (!inS2))
	return (1);

strcpy(s1, inS1);
strcpy(s2, inS2);
// Force all values to lower case
for (i = 0; s1[i] != '\0'; i++)
	{
	if (s1[i] >= 'A' && s1[i] <= 'Z')
		s1[i] = s1[i] - 'A' + 'a';
	}
for (i = 0; s2[i] != '\0'; i++)
	{
	if (s2[i] >= 'A' && s2[i] <= 'Z')
		s2[i] = s2[i] - 'A' + 'a';
	}
//printf("inS1 '%s' -> '%s'\n", inS1, s1);
//printf("inS2 '%s' -> '%s'\n", inS2, s2);

for (i = 0; s1[i] != '\0' && s2[i] != '\0'; i++)
	{
	if (s1[i] != s2[i])
		break;
	}
// Check if reached end of either string.  If both, strings are the same
if (s1[i] == '\0' && s2[i] == '\0')
	return (0);
return (1);
}	// ---- end Stricmp() ---- 


