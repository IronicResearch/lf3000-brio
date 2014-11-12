export EXE='/home/lfu/workspace/Brio2/Lightning/Samples/PlayAudio/Build/Lightning_emulation/PlayAudio'
#export EXE=./PlayAudio
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

# Voice tests for SRC aliasing
#$EXE -i $GOLD16/A_AP.brio
#$EXE -i $GOLD16/A_DJ.brio
#$EXE -i $GOLD16/Sung_MultipleVoice.brio
#$EXE -i $GOLD16/Sung_kidhighvocal.brio
#$EXE -i $GOLD16/Sung_HiFem.brio
#$EXE -i $OGG/LightningTest_Voice_preproc_16_L2W.ogg

#$EXE -i $AF/out.brio

#$EXE -i $OGG/BreakItDown_32k_Q02_mono.ogg

#$EXE -i $OGG/BreakItDown_32k_Q02_mono.ogg
#$EXE -i $OGG/BreakItDown_32k_Q02_mono.ogg

#$EXE -i $OGG/BreakItDown_16k_Q02_st.ogg

# Basic stereo/format tests
#$EXE $FLAGS -i $MUSIC/Temptation_32k_mono.wav
#$EXE $FLAGS -i $MUSIC/Temptation_32k_st.wav
#$EXE $FLAGS -i $MUSIC/Temptation_16k_mono.wav
#$EXE $FLAGS -i $MUSIC/Temptation_16k_st.wav

#$EXE $FLAGS -i $OGG/BreakItDown_32k_Q02_mono.ogg
#$EXE $FLAGS -i $OGG/BreakItDown_32k_Q02_st.ogg

#$EXE $FLAGS -i $MUSIC/Temptation_32k_mono.brio
#$EXE $FLAGS -i $MUSIC/Temptation_32k_st.brio
#$EXE $FLAGS -pan -100 -i $MIDI/Rossi/technojazz.mid

# File format tests
#$EXE $FLAGS -l 3 -i $SFX/belltree_up2_32k.brio
#$EXE $FLAGS  -i $SFX/belltree_up2_32k.wav
#$EXE $FLAGS  -i $SFX/belltree_up2_32k.aif
#$EXE $FLAGS  -i $SFX/belltree_up2_32k.ogg
#$EXE $FLAGS -i $MIDI/play60.mid

#$EXE -i $SFX/belltree_up2_32k.brio -i $SFX/belltree_up2_32k.wav
#$EXE -i $SFX/belltree_up2_32k.aif -i $SFX/belltree_up2_32k.ogg
#$EXE -l 5 -i $PRO_SFX/bomber.wav
#$EXE -i $SFX/belltree_up2_32k.brio
#$EXE -l 3 -i $SFX/belltree_up2_32k.ogg
#$EXE -i $MIDI/Rossi/TRIBAL.MID
#$EXE -i $MIDI/play60.mid

#$EXE -l 3 -i $SFX/belltree_up2_32k.brio
#$EXE -loop 3 -i $SFX/belltree_up2_32k.wav

# Multiple file tests
#EXE -v -u -p -100 -p 100 -p 0 -i $SFX/belltree_up2_32k.brio -i $OGG/BreakItDown_32k_Q00_mono.ogg -i $MUSIC/Temptation_32k_mono.wav -i $GOLD16/A_DJ.brio -i $SINE/sine_dbM3_0500Hz_32k_c1.wav

# $PRO_AMB/crickets.wav

#$EXE -u -p -100 -p 100 -p 0 -i $SFX/belltree_up2_32k.brio $PRO_INSTR/metal_mute_power.E1.aiff $MUSIC/Temptation_32k_st.wav
#$EXE -v -p -100 -p 100 -p 0 -i $SFX/belltree_up2_32k.brio $PRO_INSTR/metal_mute_power.E1.aiff $SFX/belltree_up2_32k.brio

#$EXE -u -i $GOLD16/A_DJ.brio $MUSIC/Temptation_32k_st.wav
#$EXE -u -i $GOLD16/A_DJ.brio 
#$EXE -u -i $GOLD16/A_DJ_m0p5dB.aif

