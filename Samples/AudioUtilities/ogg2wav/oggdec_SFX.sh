export OGG=./oggi2wav
export OGGSUMMARYFILE=SFX_ByBitrate.txt
# -timer -blockTimer -reps 1 -bytesToRead 256 -addSummaryTo FILE.txt 
export OGGFLAGS='-cpumhz 385 -timer -reps 3 -bytesToRead 256 -addSummaryTo' 

rm -f $OGGSUMMARYFILE

$OGG $OGGFLAGS $OGGSUMMARYFILE BX/8x+2yModul_16k_25kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/CAorbDisintegrate_1_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/changeYourOptions_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/enemyExplosion_16k_25kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/energyOrbCollected_1_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/HeroShipExplodes_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/spacShter_Girl_Looped_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/spacShter_Neut_Looped_16k_25kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/UI_Picker_Placehldr_Looped_16k_025kb.ogg 

