#!/bin/sh

# Various data rates, but difficult to burrow below 40 kbps at 44100 hz
oggenc IslandInTheSun_44k_st.wav -m  40 -M  40 -o "BX/IslandInTheSun_44k_st_040kb.ogg"
oggenc IslandInTheSun_44k_st.wav -m  56 -M  56 -o "BX/IslandInTheSun_44k_st_056kb.ogg"
oggenc IslandInTheSun_44k_st.wav -m  96 -M  96 -o "BX/IslandInTheSun_44k_st_096kb.ogg"
oggenc IslandInTheSun_44k_st.wav -m 128 -M 128 -o "BX/IslandInTheSun_44k_st_128kb.ogg"
oggenc IslandInTheSun_44k_st.wav -m 192 -M 192 -o "BX/IslandInTheSun_44k_st_192kb.ogg"

oggenc IslandInTheSun_44k_mono.wav -m  40 -M  40 -o "BX/IslandInTheSun_44k_mono_040kb.ogg"
oggenc IslandInTheSun_44k_mono.wav -m  56 -M  56 -o "BX/IslandInTheSun_44k_mono_056kb.ogg"
oggenc IslandInTheSun_44k_mono.wav -m  96 -M  96 -o "BX/IslandInTheSun_44k_mono_096kb.ogg"
oggenc IslandInTheSun_44k_mono.wav -m 128 -M 128 -o "BX/IslandInTheSun_44k_mono_128kb.ogg"
oggenc IslandInTheSun_44k_mono.wav -m 192 -M 192 -o "BX/IslandInTheSun_44k_mono_192kb.ogg"

oggenc IslandInTheSun_32k_st.wav -m  40 -M  40 -o "BX/IslandInTheSun_32k_st_040kb.ogg"
oggenc IslandInTheSun_32k_st.wav -m  56 -M  56 -o "BX/IslandInTheSun_32k_st_056kb.ogg"
oggenc IslandInTheSun_32k_st.wav -m  96 -M  96 -o "BX/IslandInTheSun_32k_st_096kb.ogg"
oggenc IslandInTheSun_32k_st.wav -m 128 -M 128 -o "BX/IslandInTheSun_32k_st_128kb.ogg"
oggenc IslandInTheSun_32k_st.wav -m 192 -M 192 -o "BX/IslandInTheSun_32k_st_192kb.ogg"

oggenc IslandInTheSun_24k_st.wav -m  40 -M  40 -o "BX/IslandInTheSun_24k_st_040kb.ogg"
oggenc IslandInTheSun_24k_st.wav -m  56 -M  56 -o "BX/IslandInTheSun_24k_st_056kb.ogg"
oggenc IslandInTheSun_24k_st.wav -m  96 -M  96 -o "BX/IslandInTheSun_24k_st_096kb.ogg"
oggenc IslandInTheSun_24k_st.wav -m 128 -M 128 -o "BX/IslandInTheSun_24k_st_128kb.ogg"
oggenc IslandInTheSun_24k_st.wav -m 192 -M 192 -o "BX/IslandInTheSun_24k_st_192kb.ogg"

oggenc IslandInTheSun_16k_st.wav -m  40 -M  40 -o "BX/IslandInTheSun_16k_st_040kb.ogg"
oggenc IslandInTheSun_16k_st.wav -m  56 -M  56 -o "BX/IslandInTheSun_16k_st_056kb.ogg"
oggenc IslandInTheSun_16k_st.wav -m  96 -M  96 -o "BX/IslandInTheSun_16k_st_096kb.ogg"
oggenc IslandInTheSun_16k_st.wav -m 128 -M 128 -o "BX/IslandInTheSun_16k_st_128kb.ogg"
oggenc IslandInTheSun_16k_st.wav -m 192 -M 192 -o "BX/IslandInTheSun_16k_st_192kb.ogg"

