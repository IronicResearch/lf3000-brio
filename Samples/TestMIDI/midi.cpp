// **********************************************************************
//
// midi.cpp		Routines for MIDI processing
//
//					Written by Gints Klimanis
// **********************************************************************
#include <string.h>

#include <AudioMPI.h>

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
negative = false;
if (*s == '-')
    {
    s++;
    negative = true;
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
    else    
        letter = 0;     // Force garbage values to '0', Arbitrary choice.

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

// **********************************************************************
// GeneralMIDI_InstrumentNumberToName:	Convert General MIDI progam # to
//                                          its string name
// ********************************************************************** 
    char * 
GeneralMIDI_InstrumentNumberToName(int noteNumber)
{
switch (noteNumber)
    {
    case kGMIDI_001_AcousticGrandPiano:
    return ("AcousticGrandPiano");
    case kGMIDI_002_BrightAcousticPiano:
    return ("BrightAcousticPiano");
    case kGMIDI_003_ElectricGrandPiano:
    return ("ElectricGrandPiano");
    case kGMIDI_004_HonkyTonkPiano :
    return ("HonkyTonkPiano");
    case kGMIDI_005_ElectricPiano1:
    return ("ElectricPiano1");
    case kGMIDI_006_ElectricPiano2:
    return ("ElectricPiano2");
    case kGMIDI_007_Harpsichord:
    return ("Harpsichord");
    case kGMIDI_008_Clavi:
    return ("Clavi");
    case kGMIDI_009_Celesta:
    return ("Celesta");
    case kGMIDI_010_Glockenspiel:
    return ("Glockenspiel");

    case kGMIDI_011_MusicBox:
    return ("MusicBox");
    case kGMIDI_012_Vibraphone:
    return ("Vibraphone");
    case kGMIDI_013_Marimba:
    return ("Marimba");
    case kGMIDI_014_Xylophone:
    return ("Xylophone");
    case kGMIDI_015_TubularBells:
    return ("TubularBells");
    case kGMIDI_016_Dulcimer:
    return ("Dulcimer");
    case kGMIDI_017_DrawbarOrgan:
    return ("DrawbarOrgan");
    case kGMIDI_018_PercussiveOrgan:
    return ("PercussiveOrgan");
    case kGMIDI_019_RockOrgan:
    return ("RockOrgan");
    case kGMIDI_020_ChurchOrgan:
    return ("ChurchOrgan");

    case kGMIDI_021_ReedOrgan :
    return ("ReedOrgan");
    case kGMIDI_022_Accordion:
    return ("Accordion");
    case kGMIDI_023_Harmonica:
    return ("Harmonica");
    case kGMIDI_024_TangoAccordion:
    return ("TangoAccordion");
    case kGMIDI_025_AcousticGuitar_Nylon:
    return ("AcousticGuitar_Nylon");
    case kGMIDI_026_AcousticGuitar_Steel:
    return ("AcousticGuitar_Steel");
    case kGMIDI_027_ElectricGuitar_Jazz:
    return ("ElectricGuitar_Jazz");
    case kGMIDI_028_ElectricGuitar_Clean:
    return ("ElectricGuitar_Clean");
    case kGMIDI_029_ElectricGuitar_Muted:
    return ("ElectricGuitar_Muted");
    case kGMIDI_030_OverdrivenGuitar:
    return ("OverdrivenGuitar");

   case kGMIDI_031_DistortionGuitar:
    return ("DistortionGuitar");
   case kGMIDI_032_GuitarHarmonics:
    return ("GuitarHarmonics");
   case kGMIDI_033_AcousticBass:
    return ("AcousticBass");
   case kGMIDI_034_ElectricBass_Finger:
    return ("ElectricBass_Finger");
   case kGMIDI_035_ElectricBass_Pick:
    return ("ElectricBass_Pick");
   case kGMIDI_036_FretlessBass:
    return ("FretlessBass");
   case kGMIDI_037_SlapBass1:
    return ("SlapBass1");
   case kGMIDI_038_SlapBass2:
    return ("SlapBass2");
   case kGMIDI_039_SynthBass1 :
    return ("SynthBass1");
   case kGMIDI_040_SynthBass2 :
    return ("SynthBass2");

   case  kGMIDI_041_Violin:
    return ("Violin");
   case  kGMIDI_042_Viola:
    return ("Viola");
   case  kGMIDI_043_Cello:
    return ("Cello");
   case  kGMIDI_044_Contrabass:
    return ("Contrabass");
   case  kGMIDI_045_TremoloStrings:
    return ("TremoloStrings");
   case  kGMIDI_046_PizzicatoStrings:
    return ("PizzicatoStrings");
   case  kGMIDI_047_OrchestralHarp:
    return ("OrchestralHarp");
   case  kGMIDI_048_Timpani:
    return ("Timpani");
   case  kGMIDI_049_StringEnsemble1:
    return ("StringEnsemble1");
   case  kGMIDI_050_StringEnsemble2:
    return ("StringEnsemble2");

   case  kGMIDI_051_SynthStrings1:
    return ("SynthStrings1");
   case  kGMIDI_052_SynthStrings2:
    return ("SynthStrings2");
   case  kGMIDI_053_ChoirAahs:
    return ("ChoirAahs");
   case  kGMIDI_054_VoiceOohs:
    return ("VoiceOohs");
   case kGMIDI_055_SynthVoice:
    return ("SynthVoice");
   case  kGMIDI_056_OrchestraHit:
    return ("OrchestraHit");
   case  kGMIDI_057_Trumpet:
    return ("Trumpet");
   case  kGMIDI_058_Trombone:
    return ("Trombone");
   case  kGMIDI_059_Tuba :
    return ("Tuba");
   case  kGMIDI_060_MutedTrumpet :
    return ("MutedTrumpet");

   case   kGMIDI_061_FrenchHorn:
    return ("FrenchHorn");
   case   kGMIDI_062_BrassSection:
    return ("BrassSection");
   case   kGMIDI_063_SynthBrass1:
    return ("SynthBrass1");
   case   kGMIDI_064_SynthBrass2:
    return ("SynthBrass2");
   case   kGMIDI_065_SopranoSax:
    return ("SopranoSax");
   case   kGMIDI_066_AltoSax:
    return ("AltoSax");
   case   kGMIDI_067_TenorSax:
    return ("TenorSax");
   case   kGMIDI_068_BaritoneSax:
    return ("BaritoneSax");
   case   kGMIDI_069_Oboe:
    return ("Oboe");
   case   kGMIDI_070_EnglishHorn:
    return ("EnglishHorn");

   case   kGMIDI_071_Bassoon:
    return ("Bassoon");
   case   kGMIDI_072_Clarinet:
    return ("Clarinet");
   case   kGMIDI_073_Piccolo:
    return ("Piccolo");
   case   kGMIDI_074_Flute:
    return ("Flute");
   case   kGMIDI_075_Recorder:
    return ("Recorder");
   case   kGMIDI_076_PanFlute:
    return ("PanFlute");
   case   kGMIDI_077_BlownBottle:
    return ("BlownBottle");
   case   kGMIDI_078_Shakuhachi:
    return ("Shakuhachi");
   case   kGMIDI_079_Whistle:
    return ("Whistle");
   case   kGMIDI_080_Ocarina:
    return ("Ocarina");

   case   kGMIDI_081_Lead1_Square:
    return ("Lead1_Square");
   case   kGMIDI_082_Lead2_Sawtooth:
    return ("Lead2_Sawtooth");
   case   kGMIDI_083_Lead3_Calliope:
    return ("Lead3_Calliope");
   case   kGMIDI_084_Lead4_Chiff:
    return ("Lead4_Chiff");
   case   kGMIDI_085_Lead5_Charang:
    return ("Lead5_Charang");
   case   kGMIDI_086_Lead6_Voice:
    return ("Lead6_Voice");
   case   kGMIDI_087_Lead7_Fifths:
    return ("Lead7_Fifths");
   case   kGMIDI_088_Lead8_BassPlusLead:
    return ("Lead8_BassPlusLead");
   case   kGMIDI_089_Pad1_NewAge:
    return ("Pad1_NewAge");
   case   kGMIDI_090_Pad2_Warm:
    return ("Pad2_Warm");

   case   kGMIDI_091_Pad3_Polysynth:
    return ("Pad3_Polysynth");
   case   kGMIDI_092_Pad4_Choir:
    return ("Pad4_Choir");
   case   kGMIDI_093_Pad5_Bowed:
    return ("Pad5_Bowed");
   case   kGMIDI_094_Pad6_Metallic:
    return ("Pad6_Metallic");
   case   kGMIDI_095_Pad7_Halo:
    return ("Pad7_Halo");
   case   kGMIDI_096_Pad8_Sweep:
    return ("Pad8_Sweep");
   case   kGMIDI_097_FX1_Rain:
    return ("FX1_Rain");
   case   kGMIDI_098_FX2_Soundtrack:
    return ("FX2_Soundtrack");
   case   kGMIDI_099_FX3_Crystal:
    return ("FX3_Crystal");
   case   kGMIDI_100_FX4_Atmosphere:
    return ("FX4_Atmosphere");

   case   kGMIDI_101_FX5_Brightness:
    return ("FX5_Brightness");
   case   kGMIDI_102_FX6_Goblins:
    return ("FX6_Goblins");
   case   kGMIDI_103_FX7_Echoes:
    return ("FX7_Echoes");
   case   kGMIDI_104_FX8_SciFi:
    return ("FX8_SciFi");
   case   kGMIDI_105_Sitar:
    return ("Sitar");
   case   kGMIDI_106_Banjo:
    return ("Banjo");
   case   kGMIDI_107_Shamisen:
    return ("Shamisen");
   case   kGMIDI_108_Koto:
    return ("Koto");
   case   kGMIDI_109_Kalimba:
    return ("Kalimba");
   case   kGMIDI_110_BagPipe:
    return ("BagPipe");

   case   kGMIDI_111_Fiddle:
    return ("Fiddle");
   case   kGMIDI_112_Shanai:
    return ("Shanai");
   case   kGMIDI_113_TinkleBell:
    return ("TinkleBell");
   case   kGMIDI_114_Agogo:
    return ("Agogo");
   case   kGMIDI_115_SteelDrums:
    return ("SteelDrums");
   case   kGMIDI_116_Woodblock:
    return ("Woodblock");
   case   kGMIDI_117_TaikoDrum:
    return ("TaikoDrum");
   case   kGMIDI_118_MelodicTom:
    return ("MelodicTom");
   case   kGMIDI_119_SynthDrum:
    return ("SynthDrum");
   case   kGMIDI_120_ReverseCymbal:
    return ("ReverseCymbal");

   case   kGMIDI_121_GuitarFretNoise:
    return ("GuitarFretNoise");
   case   kGMIDI_122_BreathNoise:
    return ("BreathNoise");
   case   kGMIDI_123_Seashore:
    return ("Seashore");
   case   kGMIDI_124_BirdTweet:
    return ("BirdTweet");
   case   kGMIDI_125_TelephoneRing:
    return ("TelephoneRing");
   case   kGMIDI_126_Helicopter:
    return ("Helicopter");
   case   kGMIDI_127_Applause:
    return ("Applause");
   case   kGMIDI_128_Gunshot:
    return ("Gunshot");

    }
return ("Dunno");
}	// ---- end GeneralMIDI_InstrumentNumberToName() ---- 


