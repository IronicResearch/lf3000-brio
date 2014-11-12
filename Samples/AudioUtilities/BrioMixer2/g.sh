export EXE='/home/lfu/workspace/Brio2/System/Audio/Utilities/BrioMixer/Build/Lightning_emulation/BrioMixer'
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

# TEST Done Message
#$EXE $FLAGS -debug_audiodonemsg -loopoff -donemsgoff
#$EXE $FLAGS -debug_audiodonemsg -loopoff -donemsgon
#$EXE $FLAGS -debug_audiodonemsg -loopon  -donemsgoff -c 3
#  Special case  -> core dumps (WAV only files)
#$EXE $FLAGS -debug_audiodonemsg -loopon  -donemsgon

# TEST AudioMPI
#echo "Test Audio MPI IsPlaying"
#$EXE $FLAGS -test_AudioMPI_IsPlaying
echo "Test Audio MPI IsPlaying"
$EXE $FLAGS -test_AudioMPI_donemsg  -donemsgon

# TEST LoopEnd Message
echo "Test LoopEndMessage"
#$EXE $FLAGS -testdonemsg  -donemsgon

#$EXE $FLAGS -loopcount 3 -loopEndMsgOn  -donemsgoff
#$EXE $FLAGS -loopcount 3 -loopEndMsgOn -donemsgon 
#-i $SFX/glass_break_32k_c2.wav
#$EXE $FLAGS -loopcount 3 -loopEndMsgOn  -donemsgoff -inL SFX/belltree_up2_32k.ogg
#$EXE $FLAGS -loopcount 3 -loopEndMsgOn -donemsgon 
$EXE $FLAGS -loopcount 3 -loopEndMsgOn  -donemsgoff 
#-inL SFX/belltree_up2_32k.ogg

# Seamless looping tests
#$EXE $FLAGS -loopcount 3 -i $SINE/SINE32.ogg
#$EXE $FLAGS -loopcount 3 -i $SINE/SINE32ST.ogg
#$EXE $FLAGS -loopcount 3 -i $SFX/belltree_up2_32k.ogg

#$EXE $FLAGS -loopcount 3 -i $SINE/SINE32.WAV
#$EXE $FLAGS -loopcount 3 -i $SINE/SINE32ST.WAV
#$EXE $FLAGS -loopcount 3 -i $SFX/glass_break_32k_c2.wav

# TEST LoopEnd Message
#$EXE $FLAGS -loopcount 3 -loopEndMsgOn -donemsgoff
#$EXE $FLAGS -loopcount 3 -loopEndMsgOff -donemsgon



#-i $MUSIC/Temptation_16k_st.brio
#      Looping DONE
#      ------- -------
# OGG            
# RAW            Y
# MIDI

#$EXE $FLAGS -sc -lm -o Joe.wav

# ------------------- UNCOMPRESSED TESTS ---------------------
#$EXE $FLAGS -inA UncompressedSample_08k.wav -inB UncompressedSample_16k.wav -outfile Test.wav -outfiletime 10
#$EXE $FLAGS -inA sine_dbM3_1000Hz_08k_c1_zc.wav -inB sine_dbM3_1000Hz_16k_c1_zc.wav -outfile Test.wav -outfiletime 10
#$EXE $FLAGS -lm -inA sine_db00_1000Hz_08k_c1_zc.wav -inB sine_db00_1000Hz_16k_c1_zc.wav -inL sine_db00_1000Hz_32k_c1_zc.wav -outfile Test.wav -outfiletime 10
#$EXE $FLAGS -lm -inA sine_db00_1000Hz_08k_c2_zc.wav -inB sine_db00_1000Hz_16k_c2_zc.wav -inL sine_db00_1000Hz_32k_c2_zc.wav -outfile Test.wav -outfiletime 10
#$EXE $FLAGS -lm -inA sine_db00_1000Hz_08k_c1_Ramp.wav -inB sine_db00_1000Hz_16k_c1_Ramp.wav -inL sine_db00_1000Hz_32k_c1_Ramp.wav -outfile Test.wav -outfiletime 10
#$EXE $FLAGS -lm -inA Impulse_08k_c1.wav -inB Impulse_16k_c1.wav -inL Impulse_32k_c1.wav -outfile Test.wav -outfiletime 10

# ----------------- OGG TESTS ------------------------
#$EXE $FLAGS -lm -inA ConnectMovie2.aogg -inB Impulse_16k_c1.wav -inL Impulse_32k_c1.wav -outfile Test.wav -outfiletime 10

# ----------------- SOFT CLIPPER TESTS ----------------------
#$EXE $FLAGS -sc -lm -inA UncompressedCut_16k_c1.wav -inB sine_db00_1000Hz_16k_c1_Ramp.wav -inL sine_db00_1000Hz_32k_c1_Ramp.wav -inR Temptation_32k_st.wav 
#-outfile Test.wav -outfiletime 10

#$EXE $FLAGS -sc -lm  sine_db00_1000Hz_08k_c1.wav -inB sine_db00_1000Hz_16k_c1.wav -inL sine_db00_1000Hz_32k_c1.wav -outfile Test.wav -outfiletime 10

#$EXE $FLAGS -sc -lm -inA Square_08k_c1.wav -inB Square_16k_c1.wav -inL Square_32k_c1.wav -outfile Test.wav -outfiletime 6
#$EXE $FLAGS -sc -lm -inA Sawtooth_08k_c1.wav -inB Sawtooth_16k_c1.wav -inL Sawtooth_32k_c1.wav -outfile Test.wav -outfiletime 6

#$EXE $FLAGS -sc -lm -inA sine_dbM6_1000Hz_32k_c1_zc.wav -inB sine_dbM3_1000Hz_32k_c1_zc.wav -inL sine_db00_1000Hz_32k_c1_zc.wav -inR UncompressedCut_32k_c1.wav -inHome "play20.mid" -outfile Test.wav -outfiletime 10
#$EXE $FLAGS -sc -lm -inA Saw_32k_m6dB_m.wav -inB Saw_32k_m3dB_m.wav -inL Saw_32k_m0dB_m.wav -inR UncompressedCut_32k_c1.wav -inHome "play60.mid" 
#-sc_PreGainDB 0
#-outfile Test.wav -outfiletime 6

#$EXE $FLAGS -sc -lm -inA LeftRightCenter_32k_c2.wav -inL Saw_32k_m0dB_m.wav -inR UncompressedCut_32k_c1.wav -inHome "play60.mid" -sc_PreGainDB 0

# ---------------- MIDI TEST --------------------------------
#$EXE $FLAGS -lm -rsm -inA Saw_32k_m6dB_m.wav -inB Saw_32k_m3dB_m.wav -inL Saw_32k_m0dB_m.wav -inR UncompressedCut_32k_c1.wav -inHome "play60.mid" -outfile Test.wav -outfiletime 6


