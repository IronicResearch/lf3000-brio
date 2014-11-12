export EXE='/home/lfu/workspace/Brio2/Lightning/Samples/PlayAudio/Build/Lightning_emulation/PlayAudio'
#export EXE=./PlayAudio
export FLAGS='-v -vol 100 -pan 0' 

export AF='/home/lfu/AudioFiles'
export MIDI=$AF/MIDI

# ------- MIDI Note Tests
# Command line arguments for MIDI Note Tests
# -midi_tempochange     [-128 .. 127]  mapped to [1/16x .. 16x]
# -midi_programnumber   [1..128]
# -midi_channel         [1..16]
# -midi_velocity        [1..127]
# -midi_duration_note   milliseconds, time between note on and off
# -midi_duration_space  millisecond, time between last note off and next note on
 
# RUN all of the MIDI Note Tests : SomeNotes, ProgramChange, Chromatic
#$EXE -midinotetest_all

# Some Notes:  Play a chord
#$EXE -midinotetest_SomeNotes

# Play some notes on each of 128 programs
#$EXE -midinotetest_ProgramChange

#  Play all 128 MIDI notes
#$EXE -v -midinotetest_Chromatic -midi_program 30 -midi_channel 2 -midi_duration_note 50 -midi_duration_space 0 -midi_velocity 120

# ---------- MIDI File Tests
# NOTE:  no programmability in these tests
# -midifiletest_All
# -midifiletest_TrackEnable
# -midifiletest_ChangeTempo 
# -midifiletest_Transpose

#$EXE -midifiletest_all $MIDI/Rossi/technojazz.mid

# Turn off all tracks, then enable one by one
#$EXE -midifiletest_TrackEnable $MIDI/Rossi/technojazz.mid

# Change Tempo : -64=1/8x, -32=1/4x, 0=1x, 32=4x, 64=8x
#$EXE -midifiletest_ChangeTempo $MIDI/Rossi/technojazz.mid

# Transpose :  from -24 to 24 in whole step increments every 4 seconds
$EXE -midifiletest_Transpose $MIDI/Rossi/technojazz.mid




