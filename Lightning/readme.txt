=============
Lightning SDK
=============

============
Installation
============

Unarchive this file to create a distribution folder.
Use any target folder to which you have user access on your system.

The installation tree will look something like this:

+-readme.txt         (this file)
|
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
|  +-Lighting
|  |  |
|  |  +-MPI       (module interface objects, directly linked to by apps)
|  |  |
|  |  +-OpenGL    (MagicEyes library)
|  |
|  +-Lighting_emulation
|     |
|     +-Module    (Brio replacable modules, not linked to by apps)
|     |
|     +-MPI       (Brio module interface objects, directly linked to by apps)
|     |
|     +-OpenGL    (PowerVR library)
|     |
|     +-PrivMPI   (module interface objects for Brio modules, not linked to by apps)
|  
+-Samples
|  |
|  +-BrioCube     (rotating cube with Brio calls that are exercised through button presses)
|  |
|  +-Button       (demonstrates button presses on the hardware)
|  |
|  +-Display      (demonstrates how to load and use fonts)
|  |
|  +-Simple       (simplest OpenGL application, minimal build system)
|
+-Tools
   |
   +-*.py         (SCons modules for C++ build system)


==================
Target Preparation
==================

If you are running on actual target hardware, you should probably have received
an updated root file system with this code drop.  It should have been called
something like nfsroot-svnxxx.tar.gz.  To install it, extract it to the
directory that you export via NFS to the target.  This directory is probably
/home/lfu/nfsroot.  Note that you must be root to successfully untar the
rootfs!!  The reason is that it must create device nodes.

	sudo tar -xzvf nfsroot-svnxxx.tar.gz
	mv nfsroot-svnxxx nfsroot

Your development system image will also need to have NFS server installed
and running, and one network adapter configured at the fixed IP address
192.168.0.113. 

	sudo apt-get installl nfs-kernel-server
	sudo /etc/init.d/nfs-kernel-server start
	
	ifconfig eth1 192.168.0.113 up
	sudo /etc/init.d/inetd start

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

Your development system image will also need to have the cross-compiler
installed for building ARM binary targets. The version used to date is
scratchbox toolchain is based on GCC 4.1 with uclib run-time library. 

You will need root privileges for this part of the Scratchbox installation.

   1. Add the line below to the /etc/apt/sources.list file:	    
   
 			deb http://scratchbox.org/debian ./
 
   			(convenient way to edit file: sudo gedit /etc/apt/sources.list) 

   2. Update the package list with command:

      # sudo apt-get update

   3. Install packages:

      # sudo apt-get install scratchbox-toolchain-arm-gcc4.1-uclibc20061004

Note you may need to modify the CC compiler directive in the arm-g++.py
script to use the explicit full path to arm-linux-g++.
	
See ReleaseNotes.txt for important information about which versions of other
software components are required on the target.

===============
Running samples
===============
From your Eclipse Workspace:
1) Create a new C++ standard make project. 
2) Change its C/C++ Make project properties for SCons
   a) Right click on the top level project and select the "Properties" menu
   b) Select the "C/C++ Make Project" item
   c) On the "Make Builder" tab, uncheck "Use default"
   d) In "Build command", enter "scons -k type=emulation" if you are running the
      emulation target.  If you are running on the actual hardware, enter
	  "scons runtests=f deploy_dir=</path/to/your/nfsroot/>".  Your nfsroot is
	  wherever you chose to untar the nfsroot, probably /home/lfu/nfsroot/.
   e) Clear out the "Build" field
   f) Put '-c' in the "Clean" field
   g) Click "OK" to dismiss the dialog
3) Copy the contents of one of the sample projects into your newly created project folder
   (you may need to use the File Browser to do this)
4) In Eclipse, right click on the top level project and select the "Refresh" menu
5) In Eclipse, open the "SConstruct" file.  On line 34, change "cdevkit_dir = '<install_dir>'"
   to the root location where you installed this distribution.  Alternatively,
   you can set the environment variable BRIO_DEVKIT_DIR to point to the root location.
6) From the Eclipse "Project" menu, select "Build Project"
7) If you are running the emulation target, open the newly created
   "Build/LightingGCC_emulation" folder.  You should see "BrioCube" there.
   Right click on "BrioCube", and select "Run As | Run Local C/C++
   Application".  Click through any dialogs you need to.  That's it!
8) If you are running on the actual hardware, you should just be able to run
   the command "BrioCube" in the serial console.

