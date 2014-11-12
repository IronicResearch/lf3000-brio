export EXE=./sfconvert
export AUDIOFILES=~/AudioFiles
#export OGGSUMMARYFILE=BreakItDown_ByQ.txt

# -timer -blockTimer -reps 1  -addSummaryTo IslandByQ.txt 
export FLAGS='-timer -reps 1 -fs 16000 -time 20' 
#valgrind --leak-check=yes oggi2wav -timer -reps 1 -bytesToRead 4096 QX/BreakItDown_32k_Q00_st.ogg 

#rm -f $OGGSUMMARYFILE

#$EXE -useEQ -eqMode ByPass  -i $AUDIOFILES/noise05_16k.wav -o outEQ_Bypass.wav

#$EXE -inGainDB  0  -useEQ -eqMode ByPass -i $AUDIOFILES/noise05_16k.wav -o outEQ_Bypass_m00.wav
#$EXE -inGainDB  0  -useEQ -Q15 -eqFc  400 -eqMode LowPass  -i $AUDIOFILES/noise05_16k.wav -o outEQ_LPFq15.wav
#$EXE -inGainDB  0  -useEQ -Q15 -eqFc  400 -eqMode HighPass -i $AUDIOFILES/noise05_16k.wav -o outEQ_HPFq15.wav


$EXE -inGainDB  0  -useEQ -eqFc 4000 -eqMode HighShelf  -eqGainDB -20 -eqQ 1  -i $AUDIOFILES/noise05_16k.wav -o outEQ_HSFq15.wav
$EXE -inGainDB 0 -useEQ -eqFc 4000 -eqMode LowShelf   -eqGainDB -20 -eqQ 1  -i $AUDIOFILES/noise05_16k.wav -o outEQ_LSFq15.wav

#$EXE -useEQ -Q15 -inGainDB 0 -eqFc 2000 -eqMode Parametric -eqGainDB -40 -eqQ 5 -i $AUDIOFILES/noise05_16k.wav -o outEQ_PFq15.wav
#$EXE -useEQ      -inGainDB 0 -eqFc 2000 -eqMode Parametric -eqGainDB -40 -eqQ 5 -i $AUDIOFILES/noise05_16k.wav -o outEQ_PFf.wav


#$EXE -useEQ      -eqFc  400 -eqMode LowPass  -i $AUDIOFILES/noise05_16k.wav -o outEQ_LPFf.wav

#$EXE -useEQ -eqFc 8000 -eqMode HighPass -i $AUDIOFILES/noise05_44k.wav -o outEQ_HPF.wav
#$EXE -useEQ -eqFc 1000 -eqMode Parametric -i $AUDIOFILES/noise05_16k.wav -o outEQ_PF.wav





