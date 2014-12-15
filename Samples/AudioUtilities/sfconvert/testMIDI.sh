# ../sfconvert -reps 10 -t -midi -voices 1 play20.mid play20.wav
#../sfconvert -reps 1 -t -midi -voices 16 -c 2 -fs 16000 -fs_midi 16000 mips16.mid mips16.wav

#../sfconvert -reps 1 -t -midi -voices 16 -c 2 -fs 32000 -fs_midi 16000 -fixedpt -srcType AddDrop mips16.mid mips16to32_AddDrop.wav

#../sfconvert -reps 1 -t -midi -voices 16 -c 2 -fs 32000 -fs_midi 16000 -fixedpt -srcType Linear mips16.mid mips16to32_Lineari.wav

../sfconvert -reps 1 -t -midi -voices 16 -c 2 -fs 32000 -fs_midi 16000 -srcType Linear mips16.mid mips16to32_Linearf.wav

../sfconvert -reps 1 -t -midi -voiceLimit 1 -c 1 -fs 32000 -fs_midi 16000 -srcType Linear mips16.mid mips01_16to32_Linearf_mono.wav
../sfconvert -reps 1 -t -midi -voiceLimit 1 -c 1 -fs 32000 -fs_midi 16000 -srcType Triangle mips16.mid mips01_16to32_Triangle_mono.wav
#../sfconvert -reps 1 -t -midi -voiceLimit 1 -c 2 -fs 32000 -fs_midi 16000 -srcType Linear mips16.mid mips01_16to32_Linearf_stereo.wav

#../sfconvert -reps 1 -t -midi -voices 16 -c 2 -fs 32000 -fs_midi 16000 -fixedpt -srcType FIR mips16.mid mips16to32_FIR.wav

#../sfconvert -reps 1 -t -midi -voices 16 -c 2 -fs 16000 -fs_midi 16000 -fixedpt Neutr_3_noDrums.mid Neutr_3_noDrums.wav

cp *.wav /mnt/hgfs/win_c/Tmp/Test_MIDI
