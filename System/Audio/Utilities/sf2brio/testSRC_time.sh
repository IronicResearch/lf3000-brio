
# Linear Interpolation upsampling tests
#./sf2brio -t -fs 44100 -srcType Linear SINE22.WAV outSine_22to44k_Linearf.raw
#./sf2brio -t -fixedpt -fs 44100 -srcType Linear SINE22.WAV outSine_22to44k_Lineari.raw

./sf2brio -t -fixedpt -fs 44100 -srcType AddDrop SINE22.WAV 
./sf2brio -t -fixedpt -fs 44100 -srcType Linear SINE22.WAV 

