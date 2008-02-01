export EXE='/home/lfu/workspace/Brio2/Lightning/Samples/BrioMixer/Build/Lightning_emulation/BrioMixer'
#export EXE=./BrioMixer
#export EXE=./Build/Lightning_emulation/BrioMixer
export FLAGS='-v -vol 100 -pan 0' 

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

#$EXE $FLAGS -sc 
$EXE $FLAGS -sc -outfile Clip.wav -outfiletime 20 -sc_PreGainDB 0 -sc_PostGainDB 0

