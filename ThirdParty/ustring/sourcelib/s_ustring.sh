#!/bin/bash
#*****************************************************************************
#	This script sceleton to create all library from scratch according to
#	 README	in the ../ derectory	 
#
#*******************************************************************************

HERE=/home/boris/investigation/ustring

#------------------------------------------------------------------------------------------
# Use the "set -e" option at the top of your script so that if it fails at any
#   step it will not proceed.  This offers the proper feedback to the caller to
#   fix the failure
#
	set -e
#---------------------------------------------------------------------------------------
	
	if ! [  -d 	$HERE/libs  ]; then
		mkdir $HERE/libs		
	fi	

	if ! [  -d 	$HERE/libs/arm  ]; then
		mkdir $HERE/libs/arm		
	fi	

	if ! [  -d 	$HERE/include-priv  ]; then
		mkdir $HERE/include-priv		
	fi	

	if ! [  -d 	$HERE/libcharset  ]; then
		mkdir $HERE/libcharset		
	fi	

#1) Build libiconv
	echo '****************************** Build libiconv '

#	if ! [ -d libiconv-1.12 ]; then
#      		wget http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.12.tar.gz
#		tar -xvzf libiconv-1.12.tar.gz
# 	fi

      cd 	libiconv-1.12
#      make 	distclean		
      ./configure --host=arm-linux
      make 	clean		
      make
      cp -a ./lib/.libs/libiconv.so* $HERE/libs/arm
      cp ./include/iconv.h $HERE/include-priv/
      cp ./include/export.h $HERE/include-priv/
      cd ..   

#2) Build gettext.  The no-error-print-program patch eliminates the use of a
#   function defined in gnuc but not in uclibc (which we are using).

	echo '****************************** Build gettext '

	if ! [ -d gettext-0.16 ]; then
	      	wget ftp://ftp.gnu.org/pub/gnu/gettext/gettext-0.16.tar.gz
		tar -xvzf gettext-0.16.tar.gz 
	fi

     cd gettext-0.16
      make clean	
#      patch -p1 < $HERE/patches/no-error-print-program.patch 
#      ./configure --host=arm-linux
      make
      cp -a ./gettext-tools/intl/.libs/libintl.so* $HERE/libs/arm
      cp ./gettext-tools/intl/libintl.h $HERE/include-priv/
      cd ..   

#3) Build glib.  Sigh.  All we need is libcharset, which I believe is just
#   libiconv's libcharset in a glib wrapper.  It probably would be easy to
#   eliminate this step.  The patch fixes a problem with the config file when
#   cross compiling.

	echo '****************************** Build glib '

	if ! [ -d glib-2.4.8 ]; then
		wget ftp://ftp.gtk.org/pub/gtk/v2.4/glib-2.4.8.tar.bz2
   		tar -xjf glib-2.4.8.tar.bz2
	fi
#      export CPPFLAGS="-I$HERE/libiconv-1.12/include/ -I$HERE/gettext-0.16/gettext-tools/intl/"
#      export LDFLAGS="-L$HERE/libiconv-1.12/lib/.libs/ -L$HERE/gettext-0.16/gettext-tools/intl/.libs/"
      export CPPFLAGS="-I$HERE/libiconv-1.12/include/ -I$HERE/gettext-0.16/gettext-tools/intl/"
      export LDFLAGS="-L$HERE/libiconv-1.12/lib/.libs/ -L$HERE/gettext-0.16/gettext-tools/intl/.libs/"
      
      cd glib-2.4.8
      make clean
      patch -p1 < ../patches/glib-no-fail-crosscompile.patch
      ./configure --host=arm-linux glib_cv_uscore=no ac_cv_func_posix_getpwuid_r=yes
      cp configure glib-2.4.8
      make -C glib/libcharset
      cp -a ./glib/libcharset/.libs/libcharset.a $HERE/libs/arm
      mkdir -p $HERE/libcharset
      cp ./glib/libcharset/libcharset.h 	$HERE/libcharset/

#4) Build libsigc++:

	echo '****************************** Build libsigc++ '
	if ! [ -d libsigc++-2.0.17 ]; then
      		wget http://ftp.gnome.org/pub/GNOME/sources/libsigc++/2.0/libsigc++-2.0.17.tar.bz2
		tar xvjf libsigc++-2.0.17.tar.bz2
	fi      

        cd libsigc++-2.0.17/
      make clean
#      ./configure --host=arm-linux
      make
      cp -a ./sigc++/.libs/libsigc*.so* 	$HERE/libs/arm
      cp ./sigc++config.h 	$HERE
      mkdir -p $HERE/sigc++/adaptors/lambda
      mkdir -p $HERE/sigc++/functors
      for f in `find sigc++ -name "*.h"`; do cp -ra $f $HERE/$f; done
      cd ..

