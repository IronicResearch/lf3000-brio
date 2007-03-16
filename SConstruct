#=============================================================================
# Top level SConstruct file for the SCons build system
#
# This file lives at the root of the repository, immediately above the "System"
# folder.
#
# Quick tour of SCons:
#
#	SCons is a build tool that replaces "make" functionality.
#	It executes as a Python script, which gives you the full power
#	of the Python scripting language in defining build steps.
#
#	SCons starts with a "SConstruct" file (this file).  This SConstruct
#	file sets up an "Environment" object with common build settings for
#	a given platform (a platform is defined by a combination of hardware
#	product and toolchain, e.g., FlyFusionARC, LightningGCC).  
#	This Environment object is used by "SConscript" files that define how 
#	to build individual modules.
#
#	The bottom section of this file sets up targets by invoking these 
#	SConscript files.
#
#	NOTES: Under Linux, you will need to use the Synaptic Package Manager
#	to install the "subversion" package.
#=============================================================================
import os
import glob
import SCons.Defaults
import Etc.Tools.SConsTools.lfutils	# LeapFrog utility functions



#-----------------------------------------------------------------------------
# Setup help options and get command line arguments
#-----------------------------------------------------------------------------
opts = Options('custom.py')
opts.Add(BoolOption('checkheaders', 'Set "checkheaders=t" to uncover order of inclusion issues', 0))
opts.Add(BoolOption('emulation', 'Set "emulation=t" to build the target using emulation', 0))
opts.Add(EnumOption('platform', 'Set platform to use', 'LightningGCC', 
						allowed_values=('FlyFusionARC', 
										'LightningGCC')))
opts.Add(BoolOption('publish', 'Set "publish=t" to create a pulic release', 0))
opts.Add(BoolOption('runtests', 'Default is to run unit tests, set "runtests=f" to disable', 1))
opts.Add('setup', 'Set to "TRUNK" or branch name to setup source tree for a platform', '')

is_checkheaders	= ARGUMENTS.get('checkheaders', 0)
is_emulation	= ARGUMENTS.get('emulation', 0)
is_publish		= ARGUMENTS.get('publish', 0)
is_runtests		= ARGUMENTS.get('runtests', 1)
platform		= ARGUMENTS.get('platform', 'LightningGCC')
source_setup	= ARGUMENTS.get('setup', '')

if is_checkheaders:
	is_emulation = 1


#-----------------------------------------------------------------------------
# Setup intermediate build and deployment target folder names
#-----------------------------------------------------------------------------
root_dir				= Dir('#').abspath
intermediate_build_dir	= os.path.join('Temp', platform)
if is_emulation:
	intermediate_build_dir += '_emulation' + (is_publish and '_publish' or '')
adjust_to_source_dir	= '../../../'
debug_deploy_dir		= os.path.join('#Build', 'Output' + (is_emulation and '_emulation' or ''), platform)
release_deploy_dir		= os.path.join('#Build', 'Output' + (is_emulation and '_emulation_publish' or ''), platform)
checkheaders_deploy_dir	= os.path.join(intermediate_build_dir, 'checkheaders')
dynamic_deploy_dir		= is_publish and release_deploy_dir or debug_deploy_dir


#-----------------------------------------------------------------------------
# Set up and Python dictionary variables that map generic names
# to specific paths or library names.  This abstraction allows us to make
# changes only here, not in all the SConscript files.  It also allows us to
# support compiling with different compilers that need to link against
# differently named libraries.
#-----------------------------------------------------------------------------
local_vars		 = { 'adjust_to_source_dir'		: adjust_to_source_dir,
					 'intermediate_build_dir'	: intermediate_build_dir,
					 'is_publish'				: is_publish,
					 'platform'					: platform,
				   }


#-----------------------------------------------------------------------------
# Setup the environment object
#-----------------------------------------------------------------------------
platform_toolset = platform + (is_emulation and '_emulation' or '_embedded')
toolpath = Dir(os.path.join(root_dir, 'Etc', 'Tools', 'SConsTools')).abspath
env = Environment(	options  = opts,
					tools = ['default', platform_toolset, 
							'checkheader', 'cxxtest', 'runtest'], 
					toolpath = [toolpath],
					LIBPATH = [dynamic_deploy_dir]
				 )


#-----------------------------------------------------------------------------
# Allow per-user overrides of global settings
#-----------------------------------------------------------------------------
override = __import__('UserOverrides', globals(), locals(), [''])
if override.__dict__.has_key(platform_toolset):
	override.__dict__[platform_toolset](env)

#print env.Dump()


#-----------------------------------------------------------------------------
# Attach the help to the Environment object
#-----------------------------------------------------------------------------
Help(opts.GenerateHelpText(env))


