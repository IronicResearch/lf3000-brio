#!/bin/sh

echo "Ogg encode: many just don't encode"

#oggenc noise05_08k.wav -m  64 -M  64 -o "NOISE/noise05_08k_064kb.ogg"

cp noise05_08k.wav NOISE/noise05_08k.wav
cp noise05_16k.wav NOISE/noise05_16k.wav
cp noise05_32k.wav NOISE/noise05_32k.wav

oggenc noise05_08k.wav -b  32 --managed -o "NOISE/noise05_08k_032kb.ogg"
oggenc noise05_08k.wav -b  40 --managed -o "NOISE/noise05_08k_040kb.ogg"
oggenc noise05_08k.wav -b  56 --managed -o "NOISE/noise05_08k_056kb.ogg"
oggenc noise05_08k.wav -b  64 --managed -o "NOISE/noise05_08k_064kb.ogg"
oggenc noise05_08k.wav -b 128 --managed -o "NOISE/noise05_08k_128kb.ogg"
oggenc noise05_08k.wav -b 192 --managed -o "NOISE/noise05_08k_192kb.ogg"

oggenc noise05_16k.wav -b  64 --managed -o "NOISE/noise05_16k_064kb.ogg"
oggenc noise05_16k.wav -b 128 --managed -o "NOISE/noise05_16k_128kb.ogg"
oggenc noise05_16k.wav -b 192 --managed -o "NOISE/noise05_16k_192kb.ogg"

oggenc noise05_32k.wav -b  64 --managed -o "NOISE/noise05_32k_064kb.ogg"
oggenc noise05_32k.wav -b 128 --managed -o "NOISE/noise05_32k_128kb.ogg"
oggenc noise05_32k.wav -b 192 --managed -o "NOISE/noise05_32k_192kb.ogg"

sf2brio -outFormat brio noise05_08k.wav NOISE/noise05_08k.brio
sf2brio -outFormat brio noise05_16k.wav NOISE/noise05_16k.brio
sf2brio -outFormat brio noise05_32k.wav NOISE/noise05_32k.brio

