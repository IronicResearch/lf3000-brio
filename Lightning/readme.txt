======================================================
Lightning SDK
======================================================

Lightning SDKs typically consist of these parts:

	LightningSDK_xxxx.tar.gz -Brio SDK components necessary for development.
	embedded-svnxxxx.tar.gz - embedded binaries to be flashed onto the target board.
	bootstrap-xxxx.lfp		- bootstrap loader binaries in .lfp package.
	firmware-xxxx.lfp		- kernel and rootfs binaries in .lfp package.
	Brio-xxxx.lfp 			- Brio binaries to be copied onto target NAND partition via USB. 
	nfsroot-svnxxxx.tar.gz  - optional nfsroot folder for booting the target board.
                            
The SDK can be unzipped as is and located anywhere.

	$ tar -xzvf LightningSDK_xxxx.tar.gz
	$ export LEAPFROG_PLUGIN_ROOT=/path/to/LightningSDK_<xxxx>

The embedded binary images are distributed in .lfp package files which can flashed
onboard by a variety of methods described in the TargetSetup.txt file.

The NFS root image must be unzipped as root user and located off the home user path.

	$ sudo tar -xzvf nfsroot-svnxxx.tar.gz
	$ mv nfsroot-svnxxx nfsroot
	
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

	lightning-boot.bin	- Lightning boot loader (replaces uniboot)
	kernel.jffs2		- Linux kernel image (replaces zImage and u-boot)
	erootfs.jffs2		- Embedded root filesystem (alternative to nfsroot)

All 3 binaries need to be flashed into new locations described in TargetSetup.txt. 
Beginning with LinuxDist release 0.10.0-2101, the binaries are delivered
in .lfp package files, and no longer contain version numbers in their names.

When transferred over USB connection from a host PC, the firmware-XXXX.lfp
package file will automatically be flashed onboard after USB is disconnected.
This is only applicable on units with at least version 0.10.0-2101 onboard.
Otherwise the embedded binaries will need to be unpacked from the .lfp file
and flashed using one of the manual methods described in TargetSetup.txt.

To unpack the .lfp files for TFTP uploading:

	$ lfpkg -a install -b ~/tftpboot -d . bootstrap-XXXX.lfp
	$ lfpkg -a install -b ~/tftpboot -d . firmware-XXXX.lfp

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

	$ export LEAPFROG_PLUGIN_ROOT=/path/to/LightningSDK_<xxxx>

You may also want to add this to your .bashrc file in the lfu home directory so it will always 
be setup:
	
	$ export LEAPFROG_PLUGIN_ROOT=/home/lfu/LightningSDK_<xxxx>

======================================================
Target Preparation : mounting NAND partition via USB
======================================================

In order to install application binaries and resources onto the embedded target board,
a designated NAND partition is available for mounting as a USB mass storage device on
the host development system.

Internally the NAND partition is mounted at bootup as '/Didj'. You can verify this
by listing all mounted devices and partitions at the target console prompt.

	(target) # mount
	
To expose the NAND partition to the development system:

	(target) # usbctl -d mass_storage -a enable
	(target) # usbctl -d mass_storage -a unlock

Connecting a USB cable to the embedded target board should make the board appear as a
mounted USB device. On Ubuntu Linux, the mounted device name is '/media/Didj'. This
name should be set as the root filesystem path for the Lightning SDK.

	(host) $ export ROOTFS_PATH=/media/Didj   

Once mounted, the Brio binaries will need to copied onto the exposed NAND partition.
This is intended to begin populating the /Didj directory after the board is reflashed.
The Brio binaries are now delivered in .lfp package file, which may be unpacked
on the /Didj NAND partition using the lfpkg tool on the target:

	(target) # lfpkg -a install Brio-xxxx.pkg
	  
The same lfpkg script may also be executed on host side using an explicit -base path:

	(host) $ lfpkg -a install -b /media/Didj Brio-xxxx.pkg 
	
Embedded target application binaries and resources will then be copied to their
respective subdirectory locations on the mounted NAND partition. When copying files
is completed and it is time to disconnect the USB cable, be sure to unmount (eject)
the mounted device on the host side before disabling it from the target.  

To remove the NAND partition from the development system:
	 
	(target) # usbctl -d mass_storage -a lock 
	(target) # usbctl -d mass_storage -a disable

Note that in order to insure filesystem coherency on the /Didj NAND partition,
/Didj is always unmounted internally when enabled over USB, and remounted
automatically when disabled over USB.  

During development it might be convenient to keep the /Didj NAND partition unlocked,
in which case the /Didj volume will appear mounted on the PC host anytime the USB
enable command is issued. This is done by setting a flag file 'usb_mass_storage'.

To leave the /Didj NAND partition unlocked:

	(target) # echo UNLOCKED > /flags/usb_mass_storage
 
	
======================================================
Target Preparation : booting to NFS root filesystem 
======================================================

In situations it might be desirable to revert to booting the root filesystem over NFS
instead of from NAND. Use of debugger and profiling tools which cannot fit into NAND
partition would be examples.

You would need to set a flag file on the embedded target to switch between booting from
NAND root filesystem to NFS mounted root filesystem. 

To examine the current root filesystem setting:

	(target) # cat /flags/rootfs

To set NFS root filesystem booting:

	(target) # echo NFS0 > /flags/rootfs
	
Restarting the embedded target should now complete booting at the NFS mounted location
at Ethernet IP address 192.168.0.113 as previous releases (described below).

In order to see files copied to the '/Didj' directory at the NFS mounted location,
you will need to unmount the NAND partition.

	(target) # umount /Didj
	(target) # ls /Didj

When building SDK apps, be sure ROOTFS_PATH is pointing to the NFS path on the
development system.

	(host) $ export ROOTFS_PATH=/home/lfu/nfsroot

Embedded target application binaries and resources will then be copied to their
respective subdirectory locations on the development system's NFS rootfs directory
during SCons builds.

======================================================
Target Preparation : installing nfsroot image
======================================================

If you are running on actual target hardware, you should probably have received
an updated root file system with this code drop.  It should have been called
something like nfsroot-svnxxx.tar.gz.  To install it, extract it to the
directory that you export via NFS to the target.  This directory is probably
/home/lfu/nfsroot.  Note that you must be root to successfully untar the
rootfs!!  The reason is that it must create device nodes.

	$ sudo tar -xzvf nfsroot-svnxxx.tar.gz
	$ mv nfsroot-svnxxx nfsroot

The environment variable ROOTFS_PATH should be set to point to the nfsroot path
so the SDK build scripts will know where to install application binaries and resources.

	$ export ROOTFS_PATH=/home/lfu/nfsroot

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

