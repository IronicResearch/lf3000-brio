# Input/Output gain tests
./sf2brio -outFormat WAV -inGain 0.25 SINE22.WAV outSINE22_InGain.wav
./sf2brio -outFormat WAV -inGainDB -12 SINE22.WAV outSINE22_InGainDB.wav

./sf2brio -outFormat WAV -outGain 0.5 SINE22.WAV outSINE22_OutGain.wav
./sf2brio -outFormat WAV -outGainDB -6 SINE22.WAV outSINE22_OutGainDB.wav

#cp out*_Unfiltered*.raw /mnt/hgfs/win_c/Tmp/SRC
