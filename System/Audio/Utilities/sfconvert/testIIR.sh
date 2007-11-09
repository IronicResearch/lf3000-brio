# IIR tests 
# Add ./ to executable because MagicEyes Linux image doesn't include . in path

export EXE=./sfconvert
export AF=~/AudioFiles
#export OGGSUMMARYFILE=BreakItDown_ByQ.txt

# -timer -blockTimer -reps 1  -addSummaryTo IslandByQ.txt 
export FLAGS='-useIIR' 
#valgrind --leak-check=yes oggi2wav -timer -reps 1 -bytesToRead 4096 QX/BreakItDown_32k_Q00_st.ogg 

#rm -f $OGGSUMMARYFILE

#$EXE $FLAGS -i $AF/Temptation_32k_mono.wav -o outIIR_T32_IIR1.wav
#$EXE $FLAGS -iir_Fc  50 -iir_Type lpf -iir_Order 1 -i $AF/NOISE/noise05_32k.wav -o outIIR_Noise32_IIR1_050.wav
#$EXE $FLAGS -iir_Fc 100 -iir_Type lpf -iir_Order 1 -i $AF/NOISE/noise05_32k.wav -o outIIR_Noise32_IIR1_100.wav
#$EXE $FLAGS -iir_Fc 400 -iir_Type lpf -iir_Order 1 -i $AF/NOISE/noise05_32k.wav -o outIIR_Noise32_IIR1_400.wav

#$EXE $FLAGS -Q15 -iir_Order 1 -i $AF/NOISE/noise05_32k.wav -o outIIR_Noise32_IIR_HalfBand_01i.wav
#$EXE $FLAGS      -iir_Order 1 -i $AF/NOISE/noise05_32k.wav -o outIIR_Noise32_IIR_HalfBand_01f.wav

#$EXE $FLAGS -Q15 -iir_Order 1 -i $AF/NOISE/noise05_32k.wav -o outIIR_Noise32_IIR_ButterLPF_02i.wav
$EXE $FLAGS -Q15 -iir_Order 1 -i $AF/NOISE/noise05_32k.wav -o outIIR_Noise32_IIR_HalfBand30_02i.wav

