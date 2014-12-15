# FIR tests 
# Add ./ to executable because MagicEyes Linux image doesn't include . in path
export EXE=./sfconvert
export AF=~/AudioFiles
export NOISE=~/AudioFiles/NOISE
#export OGGSUMMARYFILE=BreakItDown_ByQ.txt

# -timer -blockTimer -reps 1  -addSummaryTo IslandByQ.txt 
export FLAGS='-useIIR' 

#$EXE -1 -impulse -usefir -firType LowPass impulse.wav outimpulse.wav
#$EXE -usefir -firType LowPass SawC2_44.wav outSawC2_44.wav

# ******** TEST of basic FIR types ************
#$EXE -fixedpt -usefir -firType LowPass  sine880_44k.wav outSine_BasicTest_LP_44i.wav
#$EXE -t -fixedpt -usefir -firType LowPass  noise05_44k.wav outNoise_BasicTest_LP_44i.wav

#$EXE -t -largeblocks -impulse -fixedpt -usefir -firType LowPass 
#$EXE -t -largeblocks -impulse -usefir -firType LowPass 
#-outfile impulsei.wav

#$EXE -usefir -firType LowPass  sine880_44k.wav outSine_BasicTest_LP_44f.wav
#$EXE -usefir -firType HighPass sine880_44k.wav outSine_BasicTest_HP_44f.wav
#$EXE -usefir -firType BandPass sine880_44k.wav outSine_BasicTest_BP_44f.wav
#$EXE -usefir -firType BandStop sine880_44k.wav outSine_BasicTest_BS_44f.wav

#$EXE -t -usefir -firType LowPass  noise05_44k.wav outNoise_BasicTest_LP_44f.wav
#$EXE -usefir -firType HighPass noise05_44k.wav outNoise_BasicTest_HP_44f.wav
#$EXE -usefir -firType BandPass noise05_44k.wav outNoise_BasicTest_BP_44f.wav
#$EXE -usefir -firType BandStop noise05_44k.wav outNoise_BasicTest_BS_44f.wav
#cp out*BasicTest*.wav /mnt/hgfs/win_c/Tmp/FIR

#$EXE -usefir -firType BandStop noise44_30.wav outNoise_BS_44.wav

#$EXE -usefir -firType LowPass SawC2_44.wav outSawC2_44.wav
#$EXE -usefir -firType LowPass sine880_44k.wav outSine880_44k.wav

#$EXE sine880_44k.wav outSine880_44k.wav
#$EXE sine055_44k.wav outSine055_44k.wav

#$EXE -usefir -firType LowPass SawC2_44.wav outSawC2_44f.wav
#$EXE -fixedpt -usefir -firType LowPass SawC2_44.wav outSawC2_44i.wav
#cp outSawC2* /mnt/hgfs/win_c/Tmp

#$EXE -usefir -firType HalfBand_15 $NOISE/noise05_32k.wav outNoise05_32k_HalfBand15.wav
#$EXE -usefir -firType HalfBand_31 $NOISE/noise05_32k.wav outNoise05_32k_HalfBand31.wav
#$EXE -usefir -firType ThirdBand_15 $NOISE/noise05_32k.wav outNoise05_32k_ThirdBand15.wav
#$EXE -usefir -firType ThirdBand_31 $NOISE/noise05_32k.wav outNoise05_32k_ThirdBand31.wav
$EXE -usefir -firType HalfBand_30dB_15 $NOISE/noise05_32k.wav outNoise05_32k_HalfBand30dB_15.wav
$EXE -usefir -firType HalfBand_50dB_31 $NOISE/noise05_32k.wav outNoise05_32k_HalfBand50dB_31.wav
$EXE -usefir -firType HalfBand_50dB_41 $NOISE/noise05_32k.wav outNoise05_32k_HalfBand50dB_41.wav
cp outNoise*Band*.wav /mnt/hgfs/win_c/Tmp/FIR

#$EXE -usefir -firType Triangle_3 noise05_44k.wav outNoise05_44k_Triangle03.wav
#$EXE -usefir -firType Triangle_9 noise05_44k.wav outNoise05_44k_Triangle09.wav
#cp outNoise*Triangle*.wav /mnt/hgfs/win_c/Tmp/FIR

