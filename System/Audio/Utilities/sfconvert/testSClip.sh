export EXE=./sfconvert
export AF=~/AudioFiles
#export OGGSUMMARYFILE=BreakItDown_ByQ.txt

# -timer -blockTimer -reps 1  -addSummaryTo IslandByQ.txt 
export FLAGS='-useSClip' 
#valgrind --leak-check=yes oggi2wav -timer -reps 1 -bytesToRead 4096 QX/BreakItDown_32k_Q00_st.ogg 

#rm -f $OGGSUMMARYFILE

#$EXE $FLAGS -sclip_type ByPass -i $AF/Temptation_32k_mono.wav -o outSC_PG_Bypass.wav
#$EXE $FLAGS -sclip_type V1     -i $AF/Temptation_32k_mono.wav -o outSC_PG_V1.wav
#$EXE $FLAGS -sclip_type V2     -i $AF/Temptation_32k_mono.wav -o outSC_PG_V2.wav
#$EXE $FLAGS -sclip_type V3     -i $AF/Temptation_32k_mono.wav -o outSC_PG_V3.wav
#$EXE $FLAGS -sclip_type V4     -i $AF/Temptation_32k_mono.wav -o outSC_PG_V4.wav

#$EXE $FLAGS -sclip_inGainDB -6 -sclip_outGainDB 6 -sclip_type ByPass -i $AF/SINE32.WAV -o outSC_SINE_Bypass.wav
#$EXE $FLAGS -sclip_type V1     -i $AF/SINE32.WAV -o outSC_SINE_V1.wav
#$EXE $FLAGS -sclip_type V2     -i $AF/SINE32.WAV -o outSC_SINE_V2.wav
#$EXE $FLAGS -sclip_type V3     -i $AF/SINE32.WAV -o outSC_SINE_V3.wav
#$EXE $FLAGS -sclip_inGainDB -6 -sclip_outGainDB 0.4 -sclip_type V4  -i $AF/SINE32.WAV -o outSC_SINE_V4.wav
$EXE $FLAGS      -sclip_inGainDB 0 -sclip_outGainDB 0 -sclip_type V4 -i $AF/SINE32.WAV -o outSC_SINE_V4f.wav
$EXE $FLAGS -Q15 -sclip_inGainDB 0 -sclip_outGainDB 0 -sclip_type V4 -i $AF/SINE32.WAV -o outSC_SINE_V4i.wav



