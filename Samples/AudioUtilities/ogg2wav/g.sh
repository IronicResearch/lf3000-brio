export OGG=./oggi2wav
export OGGSUMMARYFILE=BreakItDown_ByQ.txt

# -timer -blockTimer -reps 1 -bytesToRead 256 -addSummaryTo IslandByQ.txt 
export OGGFLAGS='-timer -blockTimer -reps 1 -bytesToRead 256 -addSummaryTo' 
#valgrind --leak-check=yes oggi2wav -timer -reps 1 -bytesToRead 4096 QX/BreakItDown_32k_Q00_st.ogg 

rm -f $OGGSUMMARYFILE

$OGG $OGGFLAGS $OGGSUMMARYFILE -o out.wav QX/BreakItDown_32k_Q00_st.ogg 
#$OGG $OGGFLAGS  QX/BreakItDown_32k_Q00_st.ogg 
# 



