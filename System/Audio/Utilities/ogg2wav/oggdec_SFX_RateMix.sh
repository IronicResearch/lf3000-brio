export OGG=./oggi2wav
export OGGSUMMARYFILE=SFX_RateMix.txt
# -timer -blockTimer -reps 1 -bytesToRead 256 -addSummaryTo a.txt 
export OGGFLAGS='-cpumhz 385 -timer -reps 1 -bytesToRead 256 -addSummaryTo' 

rm -f $OGGSUMMARYFILE

$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/8x+2yModul_08k_25kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/8x+2yModul_16k_25kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/8x+2yModul_24k_25kb.ogg 

#$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/UI_Picker_Placehldr_Looped_08k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/UI_Picker_Placehldr_Looped_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/UI_Picker_Placehldr_Looped_24k_025kb.ogg 

#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/CAorbDisintegrate_1_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/changeYourOptions_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/enemyExplosion_16k_25kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/energyOrbCollected_1_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/HeroShipExplodes_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/spacShter_Girl_Looped_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/spacShter_Neut_Looped_16k_25kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE BX/UI_Picker_Placehldr_Looped_16k_025kb.ogg 

