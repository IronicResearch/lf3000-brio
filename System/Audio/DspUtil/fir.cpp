// *************************************************************** 
// fir.cpp:		Code for Finite Impulse Response (FIR) filters
//
//		Written by Gints Klimanis, 2007
// ***************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "util.h"
#include "fir.h"

#define kFIR_Triangle_3_Length 3
float fir_Triangle_3_GainCompensationDB = 0.0;
float fir_Triangle_3_Hz[kFIR_Triangle_3_Length] = {
1.0f/2.0f,
1.0f/1.0f,
1.0f/2.0f,
};

#define kFIR_Triangle_9_Length 9
float fir_Triangle_9_GainCompensationDB = 0.0;
float fir_Triangle_9_Hz[kFIR_Triangle_9_Length] = {
1.0f/4.0f,
1.0f/3.0f,
1.0f/2.0f,
1.0f/1.0f,
1.0f/2.0f,
1.0f/3.0f,
1.0f/4.0f
};

// FIR low pass pseudo-Half Band filter
// Edges (normalized)       : 0...0.1904761905 , 0.275...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991
// Worst deviations     (dB): 0.3934868464, -26.6782971
float fir_HalfBand_15_GainCompensationDB = -0.393f;
float fir_HalfBand_15_Hz[kFIR_HalfBand_15_Hz_Length] = {
-1.966319704923826E-002,     
 3.222681179230342E-002,     
 4.623194395383842E-002,     
-2.482473304234042E-002,     
-8.977879949891251E-002,     
 3.260044908950705E-002,     
 3.132100525943124E-001,     
 4.663313867843080E-001,     
 3.132100525943124E-001,     
 3.260044908950705E-002,     
-8.977879949891251E-002,     
-2.482473304234042E-002,     
 4.623194395383842E-002,     
 3.222681179230342E-002,     
-1.966319704923826E-002      
};

// FIR lowpass filter -> pseudo half band filter
// Edges (normalized)       : 0...0.1791666667 , 0.2654761905...0.5 
// Specified deviations (dB): 0.4237859814 , -26.02059991
// Worst deviations     (dB): 0.02865429754, -49.67700052
// 31 coefficients: 
float fir_HalfBand_31_GainCompensationDB = -0.393f;
float fir_HalfBand_31_Hz[kFIR_HalfBand_31_Hz_Length] = {
 2.247704521932946E-003,     
 3.348591333106603E-003,     
-4.269349366420656E-003,     
-6.846542685131934E-003,     
 3.705493985646418E-003,     
 1.450129295379501E-002,     
-9.951944571807069E-005,     
-2.424019128960734E-002,     
-1.079184077189607E-002,     
 3.534300127151398E-002,     
 3.409931841349848E-002,     
-4.562321268940390E-002,     
-8.613741395861341E-002,     
 5.294673024837716E-002,     
 3.112456066215704E-001,     
 4.444013734618702E-001,     
 3.112456066215704E-001,     
 5.294673024837716E-002,     
-8.613741395861341E-002,     
-4.562321268940390E-002,     
 3.409931841349848E-002,     
 3.534300127151398E-002,     
-1.079184077189607E-002,     
-2.424019128960734E-002,     
-9.951944571807069E-005,     
 1.450129295379501E-002,     
 3.705493985646418E-003,     
-6.846542685131934E-003,     
-4.269349366420656E-003,     
 3.348591333106603E-003,     
 2.247704521932946E-003      
};