#-----------------------------------------------------------------------------
# Dynamically find sources for a module
#
# The "map(lambda ...)" stuff is required because of how SCons deals with
# separating source folders from build folders.  When you specify a separation
# (see "build_target" in the "SConscript()" command), SCons copies all the
# source files into the target build folder.  The "sources" file list below
# needs to point at the COPIED-TO files in the build folder, but it needs to
# search (glob.glob in the actual SOURCE folder).  Unintuitive
#-----------------------------------------------------------------------------
def FindModuleSources(pdir):
	source_dir = Etc.Tools.SConsTools.lfutils.SourceDirFromBuildDir(pdir.abspath, root_dir)
	c_pattern = os.path.normpath(os.path.join(source_dir, '*.c'))
	cpp_pattern = os.path.normpath(os.path.join(source_dir, '*.cpp'))
	sources =  map(lambda x: os.path.join(pdir.abspath, os.path.split(x)[1]), 
					glob.glob(c_pattern))
	sources += map(lambda x: os.path.join(pdir.abspath, os.path.split(x)[1]), 
					glob.glob(cpp_pattern))
	return sources


#-----------------------------------------------------------------------------
# Build a Module (either embedded or emulation target)
#
# Prepend any platform-specific libraries to the library list
# Install the library to the correct installation folder
#-----------------------------------------------------------------------------
def MakeMyModule(penv, ptarget, psources, plibs):
	if len(psources) != 0:
		bldenv = penv.Copy()
		source_dir = Etc.Tools.SConsTools.lfutils.SourceDirFromBuildDir(os.path.dirname(psources[0]), root_dir)
		linklibs = plibs
		# TODO/tp: put all map files in a single folder, or keep hierarchy?
		mapfile = File(os.path.join(intermediate_build_dir, adjust_to_source_dir, ptarget + '.map')).abspath
		bldenv.Append(CPPPATH  = [os.path.join(source_dir, 'Include')])
		bldenv.Append(LINKFLAGS = ' -Wl,-Map=' + mapfile)
		mylib = bldenv.SharedLibrary(ptarget, psources, LIBS = linklibs)	
		bldenv.Install(dynamic_deploy_dir, mylib)
		return mylib


#-----------------------------------------------------------------------------
# Function for generating and running unit tests
#
# Note: "ptarget" is library name, used to build test<name>.exe and link the 
#		library into it.
#		"psources" is a list of additional sources to build into the EXE
#		(normally empty--needed a way to test CmdLineUtils.cpp)
#		"plibs" are the libraries the tested library was linked against
#-----------------------------------------------------------------------------
platformlibs = ['glibmm-2.4', 'glib-2.0']
def RunMyTests(ptarget, psources, plibs, penv):
	if not is_checkheaders:
		testenv = penv.Copy()
		testenv.Append(CPPPATH  = ['#ThirdParty/cxxtest', root_dir])
		testenv.Append(CPPDEFINES = 'UNIT_TESTING')
		testenv.Append(LINKFLAGS = ' -Wl,-rpath=' + Dir(dynamic_deploy_dir).abspath)
		srcdir = Etc.Tools.SConsTools.lfutils.SourceDirFromBuildDir(os.path.dirname(ptarget), root_dir)
		tests = glob.glob(os.path.join(srcdir, 'tests', '*.h'))
		unit = 'test_' + ptarget + '.cpp'
		mytest = testenv.CxxTest(unit, tests)
		if os.path.exists(unit):
			temp = testenv.Program([mytest] + psources, 
						LIBS = plibs + [ptarget] + platformlibs)
			mytestexe = testenv.Install(dynamic_deploy_dir, temp)
			if is_runtests == 1:
				testenv.RunTest(str(mytestexe[0]) + '_passed', mytestexe)


#-----------------------------------------------------------------------------
# Export environment variables to the SConscript files
#-----------------------------------------------------------------------------
Export('env local_vars FindModuleSources MakeMyModule RunMyTests')


#-----------------------------------------------------------------------------
# Setup build targets for modules
#
# Use "duplicate=0" to prevent SCons from copying the source files.
# If you don't do this, the problems pane in Eclipse opens up the copied file,
# not the true source, so you end up fixing problems in a file that will get
# overwritten on the next build.
#-----------------------------------------------------------------------------
SConscript(os.path.join(root_dir, 'System', 'SConscript'), duplicate=0)
#SConscript(os.path.join(root_dir, platform, 'SConscript'), duplicate=0)


#-----------------------------------------------------------------------------
# Handle normal and special targets (default build all)
#-----------------------------------------------------------------------------
Default('..')
if is_checkheaders:
	Etc.Tools.SConsTools.lfutils.CheckHeaders(root_dir, checkheaders_deploy_dir, env)
	Default(None)
	Default(checkheaders_deploy_dir)
elif source_setup:
	Etc.Tools.SConsTools.lfutils.CreateSourceTree(root_dir, platform, source_setup, env)
