#!/bin/sh

echo "Ogg encode with Quality=5"

# BBC sound file, originally at some weird 11111 Hz sampling frequency
oggenc -q 5 BBC_11_mono.wav -o "Q5/BBC_11_mono_Q5.ogg"
oggenc -q 5 BBC_16_mono.wav -o "Q5/BBC_16_mono_Q5.ogg"
oggenc -q 5 BBC_22_mono.wav -o "Q5/BBC_22_mono_Q5.ogg"
oggenc -q 5 BBC_32_mono.wav -o "Q5/BBC_32_mono_Q5.ogg"
oggenc -q 5 BBC_44_mono.wav -o "Q5/BBC_44_mono_Q5.ogg"

oggenc -q 2 BBC_16_mono.wav -o "Q2/BBC_16_mono_Q2.ogg"
oggenc -q 4 BBC_16_mono.wav -o "Q4/BBC_16_mono_Q4.ogg"


