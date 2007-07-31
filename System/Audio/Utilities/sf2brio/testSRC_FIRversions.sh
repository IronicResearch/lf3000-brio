
# FIR Resampling by 2 tests
#./sfconvert -outFormat wav -fixedpt -fs 44100 -srcType FIR SINE22.WAV outSineBy2_22to44k_FIRi.wav
#cp outSineBy2*.* /mnt/hgfs/win_c/Tmp/SRC

#./sfconvert -outFormat wav -fixedpt -fs 44100 -srcType FIR belltree_up2_22k.wav outBellBy2_22to44k_FIRi.wav
#./sfconvert -outFormat wav -fixedpt -fs 44100 -srcType AddDrop belltree_up2_22k.wav outBellBy2_22to44k_AddDropi.wav
#./sfconvert -outFormat wav -fixedpt -fs 44100 -srcType Linear belltree_up2_22k.wav outBellBy2_22to44k_Lineari.wav
#./sfconvert -outFormat wav -fixedpt -fs 22050 -srcType FIR belltree_up2_11k.wav outBellBy2_11to22k_FIRi.wav
#cp outBell*.wav /mnt/hgfs/win_c/Tmp/SRC

#./sfconvert -outFormat wav -fixedpt -fs 44100 -srcType AddDrop Temptation_22k_mono.wav outTemptation_22to44k_AddDropi.wav
#./sfconvert -outFormat wav -fixedpt -fs 44100 -srcType Linear Temptation_22k_mono.wav outTemptation_22to44k_Lineari.wav
#./sfconvert -outFormat wav -fixedpt -fs 44100 -srcType FIR Temptation_22k_mono.wav outTemptation_22to44k_FIRi.wav
#cp outTempt*.wav /mnt/hgfs/win_c/Tmp/SRC


#./sfconvert -outFormat wav -fixedpt -fs 32000 -srcType AddDrop Neutr_3_noDrums.wav outNeutr_3_noDrums_16to32k_AddDropi.wav
#./sfconvert -outFormat wav -fixedpt -fs 32000 -srcType Linear Neutr_3_noDrums.wav outNeutr_3_noDrums_16to32k_Lineari.wav
#./sfconvert -outFormat wav -fixedpt -fs 32000 -srcType FIR Neutr_3_noDrums.wav outNeutr_3_noDrums_16to32k_FIRi.wav
#cp *Neutr*.wav /mnt/hgfs/win_c/Tmp/SRC

#./sfconvert -fixedpt -fs 32000 -srcType AddDrop noise05_16k.WAV outNoise05_16to32k_AddDropi.wav
#./sfconvert -fixedpt -fs 32000 -srcType Linear noise05_16k.WAV  outNoise05_16to32k_Lineari.wav
#./sfconvert -fixedpt -fs 32000 -srcType FIR noise05_16k.WAV     outNoise05_16to32k_FIRi.wav
#cp *oise05_16*.wav /mnt/hgfs/win_c/Tmp/SRC

./sfconvert -fixedpt -fs 32000 -srcType AddDrop SINE16.WAV outSINE16to32k_AddDropi.wav
./sfconvert -fixedpt -fs 32000 -srcType Linear  SINE16.WAV outSINE16to32k_Lineari.wav
./sfconvert -fixedpt -fs 32000 -srcType FIR     SINE16.WAV outSINE16to32k_FIRi.wav
./sfconvert          -fs 32000 -srcType AddDrop SINE16.WAV outSINE16to32k_AddDropf.wav
./sfconvert          -fs 32000 -srcType Linear  SINE16.WAV outSINE16to32k_Linearf.wav
./sfconvert          -fs 32000 -srcType Triangle SINE16.WAV outSINE16to32k_Triangle.wav
./sfconvert          -fs 32000 -srcType FIR     SINE16.WAV outSINE16to32k_FIRf.wav

./sfconvert -fixedpt -fs 32000 -srcType AddDrop SINE16ST.WAV outSINE16to32k_AddDrop_STEREOi.wav
./sfconvert -fixedpt -fs 32000 -srcType Linear  SINE16ST.WAV outSINE16to32k_Linear_STEREOi.wav
./sfconvert -fixedpt -fs 32000 -srcType Triangle SINE16ST.WAV outSINE16to32k_Triangle_STEREOi.wav
./sfconvert -fixedpt -fs 32000 -srcType FIR  SINE16ST.WAV outSINE16to32k_FIR_STEREOi.wav

cp *SINE16*.wav /mnt/hgfs/win_c/Tmp/SRC
cp SINE16.WAV /mnt/hgfs/win_c/Tmp/SRC