// FIR band pass filter 
// Edges (normalized)       : 0...0.125 , 0.1875...0.3125 , 0.375...0.5 
// Specified deviations (dB): -26.02059991, 0.4237859814, -26.02059991
// Worst deviations     (dB): -17.52475309, 1.083071175, -17.52475309
float fir_BandPass_GainCompensationDB = -0.393f;
float fir_BandPass_Hz[kFIR_BandPass_Hz_Length] = {
 1.054007141981783E-016,     
 4.626067510926039E-002,     
 3.284771181906916E-017,    
 1.250000000000000E-001,     
-9.783117603060870E-018,    
-2.962606751092606E-001,     
 4.717363652484429E-017,     
 3.828008305593115E-001,     
 4.717363652484429E-017,     
-2.962606751092606E-001,     
-9.783117603060870E-018,     
 1.250000000000000E-001,     
 3.284771181906916E-017,     
 4.626067510926039E-002,     
 1.054007141981783E-016      
};

// FIR band stop filter 
// Edges (normalized)       : 0...0.125 , 0.1875...0.3125 , 0.375...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991, 0.4237859814
// Worst deviations     (dB): 0.4433194267, -25.65037324, 0.4433194267
float fir_BandStop_GainCompensationDB = -0.444f;
float fir_BandStop_Hz[kFIR_BandStop_Hz_Length] = {
 5.808898833541933E-002,     
 4.784520410580150E-017,     
-2.745784022961546E-002,     
-1.922226675218418E-017,     
-1.060661931723916E-001,     
-3.953511333669201E-017,     
 2.916808937762975E-001,     
-1.505127721803406E-017,     
 6.195989697824791E-001,     
-1.505127721803406E-017,     
 2.916808937762975E-001,     
-3.953511333669201E-017,     
-1.060661931723916E-001,     
-1.922226675218418E-017,     
-2.745784022961546E-002,     
 4.784520410580150E-017,     
 5.808898833541933E-002      
};

// FIR bandstop filter 
// Edges (normalized)       : 0...0.05476190476, 0.09702380952...0.2446428571, 0.2738095238...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991, 0.4237859814
// Worst deviations     (dB): 1.868578976, -12.38890846, 1.871478139
float fir_Notch_GainCompensationDB = -1.87f;
float fir_Notch_Hz[kFIR_Notch_Hz_Length] = {
 1.64909722E-002,     
 6.49425240E-002,    
-7.91175331E-002,     
 8.01686616E-002,     
 2.36217196E-001,    
 1.62159823E-001,    
-1.73590635E-001,     
 6.25478722E-001,    
-1.73590635E-001,    
 1.62159823E-001,     
 2.36217196E-001,     
 8.01686616E-002,    
-7.91175331E-002,     
 6.49425240E-002,     
 1.64909722E-002      
};

// FIR highpass filter 
// Edges (normalized)       : 0...0.225 , 0.275...0.5 
// Specified deviations (dB): -26.02059991, 0.4237859814
// Worst deviations     (dB): -20.05716543, 0.8213132248
float fir_HighPass_GainCompensationDB = -0.822f;
float fir_HighPass_Hz[kFIR_HighPass_Hz_Length] = {
 7.170315600633288E-002,     
-1.318044505937485E-004,     
-5.754117820612641E-002,     
-1.028839231777762E-004,     
 1.020287792782554E-001,     
-8.722206843596569E-005,     
-3.170894032860515E-001,     
 4.998456180270049E-001,     
-3.170894032860515E-001,     
-8.722206843596569E-005,     
 1.020287792782554E-001,     
-1.028839231777762E-004,     
-5.754117820612641E-002,     
-1.318044505937485E-004,     
 7.170315600633288E-002      
};

// FIR lowpass filter 
// Edges (normalized)       : 0...0.004761904762 , 0.1267857143...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991
// Worst deviations     (dB): 0.04450512642, -43.69790572
// 15 coefficients: 
float fir_LowPass_GainCompensationDB = -0.045f;
float fir_LowPass_Hz[kFIR_LowPass_Hz_Length] = {
 1.012450863421694E-002,     
 2.135048804652548E-002,     
 3.902844636949630E-002,     
 6.074499819991502E-002,     
 8.367209352087522E-002,     
 1.040691000995740E-001,     
 1.181803454848006E-001,     
 1.232306918154969E-001,     
 1.181803454848006E-001,     
 1.040691000995740E-001,     
 8.367209352087522E-002,     
 6.074499819991502E-002,     
 3.902844636949630E-002,     
 2.135048804652548E-002,     
 1.012450863421694E-002      
};

