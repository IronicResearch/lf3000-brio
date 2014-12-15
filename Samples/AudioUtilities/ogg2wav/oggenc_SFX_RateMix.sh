#!/bin/sh

echo "Ogg encode Lightning content files:  Representative Audio"


# Various data rates
export OGGENCFLAGS='-m  25 -M  25 --resample 4000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "RA_25kb/8x+2yModul_04k_25kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "RA_25kb/CAorbDisintegrate_1_04k_025kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "RA_25kb/changeYourOptions_04k_025kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS -o "RA_25kb/enemyExplosion_04k_25kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "RA_25kb/energyOrbCollected_1_04k_025kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "RA_25kb/HeroShipExplodes_04k_025kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Girl_Looped_04k_025kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Neut_Looped_04k_25kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "RA_25kb/UI_Picker_Placehldr_Looped_04k_025kb.ogg"

export OGGENCFLAGS='-m  25 -M  25 --resample 8000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "RA_25kb/8x+2yModul_08k_25kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "RA_25kb/CAorbDisintegrate_1_08k_025kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "RA_25kb/changeYourOptions_08k_025kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS -o "RA_25kb/enemyExplosion_08k_25kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "RA_25kb/energyOrbCollected_1_08k_025kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "RA_25kb/HeroShipExplodes_08k_025kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Girl_Looped_08k_025kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Neut_Looped_08k_25kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "RA_25kb/UI_Picker_Placehldr_Looped_08k_025kb.ogg"

export OGGENCFLAGS='-m  25 -M  25 --resample 16000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "RA_25kb/8x+2yModul_16k_25kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "RA_25kb/CAorbDisintegrate_1_16k_025kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "RA_25kb/changeYourOptions_16k_025kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS -o "RA_25kb/enemyExplosion_16k_25kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "RA_25kb/energyOrbCollected_1_16k_025kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "RA_25kb/HeroShipExplodes_16k_025kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Girl_Looped_16k_025kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Neut_Looped_16k_25kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "RA_25kb/UI_Picker_Placehldr_Looped_16k_025kb.ogg"

export OGGENCFLAGS='-m  25 -M  25 --resample 24000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "RA_25kb/8x+2yModul_24k_25kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "RA_25kb/CAorbDisintegrate_1_24k_025kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "RA_25kb/changeYourOptions_24k_025kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS -o "RA_25kb/enemyExplosion_24k_25kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "RA_25kb/energyOrbCollected_1_24k_025kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "RA_25kb/HeroShipExplodes_24k_025kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Girl_Looped_24k_025kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Neut_Looped_24k_25kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "RA_25kb/UI_Picker_Placehldr_Looped_24k_025kb.ogg"


export OGGENCFLAGS='-m  25 -M  25 --resample 32000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "RA_25kb/8x+2yModul_32k_25kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "RA_25kb/CAorbDisintegrate_1_32k_025kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "RA_25kb/changeYourOptions_32k_025kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS -o "RA_25kb/enemyExplosion_32k_25kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "RA_25kb/energyOrbCollected_1_32k_025kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "RA_25kb/HeroShipExplodes_32k_025kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Girl_Looped_32k_025kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "RA_25kb/spacShter_Neut_Looped_32k_25kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "RA_25kb/UI_Picker_Placehldr_Looped_32k_025kb.ogg"


