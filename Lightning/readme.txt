======================================================
Lightning SDK
======================================================

Lightning SDKs typically consist of these parts:

  LightningSDK_xxxx.tar.gz -Brio SDK components necessary for development.
  embedded-svnxxxx.tar.gz - embedded binaries to be flashed onto the target board.
  basebrio-svnxxxx.tar.gz - Brio binaries to be copied onto target NAND partition via USB. 
  nfsroot-svnxxxx.tar.gz  - optional nfsroot folder for booting the target board.
                            
The SDK can be unzipped as is and located anywhere.

	tar -xzvf LightningSDK_xxxx.tar.gz
	export LEAPFROG_PLUGIN_ROOT=/path/to/LightningSDK_<xxxx>

The embedded binary images need to be copied to your TFTP server base directory, and 
subsequently downloaded onto the target board via the U-Boot loader. (Refer to image
flashing instructions below.)

	tar -xzvf embedded-svnxxxx.tar.gz
	cp kernel-xxxx ~/tftpboot
	cp erootfs-xxxx ~/tftpboot
	cp lightning_boot-xxxx ~/tftpboot
	cp lightning_install.py ~/tftpboot

The NFS root image must be unzipped as root user and located off the home user path.

	sudo tar -xzvf nfsroot-svnxxx.tar.gz
	mv nfsroot-svnxxx nfsroot
	
These pieces MUST be used together!  The <xxxx> numbers for a release will generally be
the same for all three pieces.  Either way, make sure that you use only the pieces from a single 
release together or you will almost certainly have problems.

======================================================
*** IMPORTANT NOTE ***: 
======================================================

The embedded target binaries are significantly different from previous releases. 
This release now uses a fully embedded filesystem, with the option to expose
NAND flash partitions as USB mounted devices.

These binaries consist of the following:

	lightning_boot-xxxx.bin	- Lightning boot loader (replaces uniboot)
	kernel-xxxx.jffs2		- Linux kernel image (replaces zImage and u-boot)
	erootfs-xxxx.jffs2		- Embedded root filesystem (alternative to nfsroot)

All 3 binaries need to be flashed into new locations described below. 

The NFS root filesystem may be used for continued development as alternative to
the embedded root filesystem. You will need to set a flag file after booting the 
embedded target to switch booting back to the NFS root filesystem.
	
======================================================
Installation
======================================================

Unarchive this file to create a distribution folder.
Use any target folder to which you have user access on your system.

The installation tree will look more or less like this:

+-ReleaseNotes.txt
|
+-Include  (Brio MPI headers)
|  |
|  +-boost (preprocessor library, auto-generates text descriptions of errors at compile time)
|  |
|  +-OpenGL
|  |  |
|  |  +-Lighting           (subfolders for MagicEyes headers--not in release 0.1.1)
|  |  |
|  |  +-Lighting_emulation (subfolders for PowerVR headers)
|  |
|  +-ThirdParty
|     |
|     +-ustring            (for embedded CString class, should disappear in subsequent release)
|
+-Libs  (Brio MPI headers)
|  |
|  +-Lightning
|  |  |
|  |  +-MPI       (module interface objects, directly linked to by apps)
|  |  |
|  |  +-OpenGL    (MagicEyes library)
|  |
|  +-Lightning_emulation
|     |
|     +-Module    (Brio replacable modules, not linked to by apps)
|     |
|     +-MPI       (Brio module interface objects, directly linked to by apps)
|     |
|     +-OpenGL    (PowerVR library)
|     |
|     +-PrivMPI   (module interface objects for Brio modules, not linked to by apps)
|     |
|     +-ThirdParty(Ogg, Vorbis, and Theora support libraries)
|  
+-Samples
|  |
|  +-BrioCube     (rotating cube with Brio calls that are exercised through button presses)
|  |
|  +-BrioVideo    (video playback demo to YUV video surface layer)
|  |
|  +-BrioAudio    (audio playback demo for testing mixer channels)
|  |
|  +-Button       (demonstrates button presses on the hardware)
|  |
|  +-Display      (demonstrates 2D RGB display surface layer and fonts)
|  |
|  +-Simple       (simplest OpenGL application, minimal build system)
|  |
|  +-SysAppDemo   (system application demo for handling USB and Power events)
|
+-Tools
   |
   +-*.py         (SCons modules for C++ build system)

To point the build system to the SDK, set the environment variable LEAPFROG_PLUGIN_ROOT to point 
to the location of your SDK folder.  At the command line you can type:

	export LEAPFROG_PLUGIN_ROOT=/path/to/LightningSDK_<xxxx>

