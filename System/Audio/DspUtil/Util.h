// **************************************************************
//    Gints Klimanis
//				1996
// ************************************************************** 
#ifndef _util_H
#define _util_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RINT
#define RINT(x) ((int)(x + 0.5))
#endif

#define Boolean	int

#ifndef False
#define False	0
#endif
#ifndef True
#define True	1
#endif

int RemoveCharacter			(char *s, char target);
int RemoveNonNumericals	    (char *s);
Boolean IsNumerical			(char *s);
Boolean IsPositiveNumerical (char *s);

char *StringDuplicate		(char *s);

int  ProcessString			(char *source);
int  MembershipCheck		(char source[], char set[]);
void StrFormatNumerical	    (char s1[]);
int  CharacterCount			(char source[], char targetCharacter);
int  StrRemoveLeadingZeros  (char s[]);
int  StrRemoveTrailingZeros (char s[]);

#ifndef _MAX_PATH
#define _MAX_PATH 500
#endif

#ifdef __cplusplus
}
#endif

#endif
