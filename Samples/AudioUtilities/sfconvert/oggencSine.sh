#!/bin/sh

echo "Ogg encode: many just don't encode"

#oggenc noise05_08k.wav -m  64 -M  64 -o "NOISE/noise05_08k_064kb.ogg"

#cp noise05_08k.wav NOISE/noise05_08k.wav

oggenc SINE08.WAV -q 5 -o "SINE/SINE08k_Q5.ogg"
oggenc SINE16.WAV -q 5 -o "SINE/SINE16k_Q5.ogg"
oggenc SINE32.WAV -q 5 -o "SINE/SINE32k_Q5.ogg"

#sf2brio -outFormat brio noise05_08k.wav NOISE/noise05_08k.brio

oggenc SINE/sine_db0_1000Hz_08k.wav -q 5 -o "SINE/sine_db0_1000Hz_08k.ogg"
oggenc SINE/sine_db0_1000Hz_16k.wav -q 5 -o "SINE/sine_db0_1000Hz_16k.ogg"
oggenc SINE/sine_db0_1000Hz_32k.wav -q 5 -o "SINE/sine_db0_1000Hz_32k.ogg"

oggenc SINE/sine_dbM3_1000Hz_08k.wav -q 5 -o "SINE/sine_dbM3_1000Hz_08k.ogg"
oggenc SINE/sine_dbM3_1000Hz_16k.wav -q 5 -o "SINE/sine_dbM3_1000Hz_16k.ogg"
oggenc SINE/sine_dbM3_1000Hz_32k.wav -q 5 -o "SINE/sine_dbM3_1000Hz_32k.ogg"

oggenc SINE/sine_dbM6_1000Hz_08k.wav -q 5 -o "SINE/sine_dbM6_1000Hz_08k.ogg"
oggenc SINE/sine_dbM6_1000Hz_16k.wav -q 5 -o "SINE/sine_dbM6_1000Hz_16k.ogg"
oggenc SINE/sine_dbM6_1000Hz_32k.wav -q 5 -o "SINE/sine_dbM6_1000Hz_32k.ogg"


