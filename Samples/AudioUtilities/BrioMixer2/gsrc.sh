export EXE='/home/lfu/workspace/Brio2/Lightning/Samples/BrioMixer/Build/Lightning_emulation/BrioMixer'
#export EXE=./BrioMixer
#export EXE=./Build/Lightning_emulation/BrioMixer
export FLAGS='-v -vol 100 -pan 0 -sct -lm' 

export AF='/home/lfu/AudioFiles'
export MUSIC=$AF/Music
export SFX=$AF/SFX
export SINE=$AF/SINE
export MIDI=$AF/MIDI
export GOLD16=$AF/GoldenTestSet_16k
export OGG=$AF/Ogg

export PROSONUS=$AF/Prosonus
export PRO_AMB=$PROSONUS/AMBIENCE
export PRO_INSTR=$PROSONUS/INSTR
export PRO_MUSIC=$PROSONUS/musictags
export PRO_SFX=$PROSONUS/SFX

#define kSRC_Interpolation_Type_AddDrop	   0
#define kSRC_Interpolation_Type_Linear	   1
#define kSRC_Interpolation_Type_FIR	       2
#define kSRC_Interpolation_Type_IIR	       3
#define kSRC_Interpolation_Type_Unfiltered 4
#define kSRC_Interpolation_Type_Triangle   5
#define kSRC_Interpolation_Type_Box	       6
# kSRC_FilterVersion // 6=30dB_15 7=50dB_31 8=50dB_41

$EXE $FLAGS -src_Type 1
#$EXE $FLAGS -src_Type 2 src_FilterVersion 6 -lm

#-i $MUSIC/Temptation_16k_st.brio

#      Looping DONE
#      -------- -------
# OGG            
# RAW            Y
# MIDI
