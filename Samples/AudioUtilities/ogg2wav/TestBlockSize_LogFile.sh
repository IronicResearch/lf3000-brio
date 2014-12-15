#oggi2wav -timer -reps 1 -bytesToRead 32768 -useLogFile TBS32768.txt QX/BreakItDown_32k_Q00_st.ogg 

#oggi2wav -timer -reps 1 -bytesToRead 16384 -useLogFile TBS16384.txt QX/BreakItDown_32k_Q00_st.ogg 

#oggi2wav -timer -reps 1 -bytesToRead 8192 -useLogFile TBS8192.txt QX/BreakItDown_32k_Q00_st.ogg 

#oggi2wav -timer -reps 1 -bytesToRead 4096 -useLogFile TBS4096.txt QX/BreakItDown_32k_Q00_st.ogg 

#oggi2wav -timer -reps 1 -bytesToRead 2048 -useLogFile TBS2048.txt QX/BreakItDown_32k_Q00_st.ogg 

#oggi2wav -timer -reps 1 -bytesToRead 1024 -useLogFile TBS1024.txt QX/BreakItDown_32k_Q00_st.ogg 

#oggi2wav -timer -reps 1 -bytesToRead 512 -useLogFile TBS512.txt QX/BreakItDown_32k_Q00_st.ogg 

# Somewhat even, decodes at sizes 128 and 256
#oggi2wav -timer -reps 1 -bytesToRead 384 -useLogFile TBS384.txt QX/BreakItDown_32k_Q00_st.ogg 

# Nice and even # of blocks at size 256
oggi2wav -timer -reps 1 -bytesToRead 256 -useLogFile TBS256.txt QX/BreakItDown_32k_Q00_st.ogg 

# Nice and even # of blocks at size 128
#oggi2wav -timer -reps 1 -bytesToRead 128 -useLogFile TBS128.txt QX/BreakItDown_32k_Q00_st.ogg 

# Nice and even # of blocks at size 64, but MIPS are 50% higher
#oggi2wav -timer -reps 1 -bytesToRead 64 -useLogFile TBS64.txt QX/BreakItDown_32k_Q00_st.ogg 

#valgrind --leak-check=yes oggi2wav -timer -reps 1 -bytesToRead 4096 QX/BreakItDown_32k_Q00_st.ogg 


