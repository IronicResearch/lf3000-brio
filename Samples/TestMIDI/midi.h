// *************************************************************************
//
// midi.h Header file for MIDI functions
//
//				Written by Gints Klimanis
// *************************************************************************

#ifndef __MIDI_H__
#define	__MIDI_H__

#ifdef __cplusplus
extern "C" {
#endif


int	MIDINoteToNotation(int noteNumber, char *note, int useFlats);
int NotationToMIDINote(char *note);

/* General MIDI Instrument List

PC#     Family                  PC#     Family
----    ------                  -----   ------
1-8     Piano                   65-72   Reed
9-16    Chromatic Percussion    73-80   Pipe
17-24   Organ                   81-88   Synth Lead
25-32   Guitar                  89-96   Synth Pad
33-40   Bass                    97-104  Synth Effects
41-48   Strings                 105-112 Ethnic
49-56   Ensemble                113-120 Percussive
57-64   Brass                   121-128 Sound Effects
*/

#define kGMIDI_001_AcousticGrandPiano   0
#define kGMIDI_002_BrightAcousticPiano  1
#define kGMIDI_003_ElectricGrandPiano   2
#define kGMIDI_004_HonkyTonkPiano       3
#define kGMIDI_005_ElectricPiano1       4
#define kGMIDI_006_ElectricPiano2       5
#define kGMIDI_007_Harpsichord          6
#define kGMIDI_008_Clavi                7
#define kGMIDI_009_Celesta              8
#define kGMIDI_010_Glockenspiel         9

#define kGMIDI_011_MusicBox             10
#define kGMIDI_012_Vibraphone           11
#define kGMIDI_013_Marimba              12
#define kGMIDI_014_Xylophone            13
#define kGMIDI_015_TubularBells         14
#define kGMIDI_016_Dulcimer             15
#define kGMIDI_017_DrawbarOrgan         16
#define kGMIDI_018_PercussiveOrgan      17
#define kGMIDI_019_RockOrgan            18
#define kGMIDI_020_ChurchOrgan          19

#define kGMIDI_021_ReedOrgan            20
#define kGMIDI_022_Accordion            21
#define kGMIDI_023_Harmonica            22
#define kGMIDI_024_TangoAccordion       23
#define kGMIDI_025_AcousticGuitar_Nylon 24
#define kGMIDI_026_AcousticGuitar_Steel 25
#define kGMIDI_027_ElectricGuitar_Jazz  26
#define kGMIDI_028_ElectricGuitar_Clean 27
#define kGMIDI_029_ElectricGuitar_Muted 28
#define kGMIDI_030_OverdrivenGuitar     29

#define kGMIDI_031_DistortionGuitar     30
#define kGMIDI_032_GuitarHarmonics      31
#define kGMIDI_033_AcousticBass         32
#define kGMIDI_034_ElectricBass_Finger  33
#define kGMIDI_035_ElectricBass_Pick    34
#define kGMIDI_036_FretlessBass         35
#define kGMIDI_037_SlapBass1            36
#define kGMIDI_038_SlapBass2            37
#define kGMIDI_039_SynthBass1           38
#define kGMIDI_040_SynthBass2           39

#define kGMIDI_041_Violin               40
#define kGMIDI_042_Viola                41
#define kGMIDI_043_Cello                42
#define kGMIDI_044_Contrabass           43
#define kGMIDI_045_TremoloStrings       44
#define kGMIDI_046_PizzicatoStrings     45
#define kGMIDI_047_OrchestralHarp       46
#define kGMIDI_048_Timpani              47
#define kGMIDI_049_StringEnsemble1      48
#define kGMIDI_050_StringEnsemble2      49

#define kGMIDI_051_SynthStrings1        50
#define kGMIDI_052_SynthStrings2        51
#define kGMIDI_053_ChoirAahs            52
#define kGMIDI_054_VoiceOohs            53
#define kGMIDI_055_SynthVoice           54
#define kGMIDI_056_OrchestraHit         55
#define kGMIDI_057_Trumpet              56
#define kGMIDI_058_Trombone             57
#define kGMIDI_059_Tuba                 58
#define kGMIDI_060_MutedTrumpet         59

#define kGMIDI_061_FrenchHorn           60
#define kGMIDI_062_BrassSection         61
#define kGMIDI_063_SynthBrass1          62
#define kGMIDI_064_SynthBrass2          63
#define kGMIDI_065_SopranoSax           64
#define kGMIDI_066_AltoSax              65
#define kGMIDI_067_TenorSax             66
#define kGMIDI_068_BaritoneSax          67
#define kGMIDI_069_Oboe                 68
#define kGMIDI_070_EnglishHorn          69

#define kGMIDI_071_Bassoon              70
#define kGMIDI_072_Clarinet             71
#define kGMIDI_073_Piccolo              72
#define kGMIDI_074_Flute                73
#define kGMIDI_075_Recorder             74
#define kGMIDI_076_PanFlute             75
#define kGMIDI_077_BlownBottle          76
#define kGMIDI_078_Shakuhachi           77
#define kGMIDI_079_Whistle              78
#define kGMIDI_080_Ocarina              79

#define kGMIDI_081_Lead1_Square         80
#define kGMIDI_082_Lead2_Sawtooth       81
#define kGMIDI_083_Lead3_Calliope       82
#define kGMIDI_084_Lead4_Chiff          83
#define kGMIDI_085_Lead5_Charang        84
#define kGMIDI_086_Lead6_Voice          85
#define kGMIDI_087_Lead7_Fifths         86
#define kGMIDI_088_Lead8_BassPlusLead   87
#define kGMIDI_089_Pad1_NewAge          88
#define kGMIDI_090_Pad2_Warm            89

#define kGMIDI_091_Pad3_Polysynth       90
#define kGMIDI_092_Pad4_Choir           91
#define kGMIDI_093_Pad5_Bowed           92
#define kGMIDI_094_Pad6_Metallic        93
#define kGMIDI_095_Pad7_Halo            94
#define kGMIDI_096_Pad8_Sweep           95
#define kGMIDI_097_FX1_Rain             96
#define kGMIDI_098_FX2_Soundtrack       97
#define kGMIDI_099_FX3_Crystal          98

#define kGMIDI_100_FX4_Atmosphere       99
#define kGMIDI_101_FX5_Brightness       100
#define kGMIDI_102_FX6_Goblins          101
#define kGMIDI_103_FX7_Echoes           102 
#define kGMIDI_104_FX8_SciFi            103
#define kGMIDI_105_Sitar                104
#define kGMIDI_106_Banjo                105
#define kGMIDI_107_Shamisen             106
#define kGMIDI_108_Koto                 107
#define kGMIDI_109_Kalimba              108

#define kGMIDI_110_BagPipe              109
#define kGMIDI_111_Fiddle               110
#define kGMIDI_112_Shanai               111
#define kGMIDI_113_TinkleBell           112
#define kGMIDI_114_Agogo                113
#define kGMIDI_115_SteelDrums           114
#define kGMIDI_116_Woodblock            115
#define kGMIDI_117_TaikoDrum            116
#define kGMIDI_118_MelodicTom           117
#define kGMIDI_119_SynthDrum            118

#define kGMIDI_120_ReverseCymbal        119
#define kGMIDI_121_GuitarFretNoise      120
#define kGMIDI_122_BreathNoise          121
#define kGMIDI_123_Seashore             122
#define kGMIDI_124_BirdTweet            123
#define kGMIDI_125_TelephoneRing        124
#define kGMIDI_126_Helicopter           125
#define kGMIDI_127_Applause             126
#define kGMIDI_128_Gunshot              127

char *GeneralMIDI_InstrumentNumberToName(int noteNumber);

#ifdef __cplusplus
}
#endif

#endif  //	__MIDI_H__