// Low Pass FIR 1/2 Band filter
// Stop Band Ripple               : -32.8 dB
// Pass band as fraction of band  : 0.8
// Weight on transition band error: 1.0E-6
float fir_HalfBand_32dB_GainCompensationDB = 0.0f;
float fir_HalfBand_32dB_Hz[kFIR_HalfBand_32dB_Hz_Length] = {
     -0.025915889069,
      0.000000000000,
      0.043797262013,
      0.000000000000,
     -0.093198247254,
      0.000000000000,
      0.313836872578,
      0.500000000000,
      0.313836872578,
      0.000000000000,
     -0.093198247254,
      0.000000000000,
      0.043797262013,
      0.000000000000,
     -0.025915889069
};

// Low Pass FIR 1/2 Band filter
// Stop Band Ripple               : -58 dB
// Pass band as fraction of band  : 0.8
// Weight on transition band error: 1.0E-6
float fir_HalfBand_58dB_GainCompensationDB = 0.0f;
float fir_HalfBand_58dB_Hz[kFIR_HalfBand_58dB_Hz_Length] = {
     -0.002013340127,
      0.000000000000,
      0.004528076388,
      0.000000000000,
     -0.009264231659,
      0.000000000000,
      0.017020016909,
      0.000000000000,
     -0.029593331739,
      0.000000000000,
      0.051360368729,
      0.000000000000,
     -0.098308317363,
      0.000000000000,
      0.315639346838,
      0.500000000000,
      0.315639346838,
      0.000000000000,
     -0.098308317363,
      0.000000000000,
      0.051360368729,
      0.000000000000,
     -0.029593331739,
      0.000000000000,
      0.017020016909,
      0.000000000000,
     -0.009264231659,
      0.000000000000,
      0.004528076388,
      0.000000000000,
     -0.002013340127
};

// FIR low pass 1/3 Band filter
// Edges (normalized)       : 0...0.05476190476 , 0.1660714286...0.5 
// Specified deviations (dB): 0.4237859814, -26.02059991
// Worst deviations     (dB): 0.1664067777, -34.27288912
float fir_ThirdBand_15_GainCompensationDB = -0.43f;
float fir_ThirdBand_15_Hz[kFIR_ThirdBand_15_Hz_Length] = { 
-2.06583462E-002,     
-2.27924394E-002,     
-1.31107674E-002,     
 2.16633934E-002,     
 8.01341135E-002,     
 1.47992286E-001,     
 2.02693761E-001,     
 2.23676473E-001,     
 2.02693761E-001,     
 1.47992286E-001,     
 8.01341135E-002,    
 2.16633934E-002,     
-1.31107674E-002,     
-2.27924394E-002,     
-2.06583462E-002    
};
// FIR low pass 1/3 Band Filter
// Edges (normalized)       :  0...0.125  0.206547619...0.5 
// Specified deviations (dB):  0.4237859814 ,  -26.02059991
// Worst deviations     (dB):  0.04356962041 ,  -45.98843395
float fir_ThirdBand_31_GainCompensationDB = -0.044f;
float fir_ThirdBand_31_Hz[kFIR_ThirdBand_31_Hz_Length] = { 
-6.439899153609271E-004,     
 4.930494991348698E-003,    
 5.259429622388441E-003,     
-5.341869304173750E-004,     
-1.024559963005230E-002,     
-1.262301867027688E-002,     
 9.507424661052187E-004,     
 2.223353644409178E-002,     
 2.715711572640033E-002,     
-1.343170179396962E-003,     
-4.702534451436245E-002,     
-6.090389517417872E-002,     
 1.628546599067582E-003,     
 1.349405231328744E-001,     
 2.729190996458141E-001,     
 3.315991259913736E-001,     
 2.729190996458141E-001,     
 1.349405231328744E-001,     
 1.628546599067582E-003,     
-6.090389517417872E-002,     
-4.702534451436245E-002,     
-1.343170179396962E-003,     
 2.715711572640033E-002,     
 2.223353644409178E-002,   
 9.507424661052187E-004,     
-1.262301867027688E-002,     
-1.024559963005230E-002,     
-5.341869304173750E-004,     
 5.259429622388441E-003,     
 4.930494991348698E-003,     
-6.439899153609271E-004      
};

