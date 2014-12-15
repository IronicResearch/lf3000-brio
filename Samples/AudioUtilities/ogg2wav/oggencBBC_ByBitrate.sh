#!/bin/sh

echo "Ogg encode "

# BBC sound file, originally at some weird 11111 Hz sampling frequency
#oggenc -q 5 BBC_11_mono.wav -o "Q5/BBC_11_mono_Q5.ogg"

oggenc -m  40 -M  40 BBC_08k_mono.wav -o "BX/BBC_08k_mono_040kb.ogg"
oggenc -m  56 -M  56 BBC_08k_mono.wav -o "BX/BBC_08k_mono_056kb.ogg"
oggenc -m  96 -M  96 BBC_08k_mono.wav -o "BX/BBC_08k_mono_096kb.ogg"
oggenc -m 128 -M 128 BBC_08k_mono.wav -o "BX/BBC_08k_mono_128kb.ogg"
oggenc -m 192 -M 192 BBC_08k_mono.wav -o "BX/BBC_08k_mono_192kb.ogg"

oggenc -m  40 -M  40 BBC_16k_mono.wav -o "BX/BBC_16k_mono_040kb.ogg"
oggenc -m  56 -M  56 BBC_16k_mono.wav -o "BX/BBC_16k_mono_056kb.ogg"
oggenc -m  96 -M  96 BBC_16k_mono.wav -o "BX/BBC_16k_mono_096kb.ogg"
oggenc -m 128 -M 128 BBC_16k_mono.wav -o "BX/BBC_16k_mono_128kb.ogg"
oggenc -m 192 -M 192 BBC_16k_mono.wav -o "BX/BBC_16k_mono_192kb.ogg"

oggenc -m  40 -M  40 BBC_24k_mono.wav -o "BX/BBC_24k_mono_040kb.ogg"
oggenc -m  56 -M  56 BBC_24k_mono.wav -o "BX/BBC_24k_mono_056kb.ogg"
oggenc -m  96 -M  96 BBC_24k_mono.wav -o "BX/BBC_24k_mono_096kb.ogg"
oggenc -m 128 -M 128 BBC_24k_mono.wav -o "BX/BBC_24k_mono_128kb.ogg"
oggenc -m 192 -M 192 BBC_24k_mono.wav -o "BX/BBC_24k_mono_192kb.ogg"

oggenc -m  40 -M  40 BBC_32k_mono.wav -o "BX/BBC_32k_mono_040kb.ogg"
oggenc -m  56 -M  56 BBC_32k_mono.wav -o "BX/BBC_32k_mono_056kb.ogg"
oggenc -m  96 -M  96 BBC_32k_mono.wav -o "BX/BBC_32k_mono_096kb.ogg"
oggenc -m 128 -M 128 BBC_32k_mono.wav -o "BX/BBC_32k_mono_128kb.ogg"
oggenc -m 192 -M 192 BBC_32k_mono.wav -o "BX/BBC_32k_mono_192kb.ogg"

oggenc -m  40 -M  40 BBC_44k_mono.wav -o "BX/BBC_44k_mono_040kb.ogg"
oggenc -m  56 -M  56 BBC_44k_mono.wav -o "BX/BBC_44k_mono_056kb.ogg"
oggenc -m  96 -M  96 BBC_44k_mono.wav -o "BX/BBC_44k_mono_096kb.ogg"
oggenc -m 128 -M 128 BBC_44k_mono.wav -o "BX/BBC_44k_mono_128kb.ogg"
oggenc -m 192 -M 192 BBC_44k_mono.wav -o "BX/BBC_44k_mono_192kb.ogg"

