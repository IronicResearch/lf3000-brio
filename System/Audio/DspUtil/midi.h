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


#ifdef __cplusplus
}
#endif

#endif  //	__MIDI_H__