// FIR lowpass filter 
//  
// Edges (normalized)       : 0...0.1682795699 0.275...0.5 
// Specified deviations (dB): 0.4237859814 -26.02059991
// Worst deviations (dB)    : 0.2491639569-30.72535003
// 
//Target precision: fixed point 16 bit, normalized 
//15 coefficients: 
//#define kFIR_HalfBand_30dB_15_Hz_Length 15
float fir_HalfBand_30dB_15_GainCompensationDB = -0.44f;
float fir_HalfBand_30dB_15_Hz[kFIR_HalfBand_30dB_15_Hz_Length] = {
-1.58691406E-002,     /* h[0] */
 6.75659180E-002,     /* h[1] */
 6.19812012E-002,     /* h[2] */
-8.72802734E-002,     /* h[3] */
-1.77337646E-001,     /* h[4] */
 1.15447998E-001,     /* h[5] */
 6.94213867E-001,     /* h[6] */
 9.99969482E-001,     /* h[7] */
 6.94213867E-001,     /* h[8] */
 1.15447998E-001,     /* h[9] */
-1.77337646E-001,     /* h[10] */
-8.72802734E-002,     /* h[11] */
 6.19812012E-002,     /* h[12] */
 6.75659180E-002,     /* h[13] */
-1.58691406E-002      /* h[14] */
};

// FIR lowpass filter 
// Edges (normalized): 0...0.1768817204 0.2623655914...0.5 
//
// Specified deviations (dB): 0.4237859814 -26.02059991
// Worst deviations (dB):  0.02769387648  -49.85329895
//
// Target precision: fixed point 16 bit, normalized 
float fir_HalfBand_50dB_31_GainCompensationDB = -0.44f;
float fir_HalfBand_50dB_31_Hz[kFIR_HalfBand_50dB_31_Hz_Length] = {
//gain_t gain = 4.391659952401150E-001;
 6.866455078125000E-003,     /* h[0] */
 5.523681640625000E-003,     /* h[1] */
-1.208496093750000E-002,     /* h[2] */
-1.376342773437500E-002,     /* h[3] */
 1.266479492187500E-002,     /* h[4] */
 3.253173828125000E-002,     /* h[5] */
-7.019042968750000E-003,     /* h[6] */
-5.645751953125000E-002,     /* h[7] */
-1.690673828125000E-002,     /* h[8] */
 8.541870117187500E-002,     /* h[9] */
 6.985473632812500E-002,     /* h[10] */
-1.120605468750000E-001,     /* h[11] */
-1.906433105468750E-001,     /* h[12] */
 1.316528320312500E-001,     /* h[13] */
 7.065734863281250E-001,     /* h[14] */
 9.999694824218750E-001,     /* h[15] */
 7.065734863281250E-001,     /* h[16] */
 1.316528320312500E-001,     /* h[17] */
-1.906433105468750E-001,     /* h[18] */
-1.120605468750000E-001,     /* h[19] */
 6.985473632812500E-002,     /* h[20] */
 8.541870117187500E-002,     /* h[21] */
-1.690673828125000E-002,     /* h[22] */
-5.645751953125000E-002,     /* h[23] */
-7.019042968750000E-003,     /* h[24] */
 3.253173828125000E-002,     /* h[25] */
 1.266479492187500E-002,     /* h[26] */
-1.376342773437500E-002,     /* h[27] */
-1.208496093750000E-002,     /* h[28] */
 5.523681640625000E-003,     /* h[29] */
 6.866455078125000E-003      /* h[30] */
};

