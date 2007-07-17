======================================================
Lightning SDK
======================================================

Lightning SDKs typically consist of three parts:

  LightningSDK_xxxx.tar.gz -this contains the Brio components necesasry for development
  nfsroot-svnxxxx.tar.gz  - this contains the linux nfsroot folder necessary for booting the target
  zImage-svnxxxx.tar.gz   - the linux kernel.  If you aren't booting using tftp, you will need to
                            flash this file in the NAND of the target board.
                            
The SDK can be unzipped as is and located anywhere.

	tar -xzvf LightningSDK_xxxx.tar.gz
	export LEAPFROG_PLUGIN_ROOT=/path/to/LightningSDK_<xxxx>

The NFS root image must be unzipped as root user and located off the home user path.

	sudo tar -xzvf nfsroot-svnxxx.tar.gz
	mv nfsroot-svnxxx nfsroot
	
The Linux kernel zImage will copied to your TFTP server base directory, and subsequently downloaded
onto the target board via the U-Boot loader. (Refer to kernel image
download instructions below.)

	tar -xzvf zImage-svnxxxx.tar.gz
	cp zImage-xxxx ~/tftpboot

These three pieces MUST be used together!  The <xxxx> numbers for a release will generally be
the same for all three pieces.  Either way, make sure that you use only the pieces from a single 
release together or you will almost certainly have problems.

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
|  +-Button       (demonstrates button presses on the hardware)
|  |
|  +-Display      (demonstrates 2D RGB display surface layer and fonts)
|  |
|  +-Simple       (simplest OpenGL application, minimal build system)
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

======================================================
Target Preparation -- setting up NFS
======================================================

Obviouslly, you only need to do these steps if you system hasn't already 
been configured.

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

======================================================
Target Preparation -- setting up tftp
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
server_args = /home/lfu/tftproot
disable     = no
}


3. Make /home/lfu/tftproot directory.

	mkdir -p /home/lfu/tftproot
	chmod -R 777 /home/lfu/tftproot


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

	ls -l /home/lfu/tftproot
	-rw------- 1 lfu lfu 0 2007-06-04 12:10 hda.txt


7. Troubleshooting. Check the /var/log/syslog file for tftp error messages.

	tail -f /var/log/syslog

======================================================
Target Preparation -- downloading kernel image
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
	
Then download the updated kernel zImage from its TFTP location on the
development system. On Ubuntu Linux, TFTP is typically configured
at the ~/tftpboot directory, so this is where the zImage file needs to
be copied to. The zImage file provided in releases will typically have
some version number suffix, like zImage-xxxx.

	tftp 02000000 zImage-xxxx 

After the zImage is downloaded, you have the option to immediately run it
from RAM, or flash into NAND memory and reboot.

To immediately test the downloaded zImage from RAM:

	go 02000000
	
To flash the downloaded zImage into NAND:

	nand erase clean 00080000 100000
	nand write 02000000 00080000 100000

Either method should execute the updated Linux kernel and NFS mount the
root file system located on the development system.

======================================================
Target Preparation -- installing ARM cross-compiler
======================================================

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

======================================================
Running samples  -- Emulation
======================================================
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
	  The environment variable ROOTFS_PATH will be used instead if it is defined:
	  export ROOTFS_PATH=/home/lfu/nfsroot
   e) Clear out the "Build" field
   f) Put '-c' in the "Clean" field
   g) Click "OK" to dismiss the dialog
3) Copy the contents of one of the sample projects into your newly created project folder
   (you may need to use the File Browser to do this)
4) In Eclipse, right click on the top level project and select the "Refresh" menu
5) In Eclipse, open the "SConstruct" file.  On line 34, change "cdevkit_dir = '<install_dir>'"
   to the root location where you installed this distribution. The environment variable
   LEAPFROG_PLUGIN_ROOT will be used instead if it is defined.
6) From the Eclipse "Project" menu, select "Build Project"
7) If you are running the emulation target, open the newly created
   "Build/LightingGCC_emulation" folder.  You should see "BrioCube" there.
   Right click on "BrioCube", and select "Run As | Run Local C/C++
   Application".  Click through any dialogs you need to.  That's it!

======================================================
Running samples  -- Target
======================================================
1) Perform the same steps above except omitting the 'type=emulation' build
   target in step 2d. The executable and any associated resources will be
   automatically copied into the appropriate nfsroot directories.
2) If you are running on the actual hardware, you should just be able to run
   the command "BrioCube" in the serial console.

