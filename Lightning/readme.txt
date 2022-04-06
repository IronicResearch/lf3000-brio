======================================================
Lightning SDK
======================================================

The Emerald SDK is based on Lightning platform SDK for C++ development.
The following noteable differences between Emerald and Didj devices are:

-- The Emerald device connects USB as Ethernet device, not mass storage.
Telnet and FTP network sessions are used to communicate with the device. 
-- There is new internal filesytem and volume structure which is not
immediately compatible with Didj.
-- The NFS root image is not used for booting the Emerald device.
NFS mounting directories is still useful for development purposes.
  
Lightning SDKs typically consist of these parts:

	LightningSDK_xxxx.tar.gz -Brio SDK components necessary for development.
	nfsroot-xxxx.tar.gz  	- optional nfsroot image folder for development.
                            
The SDK can be unzipped as is and located anywhere.

	$ tar -xzvf LightningSDK_xxxx.tar.gz
	$ export LEAPFROG_PLUGIN_ROOT=/path/to/LightningSDK_<xxxx>

The embedded binary images are distributed in .lfp package files which can flashed
onboard by a variety of methods described in the TargetSetup.txt file.

The NFS root image must be unzipped as root user and located off the home user path.

	$ sudo tar -xzvf nfsroot-xxxx.tar.gz
	$ mv nfsroot-xxxx nfsroot
	
These pieces MUST be used together!  The <xxxx> numbers for a release will generally be
the same for all three pieces.  Either way, make sure that you use only the pieces from a single 
release together or you will almost certainly have problems.

======================================================
Installation
======================================================

NOTE: MIDI is no longer supported by Brio Audio MPI.

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
|  +-DatasetLoader	(tests XML binary dataset loader utility)
|  |
|  +-Display      (demonstrates 2D RGB display surface layer and fonts)
|  |
|  +-PageFlipDemo (demonstrates page flipping with triple or double buffering)
|  |
|  +-PlayAudio    (command line utility to test OGG files)
|  |
|  +-Power        (tests Power MPI events)
|  |
|  +-Simple       (simplest OpenGL application, minimal build system)
|  |
|  +-SysAppDemo   (system application demo for handling USB and Power events)
|  |
|  +-USBDemo      (tests USB MPI events)
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
Target Preparation : networking to the device
======================================================

The Emerald device supports networking connectivity via Ethernet over USB.
The device uses pre-assigned IP address at 192.168.0.111.
The PC host should assign an IP address at 192.168.0.113.

	$ sudo ifconfig eth1 192.168.0.113
	
When properly configured you should be able to ping the device.

	$ ping 192.168.0.111	
	
Telnet and FTP daemons are running on the device.
To open a Telnet console session on the device:

	$ telnet 192.168.0.111
	
Once your Telnet session is active on the device, you should be able to
execute commands from the device shell like:

	# ls
	# mount
	# ping 192.168.0.113

During a console session you can also copy files to the device via FTP:

	# ftpget -u lfu -p lfuser 192.168.0.113 Brio.lfp /home/lfu/payload/Brio-xxxx.lfp

The FTP server must be properly configured on your PC host, so subsequent
FTP commands can refer to the logged in user ('lfu') and payload directory
on the host (/home/lfu/payload). Note this 'payload' directory is commonly
used in other device utility scripts. 

======================================================
Target Preparation : installing nfsroot image
======================================================

If you are running on actual target hardware, you should probably have received
an updated root file system with this code drop.  It should have been called
something like nfsroot-xxxx.tar.gz.  To install it, extract it to the
directory that you export via NFS to the target.  This directory is probably
/home/lfu/nfsroot.  Note that you must be root to successfully untar the
rootfs!!  The reason is that it must create device nodes.

	$ sudo tar -xzvf nfsroot-xxxx.tar.gz
	$ mv nfsroot-xxxx nfsroot

The environment variable ROOTFS_PATH should be set to point to the nfsroot path
so the SDK build scripts will know where to install application binaries and resources.

	$ export ROOTFS_PATH=/home/lfu/nfsroot

Note the nfsroot image is no longer used for booting the device. However
it does contain an image of the device which may be useful for development
purposes.

======================================================
Target Preparation : NFS mounting 
======================================================

For development it might be convenient to NFS mount directories on your
Linux PC host. Such NFS mounted directories would supercede those already
embedded on the device for the duration of the NFS mount session. 

To NFS mount the /LF root directory:

	# mount -t nfs -o nolock 192.168.0.113:/home/lfu/nfsroot/LF /LF
	
To remove this NFS mount:

	# umount /LF
	
To view the currently mounted volumes on the device:

	# mount
	
The NFS shares can also be useful for remote debugging binaries which can remain
on the host PC along with their source. For remote debugging SDK example:

	# gdbserver 192.168.0.113:10000 /LF/Bulk/ProgramFiles/BrioCube/BrioCube

With /LF mounted as NFS share, the host PC would effectively be debugging at its
local path: '/home/lfu/nfsroot/LF/Bulk/ProgramFiles/...'

Reference Wiki for Remote Debugging with Eclipse:

http://emma.leapfrog.com/display/BRIO/Remote+debugging+with+gdbserver+and+Eclipse 

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

4) PlayAudio is a command line utility that accepts file names.  Run the program with 
no arguments to see complete usage.

5) BrioAudio needs some button input to play sounds.  The A and B buttons play audio
sample files. 

6) DisplayDemo display a pixel pattern. Buttons change brightness, contrast, alpha.
Pause or Home button exits. 
