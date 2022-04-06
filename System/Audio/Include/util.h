// *************************************************************** 
// util.h:	Header file for miscellaneous audio-dsp functions
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#ifndef __UTIL_H__
#define	__UTIL_H__

#include <math.h>
#include <time.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Add whatever here */
#ifdef __cplusplus
}
#endif

#ifndef False
#define False	0
#endif

#ifndef FALSE
#define FALSE	0
#endif
#ifndef False
#define False	0
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef True
#define True	1	
#endif

#ifndef LEFT
#define LEFT	0
#endif
#ifndef Left
#define Left	LEFT
#endif

#ifndef RIGHT
#define RIGHT	1
#endif

#ifndef Right
#define Right	RIGHT
#endif

#ifndef OFF
#define OFF		0
#endif
#ifndef Off
#define Off		OFF
#endif

#ifndef ON
#define ON		1
#endif
#ifndef On
#define On		ON
#endif

#ifndef LO
#define LO		1
#endif
#ifndef Lo
#define Lo		LO
#endif
#ifndef HI
#define HI		1
#endif
#ifndef Hi
#define Hi		HI
#endif

#define DecibelToLinear(d)		(pow(10.0, ((double)(d))*(1.0/20.0)))
#define LinearToDecibel(x)		(log10((x))*20.0)

int Stricmp(char *inS1, char *inS2);

double SecondsFromTimevalDiff(struct timeval *startTv, struct timeval *endTv);
double SecondsFromTimeval(struct timeval *Tv);

void   ClearTimeval  (struct timeval *x);
void   SetTimeval    (struct timeval *y, struct timeval *x);
void   PrintTimeval  (struct timeval *x);

void   AddTimeval    (struct timeval *y, struct timeval *x);
void   SubTimeval    (struct timeval *y, struct timeval *x);
void   AddTimevalDiff(struct timeval *y, struct timeval *a, struct timeval *b);


#endif  //	end __UTIL_H__