You may also want to add this to your .bashrc file in the lfu home directory so it will always 
be setup:
	
	export LEAPFROG_PLUGIN_ROOT=/home/lfu/LightningSDK_<xxxx>

======================================================
Target Preparation : mounting NAND partition via USB
======================================================

In order to install application binaries and resources onto the embedded target board,
a designated NAND partition is available for mounting as a USB mass storage device on
the host development system.

Internally the NAND partition is mounted at bootup as '/Didj'.

	mount -t vfat -o sync /dev/mtdblock9 /Didj
	
To expose the NAND partition to the development system:

	usbctl -d mass_storage -a enable
	usbctl -d mass_storage -a unlock

Connecting a USB cable to the embedded target board should make the board appear as a
mounted USB device. On Ubuntu Linux, the mounted device name is '/media/disk'. This
name should be set as the root filesystem path for the Lightning SDK.

	export ROOTFS_PATH=/media/disk   

Once mounted, the Brio binaries will need to copied onto the exposed NAND partition.
This is intended to begin populating the /Didj directory after the board is reflashed.
On the Ubuntu development system side:

	tar -xzvf basebrio.tar.gz 
	cp -R Base/* /media/disk/  

Embedded target application binaries and resources will then be copied to their
respective subdirectory locations on the mounted NAND partition. When copying files
is completed and it is time to disconnect the USB cable, be sure to unmount (eject)
the mounted device on the host side before disabling it from the target.  

To remove the NAND partition from the development system:
	 
	usbctl -d mass_storage -a lock 
	usbctl -d mass_storage -a disable

Note that at this stage in the USB device interface's development, some extra steps
may need to be taken to insure synchronization of the copied files onto the NAND
flash medium. For the time being you may have to issue explicit sync, or unmount and
remount commands to see the NAND partition updated at '/Didj'.

	sync 
	ls /Didj
	
	umount /Didj
	mount -t vfat -o sync /dev/mtdblock9 /Didj
	ls /Didj
	
======================================================
Target Preparation : booting to NFS root filesystem 
======================================================

In situations it might be desirable to revert to booting the root filesystem over NFS
instead of from NAND. Use of debugger and profiling tools which cannot fit into NAND
partition would be examples.

You would need to set a flag file on the embedded target to switch between booting from
NAND root filesystem to NFS mounted root filesystem. 

To examine the current root filesystem setting:

	cat /flags/rootfs

To set NFS root filesystem booting:

	echo NFS0 > /flags/rootfs
	
Restarting the embedded target should now complete booting at the NFS mounted location
at Ethernet IP address 192.168.0.113 as previous releases (described below).

In order to see files copied to the '/Didj' directory at the NFS mounted location,
you will need to unmount the NAND partition.

	umount /Didj
	ls /Didj

When building SDK apps, be sure ROOTFS_PATH is pointing to the NFS path on the
development system.

	export ROOTFS_PATH=/home/lfu/nfsroot

Embedded target application binaries and resources will then be copied to their
respective subdirectory locations on the development system's NFS rootfs directory.

======================================================
Target Preparation : installing nfsroot image
======================================================

If you are running on actual target hardware, you should probably have received
an updated root file system with this code drop.  It should have been called
something like nfsroot-svnxxx.tar.gz.  To install it, extract it to the
directory that you export via NFS to the target.  This directory is probably
/home/lfu/nfsroot.  Note that you must be root to successfully untar the
rootfs!!  The reason is that it must create device nodes.

	sudo tar -xzvf nfsroot-svnxxx.tar.gz
	mv nfsroot-svnxxx nfsroot

The environment variable ROOTFS_PATH should be set to point to the nfsroot path
so the SDK build scripts will know where to install application binaries and resources.

	export ROOTFS_PATH=/home/lfu/nfsroot

======================================================
Target Preparation -- setting up NFS
======================================================

Obviouslly, you only need to do these steps if you system hasn't already 
been configured.

Your development system image will also need to have NFS server installed
and running, and one network adapter configured at the fixed IP address
192.168.0.113. 

	sudo apt-get install nfs-user-server
	sudo /etc/init.d/nfs-user-server start
	
	ifconfig eth1 192.168.0.113 up

The network IP address configuration should be put in a startup script. 
Note the development system used with Lightning testboard used 'xinetd' 
instead of 'inetd'.

	sudo apt-get install xinetd
	sudo /etc/init.d/xinetd start

The Lightning test board will be flashed with a Linux kernel image which 
will attempt to NFS mount its root filesystem at this IP address. The test 
board will be configured to use IP address 192.168.0.111, so this may need 
to be added to list of allowable addresses in /etc/hosts.allow if firewall
iptables service is running.

Configure the nfs server using your favorite text editor:

Add these lines to the end of the file /etc/hosts.allow:

	ALL: 192.168.0.111

Add these lines to the end of the file /etc/hosts.deny

	ALL: ALL

Add this line to the end of the file /etc/exports

	/home/lfu/nfsroot 192.168.0.111(rw,no_root_squash,async)

Now restart the the NFS server:

   $ sudo /etc/init.d/nfs-kernel-server restart



======================================================
Target Preparation -- setting up TFTP
======================================================

1. Install tftp and related packages.

	sudo apt-get install tftpd tftp


2. Create /etc/xinetd.d/tftp and put this entry:

	sudo vi /etc/xinetd.d/tftp
	
service tftp
{
protocol    = udp
port        = 69
socket_type = dgram
wait        = yes
user        = lfu
server      = /usr/sbin/in.tftpd
server_args = /home/lfu/tftpboot
disable     = no
}


3. Make /home/lfu/tftpboot directory.

	mkdir -p /home/lfu/tftpboot
	chmod -R 777 /home/lfu/tftpboot
	
Note some previous release notes and wiki notes may have referred to 'tftproot' as
the TFTP server home directory. External software developers may continue to use
that name (or any other name) for their TFTP directory for consistancy. (Internal
firmware developers need to use 'tftpboot' for automated script support.) 


4. Ensure /etc/hosts.allow file allows computers on 192.168.0.0 network access.

# /etc/hosts.allow: list of hosts that are allowed to access the system.
#                   See the manual pages hosts_access(5), hosts_options(5)
#                   and /usr/doc/netbase/portmapper.txt.gz
#
# Example:    ALL: LOCAL @some_netgroup
#             ALL: .foobar.edu EXCEPT terminalserver.foobar.edu
#
# If you're going to protect the portmapper use the name "portmap" for the
# daemon name. Remember that you can only use the keyword "ALL" and IP
# addresses (NOT host or domain names) for the portmapper, as well as for
# rpc.mountd (the NFS mount daemon). See portmap(8), rpc.mountd(8) and 
# /usr/share/doc/portmap/portmapper.txt.gz for further information.
#
ALL:192.168.0.0/24


5. Start tftpd through xinetd

	sudo /etc/init.d/xinetd restart


6. Testing. Transferring file hda.txt to server

	cd ~lfu
	touch hda.txt
	tftp 192.168.0.113

	tftp> put hda.txt
	tftp> quit

	ls -l /home/lfu/tftpboot
	-rw------- 1 lfu lfu 0 2007-06-04 12:10 hda.txt


7. Troubleshooting. Check the /var/log/syslog file for tftp error messages.

	tail -f /var/log/syslog

======================================================
Target Preparation -- downloading kernel image manually
======================================================

With the network services up and running for the specific development system
IP address, you will also be able to use the U-boot loader installed on
the Lightning target board to download updated Linux kernel images.

You will need to have a serial console connected to interrupt the target
board's normal boot sequence and run U-boot commands. 

To boot up U-boot, hold down any button on the target board while pressing 
the RESET button. When the "U-boot" loader message appears, press any key
on the serial console to enter U-boot. Otherwise the board will continue
loading its pre-flashed Linux kernel.

At the U-boot prompt, test pinging the development system first.

	ping 192.168.0.113
	
Then download the updated kernel images from its TFTP location on the
development system. On Ubuntu Linux, TFTP is typically configured
at the ~/tftpboot directory, so this is where the image files need to
be copied to. The image files provided in releases will typically have
some version number 'XXXX', which is something like '0.8.0-1888-ME_LF1000'.

In this release, all new binaries need to be flashed, and it is advisable
to erase all flash memory first.

	nand erase
	
To download and flash the new boot loader:

	tftp 02000000 lightning-boot-XXXX.bin
	nand write 02000000 0 1800
	
To download and flash the new kernel image:

	tftp 02000000 kernel-XXXX.jffs2
	nand write 02000000 00200000 160000
	nand write 02000000 01200000 160000

To download and flash the new embedded root filesystem image:

	tftp 02000000 erootfs-XXXX.jffs2
	nand write 02000000 00400000 520000
	nand write 02000000 01400000 520000

Once all components are flashed, reboot the system by pressing the RESET
button. You should see Linux boot messages on the serial console when
flashing was successful.

======================================================
Target Preparation -- flashing kernel image via script
======================================================

A Python script is provided for automating the U-boot commands externally from the
development system side. You will need the Python serial package installed on your
development system.

	sudo apt-get install python-serial

You will also need a copy of the Python script in the same TFTP directory as
the firmware binaries (to simplify file paths).

	cp host_tools/lightning_install.py ~/tftpboot 

Reboot the embedded target board to the U-boot prompt as above. That is, hold one of
the buttons down when powering on or pressing the RESET button. Then instead of
entering the U-boot commands manually, close the serial terminal and run the
following script from the development system.

	cd ~/tftpboot
	./lightning_install.py /dev/ttyS0 -e lightning-boot-XXXX.bin:0 \ 
			kernel-XXXX.jffs2:200000 erootfs-XXXX.jffs2:400000 \
			kernel-XXXX.jffs2:1200000 erootfs-XXXX.jffs2:1400000 

'XXXX' refers to the version number of the release, such as '0.8.0-1888-ME_LF1000'
for the LF1000 green development board, or '0.8.0-1888-LF_LF1000' for the LF1000
form-factor board.

The '-e' option is recommended in this release to erase all flash memory.

This script will update all 3 major firmware components, including the boot loader,
the Linux kernel image (with U-boot), and the embedded root filesystem. All of these
components have changed significantly since previous releases, including their names
and (more importantly) their locations! 
 
======================================================
Target Preparation -- installing ARM cross-compiler
======================================================

Your development system image will also need to have the cross-compiler
installed for building ARM binary targets. The version used to date is
scratchbox toolchain is based on GCC 4.1 with uclib run-time library. 

You will need root privileges for this part of the Scratchbox installation.

   1. Add the line below to the /etc/apt/sources.list file:	    
   
		deb http://scratchbox.org/debian ../download/files/sbox-releases/stable/deb
 
   		(convenient way to edit file: sudo gedit /etc/apt/sources.list) 

   2. Update the package list with command:

      # sudo apt-get update

   3. Install packages:

      # sudo apt-get install scratchbox-core scratchbox-libs
      # sudo apt-get install scratchbox-toolchain-arm-gcc4.1-uclibc20061004
      # sudo /scratchbox/sbin/sbox_adduser <yourusername>

Note you may need to modify the CC compiler directive in the arm-g++.py
script to use the explicit full path to arm-linux-g++. Or put this:

export PATH=$PATH:/scratchbox/compilers/arm-gcc4.1-uclibc20061004/bin/

in your .bashrc
	
See ReleaseNotes.txt for important information about which versions of other
software components are required on the target.

======================================================
Running samples  -- Emulation
======================================================
From your Eclipse Workspace:
1) Create a new C++ project. 
   a) Right Click in the Project explorer and choose "New Project"
   b) Select C++ -> C++ Project
   c) Give the project a name -- try "BrioCube" for this example.
   d) Choose "Makefile Project" from the list of Project Types.
   e) Leave --Other Toolchain-- selected
   b) Click Finish.

 The default build command is "scons -k type=emulation" assuming you are 
      starting by running the emulation target.  If you are running on the 
      actual hardware, enter "scons runtests=f deploy_dir=</path/to/your/nfsroot/>".  
      Your nfsroot is wherever you chose to untar the nfsroot, probably 
      /home/lfu/nfsroot/.
	  The environment variable ROOTFS_PATH will be used instead if it is defined:
	  export ROOTFS_PATH=/home/lfu/nfsroot

2) Copy the contents of one of the sample projects into your newly created project folder
   (you may need to use the File Browser to do this).  You can drag the files from the ubuntu
   file browser directly into the Project folder you've created in eclipse.
3) In Eclipse, open the "SConstruct" file.  On line 36, change "cdevkit_dir = '<install_dir>'"
   to the root location where you installed this distribution. The environment variable
   LEAPFROG_PLUGIN_ROOT will be used instead if it is defined.
6) From the Eclipse "Project" menu, turn off "Build Automatically".
7) Right Click on your project folder and select "Build Project"
7) If you are running the emulation target, open the newly created
   "Build/Lighting_emulation" folder.  You should see "BrioCube" there.
   Right click on "BrioCube", and select "Run As | Run Local C/C++
   Application".  Click through any dialogs you need to.  That's it!

======================================================
Running samples  -- Target
======================================================
1) Right Click on your project folder and select "Properties"
2) Change "scons type=emulation" to "scons type=embedded"
3) Rebuild the project. The executable and any associated resources will be
   automatically copied into the appropriate nfsroot directories.
3) If you are running on the actual hardware, you should just be able to run
   the command "BrioCube" in the serial console.

