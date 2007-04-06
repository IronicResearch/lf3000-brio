=========================
Lightning Emulation 0.1.1
=========================

============
Installation
============

Unarchive this file to create a distribution folder.
Use any target folder to which you have user access on your system.

The installation tree will look like this:

+-Include  (Brio MPI headers)
|  |
|  +-boost (preprocessor library, auto-generates text descriptions of errors at compile time)
|  |
|  +-OpenGL
|     |
|     +-LightingGCC           (subfolders for MagicEyes headers--not in release 0.1.1)
|     |
|     +-LightingGCC_emulation (subfolders for PowerVR headers)
|
+-Libs  (Brio MPI headers)
|  |
|  +-LightingGCC  (not in release 0.1.1)
|  |  |
|  |  +-Module    (replacable modules, not linked to by apps)
|  |  |
|  |  +-MPI       (module interface objects, directly linked to by apps)
|  |  |
|  |  +-OpenGL    (MagicEyes library)
|  |
|  +-LightingGCC_emulation
|     |
|     +-Module    (replacable modules, not linked to by apps)
|     |
|     +-MPI       (module interface objects, directly linked to by apps)
|     |
|     +-OpenGL    (PowerVR library)
|  
+-Samples
|  |
|  +-readme.txt   (this file)
|  |
|  +-BrioCube     (rotating cube with Brio calls that are exercised through button presses)
|  |
|  +-Simple       (simplest OpenGL application, minimal build system)
|
+-Tools
   |
   +-BuildRsrc    (initial packer for resources)
   |
   +-*.py         (SCons modules for C++ build system)


===============
Running samples
===============
From your Eclipse Workspace:
1) Create a new C++ standard make project. 
2) Change its C/C++ Make project properties for SCons
   a) Right click on the top level project and select the "Properties" menu
   b) Select the "C/C++ Make Project" item
   c) On the "Make Builder" tab, uncheck "Use default"
   d) In "Build command", enter "scons -k emulation=t"
   e) Clear out the "Build" field
   f) Put '-c' in the "Clean" field
   g) Click "OK" to dismiss the dialog
3) Copy the contents of one of the sample projects into your newly created project folder
   (you may need to use the File Browser to do this)
4) In Eclipse, right click on the top level project and select the "Refresh" menu
5) In Eclipse, open the "SConstruct" file.  On line 34, change "cdevkit_dir = '<install_dir>'"
   to the root location where you installed this distribution.
6) From the Eclipse "Project" menu, select "Build Project"
7) Open the newly created "Build/LightingGCC_emulation" folder.  You should see "test2" there.
8) Right click on "test2", and select "Run As | Run Local C/C++ Application".  
   Click through any dialogs you need to.  That's it!