# Simultaneous sine test
#$EXE -p -100 -p 0 -p 100 -i $SINE/sine_dbM3_0250Hz_32k_c1.wav $SINE/SINE32.brio $SINE/sine_dbM3_0500Hz_32k_c1.wav
#$EXE -p 0 -i $SINE/sine_dbM3_0500Hz_32k_c1.wav 
#$EXE -u -i $SINE/sine_dbM3_0500Hz_32k_c1.wav $SINE/sine_dbM3_0500Hz_32k_c1.wav

# Done message tests
#$EXE $FLAGS -i $SFX/belltree_up2_32k.brio
#$EXE $FLAGS -sendDoneMsg -i $SFX/belltree_up2_32k.brio
#$EXE $FLAGS -sendDoneMsg -i $SFX/belltree_up2_32k.ogg
#$EXE $FLAGS             -i $SFX/belltree_up2_32k.brio
#$EXE $FLAGS             -i $SFX/belltree_up2_32k.ogg
#$EXE $FLAGS -i $MIDI/play60.mid

# Sine level test (for Soft Clipper)
#$EXE -p 0 -i $SINE/sine_db00_0500Hz_32k_c1.wav 
#$EXE -p 0 -i $SINE/sine_dbM3_0500Hz_32k_c1.wav 
#$EXE -p 0 -i $SINE/sine_dbM6_0500Hz_32k_c1.wav 

#$EXE -i $SINE/SINE32ST.WAV

# Seamless looping tests
#$EXE $FLAGS -l 3 -i $SINE/SINE32.ogg
#$EXE $FLAGS -l 3 -i $SINE/SINE32ST.ogg
#$EXE $FLAGS -l 3 -i $SFX/belltree_up2_32k.ogg

#$EXE $FLAGS -l 3 -i $SINE/SINE32.WAV
#$EXE $FLAGS -l 3 -i $SINE/SINE32ST.WAV
#$EXE $FLAGS -l 3 -i $SFX/belltree_up2_32k.wav

#$EXE $FLAGS -l 3 -i $SINE/SINE32.brio
#$EXE $FLAGS -l 3 -i $SINE/SINE32ST.brio
#$EXE $FLAGS -l 3 -i $SFX/belltree_up2_32k.brio

$EXE $FLAGS -l 3 -i $MIDI/play60.mid


# SRC 16->32 tests
#$EXE -i $SINE/sine_dbM3_0500Hz_16k_c1.wav 
#$EXE -i $SINE/sine_dbM3_0500Hz_16k_c1.wav 

# MIDI tests
#$EXE -i $MIDI/Rossi/technojazz.mid
#$EXE -i $MIDI/play60.mid
#$EXE -i $MIDI/Rossi/TRIBAL.MID
#$EXE -i $MUSIC/Temptation_16k_st.brio

# ----------------------- MIDI Note Tests
#$EXE -midinotetest_all
#$EXE -midinotetest_SomeNotes
#$EXE -midinotetest_ProgramChange
#$EXE -v -midinotetest_Chromatic

#$EXE -midinotetest_SomeNotes -midi_programnumber 30 -midi_duration_note 1000 -midi_duration_space 500
#$EXE -midinotetest_SomeNotes -midi_programnumber 30 -midi_duration_note  100 -midi_duration_space 2000
#$EXE -v -midinotetest_Chromatic -midi_programnumber 40 -midi_duration_note 100 -midi_duration_space 100

# ----------------------- MIDI Note Tests
#$EXE -midifiletest_all $MIDI/Rossi/technojazz.mid
#$EXE -midifiletest_TrackEnable $MIDI/Rossi/technojazz.mid
#$EXE -midifiletest_ChangeTempo $MIDI/Rossi/technojazz.mid
#$EXE -midifiletest_Transpose $MIDI/Rossi/technojazz.mid

# DURANDURAN/HungryLikeTheWolf.mid
# Nation.mid
# VICEDRUM.MID no 
#drums.mid
#RX11PT01.MID no
# Rossi/discovery.mid
# Rossi/discovery.mid


