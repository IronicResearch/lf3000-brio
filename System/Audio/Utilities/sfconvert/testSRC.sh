export EXE=./sfconvert
export AF=~/AudioFiles
export NOISE=~/AudioFiles/NOISE

# Unfiltered resampling tests
#$EXE -fs 44100 -srcType Unfiltered SINE22.WAV outSINE22to44_Unfiltered.raw
#$EXE -fs 44100 -srcType Unfiltered SawtoothC2_22k.wav outSawC2_22to44_Unfiltered.raw
#$EXE -fs 44100 -srcType Unfiltered Temptation_22k_mono.wav outTemptation22to44_Unfiltered.raw
#cp out*_Unfiltered*.raw /mnt/hgfs/win_c/Tmp/SRC

# Add/Drop resampling tests
#$EXE -fs 22050 -srcType AddDrop SawC2_44.wav outSaw44_22_AddDropf.raw
#$EXE -fixedpt -fs 22050 -srcType AddDrop SawC2_44.wav outSaw44_22_AddDropi.raw
#$EXE -fs 44100 -srcType AddDrop SINE22.WAV outSINE22to44_AddDropf.raw
#$EXE -fixedpt -fs 44100 -srcType AddDrop SINE22.WAV outSINE22to44_AddDropi.raw

#$EXE -fs 44100 -srcType AddDrop Temptation_22k_mono.wav outTemptation22to44_AddDrop.raw
#cp out*_AddDrop*.raw /mnt/hgfs/win_c/Tmp/SRC

# Linear Interpolation downsampling tests
#sf2brio -fs 22050 -srcType Linear sine220_44k.wav outSineLinear_220_44to22k_Linearf.raw
#sf2brio -fs 16000 -srcType Linear sine220_44k.wav outSineLinear_220_44to16k_Linearf.raw
#sf2brio -fs  8000 -srcType Linear sine220_44k.wav outSineLinear_220_44to08k_Linearf.raw
#sf2brio -fs 44100 -srcType Linear sine220_44k.wav outSineLinear_220_44to44k_Linearf.raw
#$EXE -fixedpt -fs 22050 -srcType Linear sine220_44k.wav outSineLinear_220_44to22k_Lineari.raw
#$EXE -fixedpt -fs 16000 -srcType Linear sine220_44k.wav outSineLinear_220_44to16k_Lineari.raw
#$EXE -fixedpt -fs  8000 -srcType Linear sine220_44k.wav outSineLinear_220_44to08k_Lineari.raw
#$EXE -fixedpt -fs 44100 -srcType Linear sine220_44k.wav outSineLinear_220_44to44k_Lineari.raw

# Linear Interpolation upsampling tests
#$EXE -fs 44100 -srcType Linear SINE08.WAV outSine_08to44k_Linearf.raw
#$EXE -fs 44100 -srcType Linear SINE16.WAV outSine_16to44k_Linearf.raw
#$EXE -t -fs 44100 -srcType Linear SINE22.WAV outSine_22to44k_Linearf.raw
#$EXE -fixedpt -fs 44100 -srcType Linear SINE08.WAV outSine_08to44k_Lineari.raw
#$EXE -fixedpt -fs 44100 -srcType Linear SINE16.WAV outSine_16to44k_Lineari.raw
#$EXE -t -fixedpt -fs 44100 -srcType Linear SINE22.WAV outSine_22to44k_Lineari.raw
#$EXE -fs 44100 -srcType Linear SINE32.WAV outSine_32to44k_Linear.raw
#$EXE -fs 44100 -srcType Linear SINE44.WAV outSine_44to44k_Linear.raw
#$EXE -fs 44100 -srcType AddDrop Temptation_22k_mono.wav outTemptation22to44_Linear.raw
#$EXE -fs 44100 -srcType Linear SawtoothC2_22k.wav outSawC2_22to44_Linear.raw
#cp out*_Linear*.raw /mnt/hgfs/win_c/Tmp/SRC

# Triangle Resampling by 2 tests
#$EXE -fs 22050 -srcType Triangle sine220_44k.wav outSineTriangleBy2_220_44to22k.raw
#$EXE -fs 44100 -srcType Triangle SINE22.WAV      outSineTriangleBy2_22to44k.raw

# FIR Resampling by 2 tests
#$EXE -fs 22050 -srcType FIR sine220_44k.wav outSineby2_220_44to22k_FIRf.raw
#$EXE -fixedpt -fs 22050 -srcType FIR sine220_44k.wav outSineby2_220_44to22k_FIRi.raw
#$EXE -t -fs 44100 -srcType FIR SINE22.WAV outSineby2_22to44kf_FIR_BREAKOUT.raw

#$EXE -outFormat wav -fs 44100 -srcType FIR SINE22.WAV outSineby2_22to44k_FIRf.wav
#$EXE -outFormat wav -fixedpt -fs 44100 -srcType FIR SINE22.WAV outSineby2_22to44k_FIRi.wav
#$EXE -fixedpt -fs 44100 -srcType FIR Temptation_22k_mono.wav outTemptation22to44_FIRi.raw
#$EXE -fs 44100 -srcType FIR SawtoothC2_22k.wav outSawC2_22to44_FIRf.raw
#cp out*_FIR*.* /mnt/hgfs/win_c/Tmp/SRC

# FIR Resampling by 2 tests
#$EXE -outFormat wav -fixedpt -fs 22050 -srcType Linear SINE44.WAV outSineBy2_44to22k_Lineari.wav
#$EXE -outFormat wav -fs 22050 -srcType FIR SINE44.WAV outSineBy2_44to22k_FIRf.wav
#$EXE -outFormat wav -fixedpt -fs 22050 -srcType FIR SINE44.WAV outSineBy2_44to22k_FIRi.wav
#$EXE -outFormat wav -fs 44100 -srcType FIR SINE22.WAV outSineBy2_22to44k_FIRf.wav
#$EXE -outFormat wav -fixedpt -fs 44100 -srcType FIR SINE22.WAV outSineBy2_22to44k_FIRi.wav
#cp outSineBy2*.* /mnt/hgfs/win_c/Tmp/SRC

# FIR Resampling by 3 tests
#$EXE -outFormat wav -fixedpt -fs 16000 -srcType Linear SINE48.WAV outSineBy3_48to16k_Lineari.wav
#$EXE -outFormat wav          -fs 16000 -srcType FIR SINE48.WAV outSineBy3_48to16k_FIRf.wav
#$EXE -outFormat wav -fixedpt -fs 16000 -srcType FIR SINE48.WAV outSineBy3_48to16k_FIRi.wav

#$EXE -outFormat wav -fs 48000 -srcType Linear SINE16.WAV outSineBy3_16to48k_Lineari.wav
#$EXE -outFormat wav -fs 48000 -srcType FIR SINE16.WAV outSineBy3_16to48k_FIRf.wav
#$EXE -outFormat wav -fixedpt -fs 48000 -srcType FIR SINE16.WAV outSineBy3_16to48k_FIRi.wav
cp outSineBy3*.* /mnt/hgfs/win_c/Tmp/SRC

# Miscellaneous tests
#sf2brio -fs 22050 -srcType Test impulse.wav testFIR_Impulse_44k.raw
#sf2brio -fs 44100 -srcType Test sine880_44k.wav testFIR_44k.raw
#sf2brio -fs 22050 -srcType Test sine880_44k.wav outSine.raw
#sf2brio -fs 22050 -srcType Test noise44_30.wav outNoise.raw
