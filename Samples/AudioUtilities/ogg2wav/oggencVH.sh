#!/bin/sh

echo "Ogg encode with Quality=5"

# Van Halen's 1984
oggenc -q 5 VH_11_mono.wav -o VH_11_mono.ogg
oggenc -q 5 VH_16_mono.wav -o VH_16_mono.ogg
oggenc -q 5 VH_22_mono.wav -o VH_22_mono.ogg
oggenc -q 5 VH_32_mono.wav -o VH_32_mono.ogg
oggenc -q 5 VH_44_mono.wav -o VH_44_mono.ogg

oggenc -q 5 VH_11_st.wav -o VH_11_st.ogg
oggenc -q 5 VH_16_st.wav -o VH_16_st.ogg
oggenc -q 5 VH_22_st.wav -o VH_22_st.ogg
oggenc -q 5 VH_32_st.wav -o VH_32_st.ogg
oggenc -q 5 VH_44_st.wav -o VH_44_st.ogg

# Various Quality settings
oggenc -q  2 VH_32_mono.wav -o "QX/VH_32k_Q02_mono.ogg"
oggenc -q  4 VH_32_mono.wav -o "QX/VH_32k_Q04_mono.ogg"
oggenc -q  6 VH_32_mono.wav -o "QX/VH_32k_Q06_mono.ogg"
oggenc -q  8 VH_32_mono.wav -o "QX/VH_32k_Q08_mono.ogg"
oggenc -q 10 VH_32_mono.wav -o "QX/VH_32k_Q10_mono.ogg"

oggenc -q  2 VH_44_mono.wav -o "QX/VH_44k_Q02_mono.ogg"
oggenc -q  4 VH_44_mono.wav -o "QX/VH_44k_Q04_mono.ogg"
oggenc -q  6 VH_44_mono.wav -o "QX/VH_44k_Q06_mono.ogg"
oggenc -q  8 VH_44_mono.wav -o "QX/VH_44k_Q08_mono.ogg"
oggenc -q 10 VH_44_mono.wav -o "QX/VH_44k_Q10_mono.ogg"

