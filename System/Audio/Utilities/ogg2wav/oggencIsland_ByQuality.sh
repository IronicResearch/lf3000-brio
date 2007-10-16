#!/bin/sh

echo "Ogg encode at several Quality levels"

# Island of the Sun sound file fragment, 17 seconds, from Elise Hurwitz
oggenc -q 0 IslandInTheSun_24k_st.wav -o "QX/IslandInTheSun_24k_st_Q0.ogg"
oggenc -q 1 IslandInTheSun_24k_st.wav -o "QX/IslandInTheSun_24k_st_Q1.ogg"
oggenc -q 2 IslandInTheSun_24k_st.wav -o "QX/IslandInTheSun_24k_st_Q2.ogg"
oggenc -q 3 IslandInTheSun_24k_st.wav -o "QX/IslandInTheSun_24k_st_Q3.ogg"
oggenc -q 4 IslandInTheSun_24k_st.wav -o "QX/IslandInTheSun_24k_st_Q4.ogg"
oggenc -q 5 IslandInTheSun_24k_st.wav -o "QX/IslandInTheSun_24k_st_Q5.ogg"
oggenc -q 6 IslandInTheSun_24k_st.wav -o "QX/IslandInTheSun_24k_st_Q6.ogg"
oggenc -q 8 IslandInTheSun_24k_st.wav -o "QX/IslandInTheSun_24k_st_Q8.ogg"

oggenc -q 0 IslandInTheSun_32k_st.wav -o "QX/IslandInTheSun_32k_st_Q0.ogg"
oggenc -q 1 IslandInTheSun_32k_st.wav -o "QX/IslandInTheSun_32k_st_Q1.ogg"
oggenc -q 2 IslandInTheSun_32k_st.wav -o "QX/IslandInTheSun_32k_st_Q2.ogg"
oggenc -q 3 IslandInTheSun_32k_st.wav -o "QX/IslandInTheSun_32k_st_Q3.ogg"
oggenc -q 4 IslandInTheSun_32k_st.wav -o "QX/IslandInTheSun_32k_st_Q4.ogg"
oggenc -q 5 IslandInTheSun_32k_st.wav -o "QX/IslandInTheSun_32k_st_Q5.ogg"
oggenc -q 6 IslandInTheSun_32k_st.wav -o "QX/IslandInTheSun_32k_st_Q6.ogg"
oggenc -q 8 IslandInTheSun_32k_st.wav -o "QX/IslandInTheSun_32k_st_Q8.ogg"

oggenc -q 0 IslandInTheSun_44k_st.wav -o "QX/IslandInTheSun_44k_st_Q0.ogg"
oggenc -q 1 IslandInTheSun_44k_st.wav -o "QX/IslandInTheSun_44k_st_Q1.ogg"
oggenc -q 2 IslandInTheSun_44k_st.wav -o "QX/IslandInTheSun_44k_st_Q2.ogg"
oggenc -q 3 IslandInTheSun_44k_st.wav -o "QX/IslandInTheSun_44k_st_Q3.ogg"
oggenc -q 4 IslandInTheSun_44k_st.wav -o "QX/IslandInTheSun_44k_st_Q4.ogg"
oggenc -q 5 IslandInTheSun_44k_st.wav -o "QX/IslandInTheSun_44k_st_Q5.ogg"
oggenc -q 6 IslandInTheSun_44k_st.wav -o "QX/IslandInTheSun_44k_st_Q6.ogg"
oggenc -q 8 IslandInTheSun_44k_st.wav -o "QX/IslandInTheSun_44k_st_Q8.ogg"

# Various data rates, but difficult to burrow below 40 kbps at 44100 hz
oggenc IslandInTheSun_44k_st.wav -m  40 -M  40 -o "QB/IslandInTheSun_44k_st_040kb.ogg"
oggenc IslandInTheSun_44k_st.wav -m  56 -M  56 -o "QB/IslandInTheSun_44k_st_056kb.ogg"
oggenc IslandInTheSun_44k_st.wav -m  96 -M  96 -o "QB/IslandInTheSun_44k_st_096kb.ogg"
oggenc IslandInTheSun_44k_st.wav -m 128 -M 128 -o "QB/IslandInTheSun_44k_st_128kb.ogg"
oggenc IslandInTheSun_44k_st.wav -m 192 -M 192 -o "QB/IslandInTheSun_44k_st_192kb.ogg"

oggenc IslandInTheSun_44k_mono.wav -m  40 -M  40 -o "QB/IslandInTheSun_44k_mono_040kb.ogg"
oggenc IslandInTheSun_44k_mono.wav -m  56 -M  56 -o "QB/IslandInTheSun_44k_mono_056kb.ogg"
oggenc IslandInTheSun_44k_mono.wav -m  96 -M  96 -o "QB/IslandInTheSun_44k_mono_096kb.ogg"
oggenc IslandInTheSun_44k_mono.wav -m 128 -M 128 -o "QB/IslandInTheSun_44k_mono_128kb.ogg"
oggenc IslandInTheSun_44k_mono.wav -m 192 -M 192 -o "QB/IslandInTheSun_44k_mono_192kb.ogg"

oggenc IslandInTheSun_32k_st.wav -m  40 -M  40 -o "QB/IslandInTheSun_32k_st_040kb.ogg"
oggenc IslandInTheSun_32k_st.wav -m  56 -M  56 -o "QB/IslandInTheSun_32k_st_056kb.ogg"
oggenc IslandInTheSun_32k_st.wav -m  96 -M  96 -o "QB/IslandInTheSun_32k_st_096kb.ogg"
oggenc IslandInTheSun_32k_st.wav -m 128 -M 128 -o "QB/IslandInTheSun_32k_st_128kb.ogg"
oggenc IslandInTheSun_32k_st.wav -m 192 -M 192 -o "QB/IslandInTheSun_32k_st_192kb.ogg"

oggenc IslandInTheSun_24k_st.wav -m  40 -M  40 -o "QB/IslandInTheSun_24k_st_040kb.ogg"
oggenc IslandInTheSun_24k_st.wav -m  56 -M  56 -o "QB/IslandInTheSun_24k_st_056kb.ogg"
oggenc IslandInTheSun_24k_st.wav -m  96 -M  96 -o "QB/IslandInTheSun_24k_st_096kb.ogg"
oggenc IslandInTheSun_24k_st.wav -m 128 -M 128 -o "QB/IslandInTheSun_24k_st_128kb.ogg"
oggenc IslandInTheSun_24k_st.wav -m 192 -M 192 -o "QB/IslandInTheSun_24k_st_192kb.ogg"