// Edges (normalized): 0...0.1962365591 0.2623655914...0.5 
// Specified deviations (dB): 0.4237859814-26.02059991 
// Worst deviations (dB): 0.02894625211-49.49195079
// 41 coefficients: 
float fir_HalfBand_50dB_41_GainCompensationDB = -0.44f;
float fir_HalfBand_50dB_41_Hz[kFIR_HalfBand_50dB_41_Hz_Length] = {
//gain_t gain = {4.587508394200423E-001
-2.227783203125000E-003,     /* h[0] */
 5.981445312500000E-003,     /* h[1] */
 5.950927734375000E-003,     /* h[2] */
-5.554199218750000E-003,     /* h[3] */
-1.101684570312500E-002,     /* h[4] */
 5.798339843750000E-003,     /* h[5] */
 1.940917968750000E-002,     /* h[6] */
-2.777099609375000E-003,     /* h[7] */
-3.027343750000000E-002,     /* h[8] */
-5.371093750000000E-003,     /* h[9] */
 4.306030273437500E-002,     /* h[10] */
 2.133178710937500E-002,     /* h[11] */
-5.667114257812500E-002,     /* h[12] */
-4.949951171875000E-002,     /* h[13] */
 6.964111328125000E-002,     /* h[14] */
 9.954833984375000E-002,     /* h[15] */
-8.035278320312500E-002,     /* h[16] */
-2.060852050781250E-001,     /* h[17] */
 8.746337890625000E-002,     /* h[18] */
 6.851806640625000E-001,     /* h[19] */
 9.999694824218750E-001,     /* h[20] */
 6.851806640625000E-001,     /* h[21] */
 8.746337890625000E-002,     /* h[22] */
-2.060852050781250E-001,     /* h[23] */
-8.035278320312500E-002,     /* h[24] */
 9.954833984375000E-002,     /* h[25] */
 6.964111328125000E-002,     /* h[26] */
-4.949951171875000E-002,     /* h[27] */
-5.667114257812500E-002,     /* h[28] */
 2.133178710937500E-002,     /* h[29] */
 4.306030273437500E-002,     /* h[30] */
-5.371093750000000E-003,     /* h[31] */
-3.027343750000000E-002,     /* h[32] */
-2.777099609375000E-003,     /* h[33] */
 1.940917968750000E-002,     /* h[34] */
 5.798339843750000E-003,     /* h[35] */
-1.101684570312500E-002,     /* h[36] */
-5.554199218750000E-003,     /* h[37] */
 5.950927734375000E-003,     /* h[38] */
 5.981445312500000E-003,     /* h[39] */
-2.227783203125000E-003      /* h[40] */
};

// *************************************************************** 
// DefaultFIR:		Set Default high-level parameter values
// ***************************************************************
    void 
DefaultFIR(FIR *d)
{
//long i = 0;

d->type = kFIR_Type_LowPass;
d->outLevelDB = 0.0f;
d->useFixedPoint = False;

d->samplingFrequency = 1.0f;
}	// ---- end DefaultFIR() ---- 

// *************************************************************** 
// UpdateFIR:	Convert high-level parameter values to low level data
// ***************************************************************
    void 
