// **************************************************************
//		General utility functions
//
//				Written by Gints Klimanis
// ************************************************************** 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

/* **********************************************************************
 * RemoveCharacter:   remove all target characters
 *		    Return # removed.
 * ********************************************************************** */
    int 
RemoveCharacter(char *s, char target)
{
int i, j;

if (!s)
    return (0);

/* scan for last space=' ' character at beginning of string */
for (i = 0, j = 0; s[j] != '\0'; j++)
    {    
    if (s[j] != target)
	s[i++] = s[j];
    }
s[i] = '\0';

return (j-i);
}   /* ---- end RemoveCharacter() ---- */

/* **********************************************************************
 * RemoveNonNumericals:   remove all characters not in set 
 *			    {'0'..'9', '.', '-'}
 *		    Return # characters removed.
 * ********************************************************************** */
    int 
RemoveNonNumericals(char *s)
{
int i, j;
if (!s)
    return (0);

for (i = 0, j = 0; s[j] != '\0'; j++)
    {    
    if (s[j] == '-' || s[j] == '.' || (s[j] >= '0' && s[j] <= '9'))
	s[i++] = s[j];
    }
s[i] = '\0';

return (j-i);
}   /* ---- end RemoveNonNumericals() ---- */

/* **********************************************************************
 * IsNumerical:		disallow non-numerical characters:  
 *			    Must be in set: 
 *				{'0'..'9', '.', '-'}
 *				Input typed from keyboard is of length 1.  
 *				Xpasted input may be any length
 * ********************************************************************** */
    Boolean 
IsNumerical(char *s) 
{
int i;
int foundNumerical = False;

if (!s)
    return (False);

for (i = 0; s[i] != '\0'; i++)
    {    
    if	    (s[i] == ' ' || s[i] == '-' || s[i] == '+' || s[i] == '.')
		foundNumerical = True;
    else if (s[i] >= '0' && s[i] <= '9')
		foundNumerical = True;
    else
		return (False);
    }
return (foundNumerical);
}   /* ---- end IsNumerical() ---- */

/* **********************************************************************
 * IsPositiveNumerical:		disallow non-numerical characters: 
 *			    Must be in set: 
 *				{'0'..'9', '.'}
 *				Input typed from keyboard is of length 1.  
 *				Xpasted input may be any length
 * ********************************************************************** */
    Boolean 
IsPositiveNumerical(char *s) 
{
Boolean foundNumerical = False;
int i;
if (!s)
    return (False);

for (i = 0; s[i] != '\0'; i++)
    {    
    if	    (s[i] == ' ' || s[i] == '.' || s[i] == '+')
	foundNumerical = True;
    else if (s[i] >= '0' && s[i] <= '9')
	foundNumerical = True;
    else
	return (False);
    }

return (foundNumerical);
}   /* ---- end IsPositiveNumerical() ---- */

// ********************************************************************** 
// TranslateWaitID:   Translate Event wait ID to an English string
// ********************************************************************** 
    char *
TranslateWaitID(DWORD x)
{
switch (x)
	{
	 case WAIT_ABANDONED:
		return ("WAIT_ABANDONED");
	 case WAIT_OBJECT_0:
		return ("WAIT_OBJECT_0");
	 case WAIT_TIMEOUT:
		return ("WAIT_TIMEOUT");
	 case WAIT_FAILED:
		return ("WAIT_FAILED");
	}

return ("Bogus");
}	// ---- end TranslateWaitID() ---- 

