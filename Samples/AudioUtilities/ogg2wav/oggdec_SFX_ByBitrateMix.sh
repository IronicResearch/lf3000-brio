export OGG=./oggi2wav
export OGGSUMMARYFILE=SFX_ByBitrateMix.txt
# -timer -blockTimer -reps 1 -bytesToRead 256 -addSummaryTo a.txt 
export OGGFLAGS='-cpumhz 385 -timer -reps 1 -minout -bytesToRead 256 -addSummaryTo' 

rm -f $OGGSUMMARYFILE

$OGG $OGGFLAGS $OGGSUMMARYFILE BX/8x+2yModul_16k_025kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/8x+2yModul_16k_040kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/8x+2yModul_16k_056kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/8x+2yModul_16k_080kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/8x+2yModul_16k_096kb.ogg 

#$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/UI_Picker_Placehldr_Looped_08k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/UI_Picker_Placehldr_Looped_16k_025kb.ogg 
#$OGG $OGGFLAGS $OGGSUMMARYFILE RA_25kb/UI_Picker_Placehldr_Looped_24k_025kb.ogg 

$OGG $OGGFLAGS $OGGSUMMARYFILE BX/CAorbDisintegrate_1_16k_025kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/changeYourOptions_16k_025kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/enemyExplosion_16k_25kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/energyOrbCollected_1_16k_025kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/HeroShipExplodes_16k_025kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/spacShter_Girl_Looped_16k_025kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/spacShter_Neut_Looped_16k_25kb.ogg 
$OGG $OGGFLAGS $OGGSUMMARYFILE BX/UI_Picker_Placehldr_Looped_16k_025kb.ogg 