UpdateFIR(FIR *d)
{
long i = 0;

switch (d->type)
	{
	default:
	case kFIR_Type_LowPass:
		d->order      = kFIR_LowPass_Hz_Length;
		d->outLevelDB = fir_LowPass_GainCompensationDB;
		CopyFloats(fir_LowPass_Hz, d->h, d->order);
	break;
	case kFIR_Type_HighPass:
		d->order      = kFIR_HighPass_Hz_Length;
		d->outLevelDB = fir_HighPass_GainCompensationDB;
		CopyFloats(fir_HighPass_Hz, d->h, d->order);
	break;
	case kFIR_Type_BandPass:
		d->order      = kFIR_BandPass_Hz_Length;
		d->outLevelDB = fir_BandPass_GainCompensationDB;
		CopyFloats(fir_BandPass_Hz, d->h, d->order);
	break;
	case kFIR_Type_BandStop:
		d->order     =  kFIR_BandStop_Hz_Length;
		d->outLevelDB = fir_BandStop_GainCompensationDB;
		CopyFloats(fir_BandStop_Hz, d->h, d->order);
	break;
// 1/2 Band filters (fc = fs/4)
	case kFIR_Type_HalfBand_32dB:
		d->order      = kFIR_HalfBand_32dB_Hz_Length;
		d->outLevelDB = fir_HalfBand_32dB_GainCompensationDB;
		CopyFloats(fir_HalfBand_32dB_Hz, d->h, d->order);
	break;
	case kFIR_Type_HalfBand_58dB:
		d->order      = kFIR_HalfBand_58dB_Hz_Length;
		d->outLevelDB = fir_HalfBand_58dB_GainCompensationDB;
		CopyFloats(fir_HalfBand_58dB_Hz, d->h, d->order);
	break;
	case kFIR_Type_HalfBand_15:
		d->order      = kFIR_HalfBand_15_Hz_Length;
		d->outLevelDB = fir_HalfBand_15_GainCompensationDB;
		CopyFloats(fir_HalfBand_15_Hz, d->h, d->order);
	break;
	case kFIR_Type_HalfBand_31:
		d->order      = kFIR_HalfBand_31_Hz_Length;
		d->outLevelDB = fir_HalfBand_31_GainCompensationDB;
		CopyFloats(fir_HalfBand_31_Hz, d->h, d->order);
	break;
    case kFIR_Type_HalfBand_30dB_15:
		d->order      = kFIR_HalfBand_30dB_15_Hz_Length;
		d->outLevelDB = fir_HalfBand_30dB_15_GainCompensationDB;
		CopyFloats(fir_HalfBand_30dB_15_Hz, d->h, d->order);
printf("kFIR_Type_HalfBand_30dB_15: order=%d \n", d->order);
    break;
    case kFIR_Type_HalfBand_50dB_31:
		d->order      = kFIR_HalfBand_50dB_31_Hz_Length;
		d->outLevelDB = fir_HalfBand_50dB_31_GainCompensationDB;
		CopyFloats(fir_HalfBand_50dB_31_Hz, d->h, d->order);
printf("kFIR_Type_HalfBand_50dB_31: order=%d \n", d->order);
    break;
    case kFIR_Type_HalfBand_50dB_41:
		d->order      = kFIR_HalfBand_50dB_41_Hz_Length;
		d->outLevelDB = fir_HalfBand_50dB_41_GainCompensationDB;
		CopyFloats(fir_HalfBand_50dB_41_Hz, d->h, d->order);
printf("kFIR_Type_HalfBand_50dB_41: order=%d \n", d->order);
    break;

// 1/3 Band filters (fc = fs/6)
	case kFIR_Type_ThirdBand_15:
		d->order      = kFIR_ThirdBand_15_Hz_Length;
		d->outLevelDB = fir_ThirdBand_15_GainCompensationDB;
		CopyFloats(fir_ThirdBand_15_Hz, d->h, d->order);
	break;
	case kFIR_Type_ThirdBand_31:
		d->order      = kFIR_ThirdBand_31_Hz_Length;
		d->outLevelDB = fir_ThirdBand_31_GainCompensationDB;
		CopyFloats(fir_ThirdBand_31_Hz, d->h, d->order);
	break;
// Triangle filters
	case kFIR_Type_Triangle_3:
		d->order      = kFIR_Triangle_3_Hz_Length;
		d->outLevelDB = fir_Triangle_3_GainCompensationDB;
		CopyFloats(fir_Triangle_3_Hz, d->h, d->order);
	break;
	case kFIR_Type_Triangle_9:
		d->order      = kFIR_Triangle_9_Hz_Length;
		d->outLevelDB = fir_Triangle_9_GainCompensationDB;
		CopyFloats(fir_Triangle_9_Hz, d->h, d->order);
	break;

	}
d->outLevel = DecibelToLinear(d->outLevelDB);

// Scale coefficients by output level
for (i = 0; i < d->order; i++)
	d->h[i] *= d->outLevel;

// Convert 32-bit floating point coefficients to signed 16-bit fixed point
for (i = 0; i < d->order; i++)
	{
	d->hI[i] = (short)(k2To15m1f * d->h[i]);
//	printf("UpdateFIR  hI[%2d] = %d <- %g \n", i, d->hI[i], d->h[i]);
	}

//printf("UpdateFIR: outLevelDB = %g -> %g \n", d->outLevelDB, d->outLevel);
}	// ---- end UpdateFIR() ---- 

