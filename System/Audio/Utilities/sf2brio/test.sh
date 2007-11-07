#./sf2brio -v -i belltree_up2.aiff -outFormat brio -o out.brio
#./sf2brio -i out.brio

# No you don't want the padding from 12 to 16 bytes
#./sf2brio -v -i belltree_up2.aiff -outFormat brio -pad 4 -o outPad4.brio
#./sf2brio -i outPad4.brio

#./sf2brio -i belltree_up2.aiff -outFormat wav  -o out.wav
./sf2brio -i belltree_up2.aiff -outFormat raw  -o out22.raw
#./sf2brio -i belltree_up2.aiff -outFormat aiff -o out.aiff


