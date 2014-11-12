#!/bin/sh

echo "Ogg encode with Quality=5"

# BBC sound file, originally at some weird 11111 Hz sampling frequency
oggenc -q 5 noise16_30.wav -o "Q5/noise16_30_Q5.ogg"
oggenc -q 5 noise22_30.wav -o "Q5/noise22_30_Q5.ogg"
oggenc -q 5 noise44_30.wav -o "Q5/noise44_30_Q5.ogg"