// *************************************************************** 
// ResetFIR:	Reset unit to initial state
// ***************************************************************
    void 
ResetFIR(FIR *d)
{
long i;

for (i = 0; i < kFIR_MaxDelayElements; i++)
	d->z[i] = 0;
}	// ---- end ResetFIR() ---- 

// *************************************************************** 
// PrepareFIR:	Update() + Reset()
// ***************************************************************
    void 
PrepareFIR(FIR *d)
{
UpdateFIR(d);
ResetFIR(d);
}	// ---- end PrepareFIR() ---- 

// *************************************************************** 
// RunFIR_Shortsf:	Main calculation routine, 32-bit floating point
// ***************************************************************
	void
RunFIR_Shortsf(short *in, short *out, long length, FIR *d)
{
long    i, j;
long lAcc;

//{static long c=0; printf("RunFIR_Shortsf: %d \n", c++);}

for (i = 0; i < length; i++)
	{
	float acc = 0.0f;
        short *zP = &in[i-(d->order-1)];
// Compute FIR filter 
        for (j = 0; j < d->order; j++)
		acc += d->h[j]*(float)zP[j];

// Saturate and output:  
	lAcc = (long) acc;
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->order; i++)
	in[-1-i] = in[length-1-i];
}	// ---- end RunFIR_Shortsf() ---- 

// *************************************************************** 
// RunFIR_Shortsi:	Main calculation routine, 16-bit fixed point
// ***************************************************************
	void
RunFIR_Shortsi(short *in, short *out, long length, FIR *d)
{
long i;
short *hI = d->hI;
//{static long c=0; printf("RunFIR_Shortsi %d \n", c++);}

for (i = 0; i < length; i++)
	{
	long lAcc = 0;

        short *zP = &in[i-(d->order-1)];
// Compute FIR filter 
        for (long j = 0; j < d->order; j++)
		lAcc += (long)(hI[j]*zP[j]);

// Saturate and output:  FIXXX add guard bits to accumulator, then add saturation code
	lAcc = lAcc>>15;  // instead of 16 for guard bits, TEMPORARY
	SATURATE_16BIT(lAcc);  // Macro but no return value
	out[i] = (short) lAcc;
	}

// Save end of buffer to start of buffer for next iteration
for (i = 0; i < d->order; i++)
	in[-1-i] = in[length-1-i];
}	// ---- end RunFIR_Shortsi() ---- 

// *************************************************************** 
// RunFIR_Shorts:	Main calculation routine
// ***************************************************************
	void
RunFIR_Shorts(short *inP, short *outP, long length, FIR *d)
{
if (d->useFixedPoint)
	RunFIR_Shortsi(inP, outP, length, d);
else
	RunFIR_Shortsf(inP, outP, length, d);
}	// ---- end RunFIR_Shorts() ---- 


