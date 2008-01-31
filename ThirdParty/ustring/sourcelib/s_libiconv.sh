#!/bin/bash

#------------------------------------------------------------------------------------------
# Use the "set -e" option at the top of your script so that if it fails at any
#   step it will not proceed.  This offers the proper feedback to the caller to
#   fix the failure
#
	set -e
#---------------------------------------------------------------------------------------

#1) Build libiconv
      echo '***** Build libiconv *****'

      mkdir -p tmp      
      cd 	libiconv-1.12

      ./configure --host=arm-linux
      make 	clean		
      make
      cp 	-a ./lib/.libs/libiconv.so*  ../tmp
      cd 	..
      arm-linux-strip 	tmp/libiconv.so.2.4.0
      cp   -a    tmp/*   ../libs/arm	
      rm   tmp       			 
#      cp ./include/iconv.h $HERE/include-priv/
#      cp ./include/export.h $HERE/include-priv/

