#!/bin/sh

#echo "Ogg encode"

# Van Halen's 1984

# Various Quality settings
oggenc -m  40 -M  40 VH_16k_mono.wav -o "BX/VH_16k_mono_040kbps.ogg"
oggenc -m  56 -M  56 VH_16k_mono.wav -o "BX/VH_16k_mono_056kbps.ogg"
oggenc -m  96 -M  96 VH_16k_mono.wav -o "BX/VH_16k_mono_096kbps.ogg"
oggenc -m 128 -M 128 VH_16k_mono.wav -o "BX/VH_16k_mono_128kbps.ogg"
oggenc -m 192 -M 192 VH_16k_mono.wav -o "BX/VH_16k_mono_192kbps.ogg"

oggenc -m  40 -M  40 VH_16k_st.wav -o "BX/VH_16k_st_040kbps.ogg"
oggenc -m  56 -M  56 VH_16k_st.wav -o "BX/VH_16k_st_056kbps.ogg"
oggenc -m  96 -M  96 VH_16k_st.wav -o "BX/VH_16k_st_096kbps.ogg"
oggenc -m 128 -M 128 VH_16k_st.wav -o "BX/VH_16k_st_128kbps.ogg"
oggenc -m 192 -M 192 VH_16k_st.wav -o "BX/VH_16k_st_192kbps.ogg"

oggenc -m  40 -M  40 VH_32k_mono.wav -o "BX/VH_32k_mono_040kbps.ogg"
oggenc -m  56 -M  56 VH_32k_mono.wav -o "BX/VH_32k_mono_056kbps.ogg"
oggenc -m  96 -M  96 VH_32k_mono.wav -o "BX/VH_32k_mono_096kbps.ogg"
oggenc -m 128 -M 128 VH_32k_mono.wav -o "BX/VH_32k_mono_128kbps.ogg"
oggenc -m 192 -M 192 VH_32k_mono.wav -o "BX/VH_32k_mono_192kbps.ogg"

oggenc -m  40 -M  40 VH_32k_st.wav -o "BX/VH_32k_st_040kbps.ogg"
oggenc -m  56 -M  56 VH_32k_st.wav -o "BX/VH_32k_st_056kbps.ogg"
oggenc -m  96 -M  96 VH_32k_st.wav -o "BX/VH_32k_st_096kbps.ogg"
oggenc -m 128 -M 128 VH_32k_st.wav -o "BX/VH_32k_st_128kbps.ogg"
oggenc -m 192 -M 192 VH_32k_st.wav -o "BX/VH_32k_st_192kbps.ogg"

oggenc -m  40 -M  40 VH_44k_mono.wav -o "BX/VH_44k_mono_040kbps.ogg"
oggenc -m  56 -M  56 VH_44k_mono.wav -o "BX/VH_44k_mono_056kbps.ogg"
oggenc -m  96 -M  96 VH_44k_mono.wav -o "BX/VH_44k_mono_096kbps.ogg"
oggenc -m 128 -M 128 VH_44k_mono.wav -o "BX/VH_44k_mono_128kbps.ogg"
oggenc -m 192 -M 192 VH_44k_mono.wav -o "BX/VH_44k_mono_192kbps.ogg"

oggenc -m  40 -M  40 VH_44k_st.wav -o "BX/VH_44k_st_040kbps.ogg"
oggenc -m  56 -M  56 VH_44k_st.wav -o "BX/VH_44k_st_056kbps.ogg"
oggenc -m  96 -M  96 VH_44k_st.wav -o "BX/VH_44k_st_096kbps.ogg"
oggenc -m 128 -M 128 VH_44k_st.wav -o "BX/VH_44k_st_128kbps.ogg"
oggenc -m 192 -M 192 VH_44k_st.wav -o "BX/VH_44k_st_192kbps.ogg"

