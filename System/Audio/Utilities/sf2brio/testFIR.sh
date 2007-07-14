# FIR tests 
# Add ./ to executable because MagicEyes Linux image doesn't include . in path

#./sf2brio -1 -impulse -usefir -firType LowPass impulse.wav outimpulse.raw
#.sf2brio -usefir -firType LowPass SawC2_44.wav outSawC2_44.raw

# ******** TEST of basic FIR types ************
#./sf2brio -fixedpt -usefir -firType LowPass  sine880_44k.wav outSine_BasicTest_LP_44i.raw
#./sf2brio -t -fixedpt -usefir -firType LowPass  noise05_44k.wav outNoise_BasicTest_LP_44i.raw

#./sf2brio -t -largeblocks -impulse -fixedpt -usefir -firType LowPass 
#./sf2brio -t -largeblocks -impulse -usefir -firType LowPass 
#-outfile impulsei.raw

#./sf2brio -usefir -firType LowPass  sine880_44k.wav outSine_BasicTest_LP_44f.raw
#./sf2brio -usefir -firType HighPass sine880_44k.wav outSine_BasicTest_HP_44f.raw
#./sf2brio -usefir -firType BandPass sine880_44k.wav outSine_BasicTest_BP_44f.raw
#./sf2brio -usefir -firType BandStop sine880_44k.wav outSine_BasicTest_BS_44f.raw

#./sf2brio -t -usefir -firType LowPass  noise05_44k.wav outNoise_BasicTest_LP_44f.raw
#./sf2brio -usefir -firType HighPass noise05_44k.wav outNoise_BasicTest_HP_44f.raw
#./sf2brio -usefir -firType BandPass noise05_44k.wav outNoise_BasicTest_BP_44f.raw
#./sf2brio -usefir -firType BandStop noise05_44k.wav outNoise_BasicTest_BS_44f.raw
#cp out*BasicTest*.raw /mnt/hgfs/win_c/Tmp/FIR

#./sf2brio -usefir -firType BandStop noise44_30.wav outNoise_BS_44.raw

#./sf2brio -usefir -firType LowPass SawC2_44.wav outSawC2_44.raw
#./sf2brio -usefir -firType LowPass sine880_44k.wav outSine880_44k.raw

#./sf2brio sine880_44k.wav outSine880_44k.raw
#./sf2brio sine055_44k.wav outSine055_44k.raw

#./sf2brio -usefir -firType LowPass SawC2_44.wav outSawC2_44f.raw
#./sf2brio -fixedpt -usefir -firType LowPass SawC2_44.wav outSawC2_44i.raw
#cp outSawC2* /mnt/hgfs/win_c/Tmp

#./sf2brio -usefir -firType HalfBand_15 noise05_44k.wav outNoise05_44k_HalfBand15.raw
#./sf2brio -usefir -firType HalfBand_31 noise05_44k.wav outNoise05_44k_HalfBand31.raw
#./sf2brio -usefir -firType ThirdBand_15 noise05_44k.wav outNoise05_44k_ThirdBand15.raw
#./sf2brio -usefir -firType ThirdBand_31 noise05_44k.wav outNoise05_44k_ThirdBand31.raw
#cp outNoise*Band*.raw /mnt/hgfs/win_c/Tmp/FIR

#./sf2brio -usefir -firType Triangle_3 noise05_44k.wav outNoise05_44k_Triangle03.raw
#./sf2brio -usefir -firType Triangle_9 noise05_44k.wav outNoise05_44k_Triangle09.raw
#cp outNoise*Triangle*.raw /mnt/hgfs/win_c/Tmp/FIR

# Format type tests
./sf2brio -usefir -outformat brio SawtoothC2_44k.wav outSawtoothC2_44k_FormatTest.brio
./sf2brio -usefir -outformat aiff SawtoothC2_44k.wav outSawtoothC2_44k_FormatTest.aiff
./sf2brio -usefir -outformat wav  SawtoothC2_44k.wav outSawtoothC2_44k_FormatTest.wav
./sf2brio -usefir -outformat raw  SawtoothC2_44k.wav outSawtoothC2_44k_FormatTest.raw
cp out*FormatTest*.* /mnt/hgfs/win_c/Tmp/Test

