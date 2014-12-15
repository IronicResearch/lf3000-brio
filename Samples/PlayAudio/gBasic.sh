export EXE='/home/lfu/workspace/Brio2/Lightning/Samples/PlayAudio/Build/Lightning_emulation/PlayAudio'

# File format tests
$EXE -v -vol 100 -pan -100 $FLAGS -i belltree_up2_32k.brio 
$EXE -v -vol 100 -pan    0 $FLAGS -i belltree_up2_32k.wav 
$EXE -v -vol 100 -pan  100 $FLAGS -i belltree_up2_32k.ogg
$EXE -v -i Neutr_3_noDrums.mid

