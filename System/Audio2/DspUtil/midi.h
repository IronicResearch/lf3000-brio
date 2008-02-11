// *************************************************************************
//
// midi.h Header file for MIDI functions
//
//				Written by Gints Klimanis
// *************************************************************************

#ifndef __MIDI_H__
#define	__MIDI_H__

#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define kMIDI_Cm1	0
#define kMIDI_Dbm1	1
#define kMIDI_Dm1	2
#define kMIDI_Ebm1	3
#define kMIDI_Em1	4
#define kMIDI_Fm1	5
#define kMIDI_Gbm1	6
#define kMIDI_Gm1	7
#define kMIDI_Abm1	8
#define kMIDI_Am1	9
#define kMIDI_Bbm1	10
#define kMIDI_Bm1	11
#define kMIDI_C0	12
#define kMIDI_Db0	13
#define kMIDI_D0	14
#define kMIDI_Eb0	15
#define kMIDI_E0	16
#define kMIDI_F0	17
#define kMIDI_Gb0	18
#define kMIDI_G0	19
#define kMIDI_Ab0	20
#define kMIDI_A0	21
#define kMIDI_Bb0	22
#define kMIDI_B0	23
#define kMIDI_C1	24
#define kMIDI_Db1	25
#define kMIDI_D1	26
#define kMIDI_Eb1	27
#define kMIDI_E1	28
#define kMIDI_F1	29
#define kMIDI_Gb1	30
#define kMIDI_G1	31
#define kMIDI_Ab1	32
#define kMIDI_A1	33
#define kMIDI_Bb1	34
#define kMIDI_B1	35
#define kMIDI_C2	36
#define kMIDI_Db2	37
#define kMIDI_D2	38
#define kMIDI_Eb2	39
#define kMIDI_E2	40
#define kMIDI_F2	41
#define kMIDI_Gb2	42
#define kMIDI_G2	43
#define kMIDI_Ab2	44
#define kMIDI_A2	45
#define kMIDI_Bb2	46
#define kMIDI_B2	47
#define kMIDI_C3	48
#define kMIDI_Db3	49
#define kMIDI_D3	50
#define kMIDI_Eb3	51
#define kMIDI_E3	52
#define kMIDI_F3	53
#define kMIDI_Gb3	54
#define kMIDI_G3	55
#define kMIDI_Ab3	56
#define kMIDI_A3	57
#define kMIDI_Bb3	58
#define kMIDI_B3	59
#define kMIDI_C4	60
#define kMIDI_Db4	61
#define kMIDI_D4	62
#define kMIDI_Eb4	63
#define kMIDI_E4	64
#define kMIDI_F4	65
#define kMIDI_Gb4	66
#define kMIDI_G4	67
#define kMIDI_Ab4	68
#define kMIDI_A4	69
#define kMIDI_Bb4	70
#define kMIDI_B4	71
#define kMIDI_C5	72
#define kMIDI_Db5	73
#define kMIDI_D5	74
#define kMIDI_Eb5	75
#define kMIDI_E5	76
#define kMIDI_F5	77
#define kMIDI_Gb5	78
#define kMIDI_G5	79
#define kMIDI_Ab5	80
#define kMIDI_A5	81
#define kMIDI_Bb5	82
#define kMIDI_B5	83
#define kMIDI_C6	84
#define kMIDI_Db6	85
#define kMIDI_D6	86
#define kMIDI_Eb6	87
#define kMIDI_E6	88
#define kMIDI_F6	89
#define kMIDI_Gb6	90
#define kMIDI_G6	91
#define kMIDI_Ab6	92
#define kMIDI_A6	93
#define kMIDI_Bb6	94
#define kMIDI_B6	95
#define kMIDI_C7	96
#define kMIDI_Db7	97
#define kMIDI_D7	98
#define kMIDI_Eb7	99
#define kMIDI_E7	100
#define kMIDI_F7	101
#define kMIDI_Gb7	102
#define kMIDI_G7	103
#define kMIDI_Ab7	104
#define kMIDI_A7	105
#define kMIDI_Bb7	106
#define kMIDI_B7	107
#define kMIDI_C8	108
#define kMIDI_Db8	109
#define kMIDI_D8	110
#define kMIDI_Eb8	111
#define kMIDI_E8	112
#define kMIDI_F8	113
#define kMIDI_Gb8	114
#define kMIDI_G8	115
#define kMIDI_Ab8	116
#define kMIDI_A8	117
#define kMIDI_Bb8	118
#define kMIDI_B8	119
#define kMIDI_C9	120
#define kMIDI_Db9	121
#define kMIDI_D9	122
#define kMIDI_Eb9	123
#define kMIDI_E9	124
#define kMIDI_F9	125
#define kMIDI_Gb9	126
#define kMIDI_G9	127
// Above MIDI range 
#define kMIDI_Ab9	128
#define kMIDI_A9	129
#define kMIDI_Bb9	130
#define kMIDI_B9	131
#define kMIDI_C10	132
#define kMIDI_Db10	133
#define kMIDI_D10	134
#define kMIDI_Eb10	135
#define kMIDI_E10	136
#define kMIDI_F10	137
#define kMIDI_Gb10	138
#define kMIDI_G10	139

int	MIDINoteToNotation(int noteNumber, char *note, int useFlats);
int NotationToMIDINote(char *note);

// MIDI Channel Message
// Upper Nibble: Message ID  
// Lower Nibble: Channel # [0..15]
#define kMIDI_CHANNELMESSAGE_NOTEOFF          0x80
#define kMIDI_CHANNELMESSAGE_NOTEON           0x90
#define kMIDI_CHANNELMESSAGE_AFTERTOUCH       0xA0
#define kMIDI_CHANNELMESSAGE_CONTROLCHANGE    0xB0
#define kMIDI_CHANNELMESSAGE_PROGRAMCHANGE    0xC0
#define kMIDI_CHANNELMESSAGE_CHANNELPRESSURE  0xD0
#define kMIDI_CHANNELMESSAGE_PITCHWHEEL       0xE0

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
