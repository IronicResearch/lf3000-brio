#!/bin/sh

echo "Ogg encode Lightning content files:  Representative Audio"


# Various data rates
export OGGENCFLAGS='-m  25 -M  25 --resample 16000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "BX/8x+2yModul_16k_025kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "BX/CAorbDisintegrate_1_16k_025kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "BX/changeYourOptions_16k_025kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS-o "BX/enemyExplosion_16k_025kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "BX/energyOrbCollected_1_16k_025kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "BX/HeroShipExplodes_16k_025kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Girl_Looped_16k_025kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Neut_Looped_16k_025kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "BX/UI_Picker_Placehldr_Looped_16k_025kb.ogg"

export OGGENCFLAGS='-m  40 -M  40 --resample 16000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "BX/8x+2yModul_16k_040kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "BX/CAorbDisintegrate_1_16k_040kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "BX/changeYourOptions_16k_040kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS-o "BX/enemyExplosion_16k_040kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "BX/energyOrbCollected_1_16k_040kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "BX/HeroShipExplodes_16k_040kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Girl_Looped_16k_040kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Neut_Looped_16k_040kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "BX/UI_Picker_Placehldr_Looped_16k_040kb.ogg"

export OGGENCFLAGS='-m  56 -M  56 --resample 16000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "BX/8x+2yModul_16k_056kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "BX/CAorbDisintegrate_1_16k_056kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "BX/changeYourOptions_16k_056kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS-o "BX/enemyExplosion_16k_056kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "BX/energyOrbCollected_1_16k_056kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "BX/HeroShipExplodes_16k_056kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Girl_Looped_16k_056kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Neut_Looped_16k_056kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "BX/UI_Picker_Placehldr_Looped_16k_056kb.ogg"

export OGGENCFLAGS='-m  80 -M  80 --resample 16000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "BX/8x+2yModul_16k_080kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "BX/CAorbDisintegrate_1_16k_80kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "BX/changeYourOptions_16k_080kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS-o "BX/enemyExplosion_16k_080kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "BX/energyOrbCollected_1_16k_080kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "BX/HeroShipExplodes_16k_080kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Girl_Looped_16k_080kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Neut_Looped_16k_080kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "BX/UI_Picker_Placehldr_Looped_16k_080kb.ogg"

export OGGENCFLAGS='-m  96 -M  96 --resample 16000' 
oggenc RA/8x+2yModul.aif            $OGGENCFLAGS -o "BX/8x+2yModul_16k_096kb.ogg"
oggenc RA/CAorbDisintegrate_1.aif   $OGGENCFLAGS -o "BX/CAorbDisintegrate_1_16k_96kb.ogg"
oggenc RA/changeYourOptions.aif     $OGGENCFLAGS -o "BX/changeYourOptions_16k_096kb.ogg"
oggenc RA/enemyExplosion.aif        $OGGENCFLAGS-o "BX/enemyExplosion_16k_096kb.ogg"
oggenc RA/energyOrbCollected_1.aif  $OGGENCFLAGS -o "BX/energyOrbCollected_1_16k_096kb.ogg"
oggenc RA/HeroShipExplodes.aif      $OGGENCFLAGS -o "BX/HeroShipExplodes_16k_096kb.ogg"
oggenc RA/spacShter_Girl_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Girl_Looped_16k_096kb.ogg"
oggenc RA/spacShter_Neut_Looped.aif $OGGENCFLAGS -o "BX/spacShter_Neut_Looped_16k_096kb.ogg"
oggenc RA/UI_Picker_Placehldr_Looped.aif $OGGENCFLAGS -o "BX/UI_Picker_Placehldr_Looped_16k_096kb.ogg"





