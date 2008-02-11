#!/bin/bash
#	Software components for target hardware:
#	 (Specific number only as examples) 
#	bootstrap-LF_LF1000-0.21.0.2611.lfp 	= kernel/rootfs firmware for LF1000 form-factor board
#	bootstrap-ME_LF1000-0.21.0.2611.lfp 	= bootstrap loader for LF1000 development board
#	firmware-ME_LF1000-0.21.0.2611.lfp		= kernel/rootfs firmware for LF1000 development board
#	firmware-LF_LF1000-0.21.0.2611.lfp		= kernel/rootfs firmware for LF1000 form-factor board
#
#	Brio-0.1.31.2614.lfp			= Brio library firmware for either LF1000 boards
#	host-tools-SVN2614.tar.gz		= Linux host tools subset for installing firmware
#	LightningSDK_2614.tar.gz		    = Lightning SDK for emulation and embedded development
#	nfsroot_SVN2614.tar.gz			= NFS root filesystem for LF1000 development board
#
#-----------------------------------------------------------------------------------------
# 	PREREQUISIE
#	1. To get last LinuxDist
#	   from http://lightning-release/releases/
#          in /home/lfu directory 
#	2. Prepare board for unpack_release.sh script
#
#------------------------------------------------------------------------------------------
	TFTP_PATH=/home/lfu/tftpboot
	TFTP_MAIN=/home/lfu
	VERSION=NUM
	ROOTFS_PATH=/home/lfu/nfsroot
	PATH_TO_BRIO=/home/boris/workspace/Brio2
	S_DIR=~/sdrive

#------------------------------------------------------------------------------------------
# Use the "set -e" option at the top of your script so that if it fails at any
#   step it will not proceed.  This offers the proper feedback to the caller to
#   fix the failure
#
	set 	-e
#---------------------------------------------------------------------------------------
#	cd 		$PATH_TO_BRIO	
#	svn 	up
#------------------------------------------------------------------------------------------
#
#	cd 		$TFTP_MAIN
#	if ! [  -f LinuxDist*.tar.gz  ]; then
#	 	echo 'Copy LinuxDist tarball file ih the /home/lfu from linuxreleases directory' directoty
#		exit 1
#	fi
#------------------------------------------------------------------------------------------
#
# 	Generation release
#
#	cd
#	Create temporary directory to collect files that will be copied on the S: drive
#	mkdir 	-p $S_DIR	

#	Get the current SVN Revision number  
#	
	cd 		$PATH_TO_BRIO
	REV_NUM=$(svn info | grep Revision | cut -c11-14)
	echo "The current SVN Revision number=$REV_NUM"

#----------------------------------------------------------------------------------
#	
	cd 		$TFTP_MAIN
    LINUXDIST_NUM=$(ls LinuxDist-*.tar.gz | cut -c11-21) 
	echo "The current LinuxDist number=$LINUXDIST_NUM"

#	sudo 	tar -xzvf $TFTP_MAIN/LinuxDist-$LINUXDIST_NUM.tar.gz 
#	cd 		$TFTP_MAIN/LinuxDist-$LINUXDIST_NUM/host_tools 
#	./lightning_install.py

#	./unpack_release.sh 

	cp 		$TFTP_MAIN/LinuxDist-$LINUXDIST_NUM/packages/* 		$S_DIR 
    	
    cd  	$ROOTFS_PATH
	sudo 	chown -R boris:boris Didj

	cd 		$TFTP_MAIN/LinuxDist-$LINUXDIST_NUM/host_tools	
	tar 	-czvf HostTools_$REV_NUM.tar.gz backdoor/ \
					flash.map \
					lfpkg README
	mv 		-f HostTools_$REV_NUM.tar.gz $S_DIR
#------------------------------------------------------------------ 
# TO DO:
# To use the "Updated to revision NNNN" svn output in order to
# to edit Etc\Docs\ReleaseProcedure.txt and
# Lightning/meta.inf files
# 	 
#-------------------------------------------------------------------------------------
#
#	Create LightningSDK_XXXX

	cd 		$BRIO_PATH
	rm 		-rf Build/
	rm 		-rf XBuild/
	rm 		-rf System/Temp/
	scons 	type=publish -c
	scons 	type=publish runtests=f
	mv  	-f Publish_$REV_NUM  LightningSDK_$REV_NUM  
    tar 	-czvf LightningSDK_$REV_NUM.tar.gz 	LightningSDK_$REV_NUM
	mv 		-f    LightningSDK_$REV_NUM.tar.gz 	$S_DIR
#   
#	Build tests	
#
		
	rm 		-rf   LightningSDK_$REV_NUM 	
#==============================================================================

	cd 		$ROOTFS_PATH/Didj/Base	
    cp 		$PATH_TO_BRIO/Lightning/meta.inf Brio
	rm 		$ROOTFS_PATH/Didj/Base/*.lfp
	
	$TFTP_MAIN/LinuxDist-$LINUXDIST_NUM/host_tools/lfpkg -a create Brio
	cp 		*.lfp $S_DIR	
#==============================================================================
#	Creating package for updating ATAP

	rm		$TFTP_MAIN//LinuxDist-$LINUXDIST_NUM/mfg-cart/ATAP/Packages/*.lfp
	cp		*.lfp	$TFTP_MAIN//LinuxDist-$LINUXDIST_NUM/mfg-cart/ATAP/Packages				
    cd 	 	$TFTP_MAIN//LinuxDist-$LINUXDIST_NUM/mfg-cart
	tar     -czvf	ATAP_SVN$REV_NUM.tar.gz		ATAP
	mv 		-f 		ATAP_SVN$REV_NUM.tar.gz		$S_DIR
#==============================================================================
#	Tar nfsroot directory

	cd 		$TFTP_MAIN
	sudo 	tar 	-czvf 	nfsroot_SVN$REV_NUM.tar.gz nfsroot
	sudo 	mv  	-f  	nfsroot_SVN$REV_NUM.tar.gz $S_DIR
#==============================================================================
#   svn commit Lightning/ReleaseNotes.txt Lightning/meta.inf -m "Updating revesion numbers"
#	rm 		-rf $S_DIR

