export APP=./oggi2wav
#export APP=/usr/local/bin/oggi2wav
# timer -blockTimer

$APP -cpumhz 385 -timer -reps 1 -bytesToRead 256 BX/BreakItDown_32k_st_056kb.ogg
$APP -cpumhz 385 -timer -reps 1 -bytesToRead 256 BX/BreakItDown_32k_st_096kb.ogg
$APP -cpumhz 385 -timer -reps 1 -bytesToRead 256 BX/BreakItDown_32k_st_128kb.ogg

#$APP -cpumhz 385 -timer -reps 1 -bytesToRead 256 QX/BreakItDown_32k_Q00_mono.ogg 
#$APP -cpumhz 385 -timer -reps 1 -bytesToRead 256 QX/BreakItDown_32k_Q00_st.ogg 

#  -blockTimer
#valgrind --leak-check=yes oggi2wav -timer -reps 1 -bytesToRead 4096 QX/BreakItDown_32k_Q00_st.ogg 


