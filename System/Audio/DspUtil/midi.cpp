// **********************************************************************
//
// midi.cpp		Routines for MIDI processing
//
//					Written by Gints Klimanis
// **********************************************************************
#include <string.h>

#include "midi.h"

// **********************************************************************
// MIDINoteToNotation:	Convert MIDI Note to string
//					w/musical notation 
//
//	Range [0..127] -> [C-1..G9], where middle C is C4=60. 
// ********************************************************************** 
    int 
MIDINoteToNotation(int noteNumber, char *note, int useFlats)
// noteNumber	note# to convert
//    note		ptr to space for output 
{
char	*ptr;
int	octave;

if (noteNumber > 127)
    return (0);

// Determine note letter 
ptr = note;
switch (((int) noteNumber)%12)
    {
    case 0:
	*ptr++ = 'C';
    break;
    case 1:
		if (useFlats)
		{
		*ptr++ = 'D';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'C';
		*ptr++ = '#';
		}
    break;
    case 2:
	*ptr++ = 'D';
    break;
    case 3:
    	if (useFlats)
		{
		*ptr++ = 'E';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'D';
		*ptr++ = '#';
		} 
	break;
    case 4:
	*ptr++ = 'E';
    break;
    case 5:
	*ptr++ = 'F';
    break;
    case 6:
       	if (useFlats)
		{
		*ptr++ = 'G';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'F';
		*ptr++ = '#';
		} 
	break;
    case 7:
	*ptr++ = 'G';
    break;
    case 8:
		if (useFlats)
		{
		*ptr++ = 'A';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'G';
		*ptr++ = '#';
		} 
	break;
    case 9:
	*ptr++ = 'A';
    break;
    case 10:
		if (useFlats)
		{
		*ptr++ = 'B';
		*ptr++ = 'b';
		}
		else
		{
		*ptr++ = 'A';
		*ptr++ = '#';
		} 
	break;
    case 11:
	*ptr++ = 'B';
    break;
    }

// Determine octave# around C0 = MIDI note #12 
octave = ((int) noteNumber)/12 - 1;
if (octave < 0)
    {
    *ptr++ = '-';
    octave = -octave;
    }
*ptr++ = '0' + ((char) octave);
*ptr = '\0';

#ifdef DEBUG
printf("MIDINoteToNotation() octave=%d", octave);
printf("MIDINoteToNotation(): %d -> '%s'", noteNumber, note);
#endif
return (1);
}	// ---- end MIDINoteToNotation() ---- 

// **********************************************************************
// NotationToMIDINote:	Return MIDI Note # of string
//							w/musical notation  
//
//					Return (-1) on error
// ********************************************************************** */
    int 
NotationToMIDINote(char *note)
// note	    ptr to notation string 
{
int	MIDINoteNumber;
char	*s;
int	negative, octave, letter;

/* MIDI range is values 0..127
MIDI Note #	note
	0   = C-1
	60  = C4
	120 = C9
	127 = G9
note will be in format Letter[A..G,a..g]<#,f,b>number[-2..9] 
example:    D#4
*/

// 1st character must be letter in range ['a'..'g','A'..'G'] 
s = note;
// capitalize 
if  (*s >= 'a' && *s <= 'g')
    {
    *s -= 'a';
    *s += 'A';
    }
if  (*s >= 'A' && *s <= 'G')
    s++;
else
    return (-1);

// 2nd character may be '#'=sharp, 'b'or'f'=flat, '-', '0'..'9' *
if (*s == '#' || *s == 'b'|| *s == 'f')	    //  advance past modifier 
    s++;
negative = FALSE;
if (*s == '-')
    {
    s++;
    negative = TRUE;
    }
if (*s >= '0' && *s <= '9')
    {
    octave = (int)(*s - '0');
    if (negative)
	octave = -octave;

    if	    (note[0] == 'C')
		letter = 0;
    else if (note[0] == 'D')
		letter = 2;
    else if (note[0] == 'E')
		letter = 4;
    else if (note[0] == 'F')
		letter = 5;
    else if (note[0] == 'G')
		letter = 7;
    else if (note[0] == 'A')
		letter = 9;
    else if (note[0] == 'B')
		letter = 11;

// Deal w/"sharp" character 
    if (note[1] == '#')
		letter++;
// Deal w/"flat" character 
   else if (note[1] == 'b' || note[1] == 'f')
		letter--;

// Compute MIDI note#.  12 is value of C0 
    MIDINoteNumber = 12 + letter + octave*12;

#ifdef DEBUG_UTIL
printf("NotationToMIDINote() '%s'->%d, letter=%d, octave=%d\n", 
	note, MIDINoteNumber, letter, octave);
#endif
// Ensure result is in MIDI note range 0..127 
    if (MIDINoteNumber < 0 || MIDINoteNumber > 127)
	return (-1);
    s++;
    }
else
    return (-1);

if (*s != '\0')
    return (-1);

return ((int) MIDINoteNumber);
}	// ---- end NotationToMIDINote() ---- 

