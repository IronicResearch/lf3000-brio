# Encoding
#oggenc BreakItDown_44k_st.wav --bitrate 128 -m 128 -M 128 --managed -o "QB/BreakItDown_44k_st_128kb_CBR.ogg"
#oggenc BreakItDown_44k_st.wav --bitrate 128               --managed -o "QB/BreakItDown_44k_st_128kb_VBR.ogg"


# Nice and even # of blocks at size 256
oggi2wav_O4 -timer -reps 1 -decodeBytesPerBlock 256 -useLogFile T_CBR.txt QB/BreakItDown_44k_st_128kb_CBR.ogg 
oggi2wav_all -timer -reps 1 -decodeBytesPerBlock 256 -useLogFile T_CBR.txt QB/BreakItDown_44k_st_128kb_CBR.ogg 
#oggi2wav -timer -reps 1 -decodeBytesPerBlock 256 -useLogFile T_VBR.txt QB/BreakItDown_44k_st_128kb_VBR.ogg 

# Nice and even # of blocks at size 128
#oggi2wav -timer -reps 1 -bytesToRead 128 -useLogFile TBS128.txt QX/BreakItDown_32k_Q00_st.ogg 

# Nice and even # of blocks at size 64, but MIPS are 50% higher
#oggi2wav -timer -reps 1 -bytesToRead 64 -useLogFile TBS64.txt QX/BreakItDown_32k_Q00_st.ogg 

#valgrind --leak-check=yes oggi2wav -timer -reps 1 -bytesToRead 4096 QX/BreakItDown_32k_Q00_st.ogg 


