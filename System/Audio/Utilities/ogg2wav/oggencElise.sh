#!/bin/sh

echo "Ogg encode Lightning content files from Elise"

# Lighting content files from Elise Hurwitz

oggenc -q 5 RunawayBunny_16k.wav -o "Q5/RunawayBunny_16k_Q5.ogg"
oggenc -q 5 RunawayBunny_22k.wav -o "Q5/RunawayBunny_22k_Q5.ogg"

oggenc -q 5 HortonHearsWho_16k.wav -o "Q5/HortonHearsWho_16k_Q5.ogg"
oggenc -q 5 HortonHearsWho_22k.wav -o "Q5/HortonHearsWho_22k_Q5.ogg"

oggenc -q 5 Shrek_16k.wav -o "Q5/Shrek_16k_Q5.ogg"
oggenc -q 5 Shrek_22k.wav -o "Q5/Shrek_22k_Q5.ogg"

# Mono-fied versions
oggenc -q 5 RunawayBunny_16k_mono.wav -o "Q5/RunawayBunny_16k_Q5_mono.ogg"
oggenc -q 5 RunawayBunny_22k_mono.wav -o "Q5/RunawayBunny_22k_Q5_mono.ogg"

oggenc -q 5 HortonHearsWho_16k_mono.wav -o "Q5/HortonHearsWho_16k_Q5_mono.ogg"
oggenc -q 5 HortonHearsWho_22k_mono.wav -o "Q5/HortonHearsWho_22k_Q5_mono.ogg"

oggenc -q 5 Shrek_16k_mono.wav -o "Q5/Shrek_16k_Q5_mono.ogg"
oggenc -q 5 Shrek_22k_mono.wav -o "Q5/Shrek_22k_Q5_mono.ogg"

# Various Q
oggenc -q  2 RunawayBunny_16k_mono.wav -o "QX/RunawayBunny_16k_Q02_mono.ogg"
oggenc -q  4 RunawayBunny_16k_mono.wav -o "QX/RunawayBunny_16k_Q04_mono.ogg"
oggenc -q  6 RunawayBunny_16k_mono.wav -o "QX/RunawayBunny_16k_Q06_mono.ogg"
oggenc -q  8 RunawayBunny_16k_mono.wav -o "QX/RunawayBunny_16k_Q08_mono.ogg"
oggenc -q 10 RunawayBunny_16k_mono.wav -o "QX/RunawayBunny_16k_Q10_mono.ogg"



