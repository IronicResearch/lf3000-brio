#-----------------------------------------------------------------------------
# LfUtils.py
#
# Common utilities for use by top level SConstruct files
#-----------------------------------------------------------------------------
import os
import SCons.Util
from SCons.Node.FS import FS
from SCons.Script import Glob
#import pysvn
import imp

root_dir = os.path.normpath(os.path.join(__file__, '../../../../..'))

#-----------------------------------------------------------------------------
# Public constants
#-----------------------------------------------------------------------------
kBuildModule	= 0
kBuildMPI		= 1

#-----------------------------------------------------------------------------
# Get the repository version from a SVN repository
#-----------------------------------------------------------------------------
def GetRepositoryVersion(platform, branch):
	common = imp.load_source(platform + '_common', os.path.join('Etc/Tools/SConsTools', platform + '_common.py'))
	mappings = common.GetRepositoryMappings()
	path = ''
	revision = 'XXXX'
	for repo, reldir in mappings.iteritems():
		command = 'svn info ' + os.path.join(repo, branch)
		info = os.popen(command)
		rev = info.read()
		begin = rev.find('Revision:')
		end = rev.find('\n', begin)
		rev = rev[begin+10:end]
		revision = rev
			
	print '*** Using repository revision "' + revision + '" as build number ***'
	return revision


#-----------------------------------------------------------------------------
# Build a Module (either embedded or emulation target)
#
# Prepend any platform-specific libraries to the library list
# Install the library to the correct installation folder
# 'ptype' is 'kBuildModule' for modules and 'kBuildMPI' for mpi libraries
#-----------------------------------------------------------------------------
def MakeMyModule(penv, ptarget, psources, plibs, ptype):
	targets = []
	#Build the module
	if len(psources) != 0 and penv['build_dir']:
		bldenv = penv.Clone()
		
		#Setup mapfile name, it will go into the Temp/"Platform" directory
		mapfile = bldenv['intermediate_dir'].File('lib' + ptarget + '.map')
		bldenv.Append(LINKFLAGS = ' -Wl,-Map=' + mapfile.abspath)
		
		if bldenv['debug']:
			bldenv.Append(CCFLAGS = '-g -O0')
			bldenv.Append(CPPDEFINES = 'DEBUG')

		if bldenv['monolithic']:
			mylib = bldenv.StaticLibrary(ptarget, psources, LIBS = plibs)
		else:
			mylib = bldenv.SharedLibrary(ptarget, psources, LIBS = plibs)
		
		#The below line isn't strictly neccessary since nothing depends on a map file
		#But in the off chance that something does in the future...
		bldenv.SideEffect(mapfile, mylib)
	
		if ptype == kBuildModule:
			targets += bldenv.Install(bldenv['build_dir'].Dir('Module'), mylib)
		else:
			#kBuildMPI
			targets += bldenv.Install(bldenv['build_dir'].Dir('MPI'), mylib)
	
	#Install the module
	#Note that this is actually cheating, we aren't installing from the Build directory,
	#But rather from the Temp directory
	if penv['install_dir']:
		mylib = "lib" + ptarget + (penv['monolithic'] and ".a" or ".so")
		if ptype == kBuildModule:
				#SDK shouldn't need modules
				if not penv['is_sdk']:
					targets += penv.Install(penv['install_dir'].Dir('Module'), mylib)
		else:
			#kBuildMPI
			if penv['install_dir']:
				if penv['is_sdk']:
					targets += penv.Install(penv['install_dir'].Dir('LF'), mylib)
				else:
					targets += penv.Install(penv['install_dir'].Dir('lib'), mylib)
	return targets


#-----------------------------------------------------------------------------
# Function for generating and running unit tests
#
# Note: "ptarget" is library name, used to build test<name>.exe and link the 
#		library into it.
#		"psources" is a list of additional sources to build into the EXE
#		(normally empty--needed a way to test CmdLineUtils.cpp)
#		"plibs" are the libraries the tested library was linked against
#-----------------------------------------------------------------------------
def RunMyTests(ptarget, psources, plibs, penv):

	#Only build in embedded/emulation env if tests are desired
	if not penv['buildtests'] or not penv['build_dir']:
		return

	#Gather up test source files
	test_files = Glob(os.path.join('tests', '*.h'))
	subdir = ( (penv['cpu'] == 'x86') and 'Emulation' or penv['platform']) 
	test_files += Glob(os.path.join('tests', subdir, '*.h'))
	if len(test_files) == 0:
		print "No tests found for "+ptarget
		return
	
	#Setup to use cxxtest
	testenv = penv.Clone()
	if penv['debug']:
		testenv.Append(CCFLAGS = '-g')
	testenv.Append(CPPPATH  = [os.path.join('#ThirdParty','cxxtest'), '#'])
	testenv.Append(CPPDEFINES = 'UNIT_TESTING')
	#Set rpath-link to let Brio libraries resolve their dependant libraries
	testenv.Append(RPATH = [penv['build_dir'].Dir('MPI').abspath, penv['install_dir'].Dir('lib').abspath])
	
	# Sort source files. This puts source files with the same name together so that e.g.
	# TestKernel.cpp and Lightning/TestKernel.cpp are compiled together into
	# test_Kernel
	tests = {}
	for t in test_files:
		test_name = os.path.basename( os.path.splitext(t.path)[0] )[len('Test'):]
		if test_name in tests:
			tests[test_name].append(t)
		else:
			tests[test_name] = [t]
	
	mytests = []
	for name, files in tests.iteritems():
		unit = os.path.join('tests', 'test_' + name + '.cpp')
		mytests += testenv.CxxTest(unit, files)
	
	platformlibs = ['DebugMPI']
	if penv['cpu'] == 'x86':
		platformlibs += ['glibmm-2.4', 'glib-2.0', 'pthread']
	else:
		platformlibs += ['dl', 'ustring', 'iconv', 'intl', 'sigc-2.0', 'pthread', 'rt']
		testenv.Append(LIBPATH = ['#ThirdParty/ustring/libs/' + penv['cpu']])
		testenv.Append(RPATH = [ os.path.join( penv['staging_dir'].abspath, 'usr', 'lib' ) ])

		if penv['platform'] == 'LF2000':
			# FIXME: SCons 1.2.0 bug: leading hash not parsed
			testenv.Append(RPATH = ['ThirdParty/Nexell/Libs/' + penv['cpu']])

	# MPI library test?
	if os.path.exists(os.path.join(penv['build_dir'].Dir('MPI').abspath, 'lib' + ptarget + 'MPI.so')):
		fulllibs = plibs + [ptarget + 'MPI']
	else:
		fulllibs = plibs + [ptarget]
	if penv['cpu'] == 'x86':
		fulllibs += ['Emulation']
		testenv.Append(LIBPATH = ['#ThirdParty/PowerVR/Libs'])
		testenv.Append(RPATH = [os.path.join('#ThirdParty', 'PowerVR', 'Libs')])
	if penv['monolithic']:
		hwogllibs = (penv['platform'] == 'Lightning') and ['ogl_lf1000'] or ['ogl']
		fulllibs += (penv['cpu'] == 'x86') and ['gles_cl'] or hwogllibs
		libpaths = [penv['install_dir'].Dir('Module'), penv['install_dir'].Dir('lib'), 
					os.path.join('#ThirdParty/ustring/libs', penv['cpu']),
					os.path.join('#ThirdParty/MagicEyes/Libs', penv['cpu']),
					os.path.join('#ThirdParty/FreeType/Libs', penv['cpu']),
					os.path.join('#ThirdParty/LibSndFile/Libs', penv['cpu']),
					os.path.join('#ThirdParty/Theora/Libs', penv['cpu']),
					os.path.join('#ThirdParty/Portaudio/Libs', penv['cpu']),
					os.path.join('#ThirdParty/Mobileer/Libs', penv['cpu'])]
		testenv.Append(LIBPATH = libpaths)
		testenv.Append(RPATH = penv['install_dir'].Dir('lib'))
		fulllibs += testenv.Split('''ModuleMPI 
									AudioMPI ButtonMPI CartridgeMPI DebugMPI DisplayMPI EventMPI 
									FontMPI KernelMPI PowerMPI USBDeviceMPI VideoMPI 
									Audio Button Debug Display Event Font 
									Kernel Module Power USBDevice Video 
									AudioMPI ButtonMPI CartridgeMPI DebugMPI DisplayMPI EventMPI 
									FontMPI KernelMPI PowerMPI USBDeviceMPI VideoMPI 
									portaudio me2000 vorbisidec dsputil sndfile
									ogg theora freetype
									''')
		if penv['cpu'] == 'x86':
			fulllibs +=  ['X11']

	mytestexe = []
	for test in mytests:
		temp = testenv.Program([test] + psources, LIBS = fulllibs + platformlibs)
		mytestexe += testenv.Install(penv['install_dir'].Dir('bin'), temp)
	targets = mytestexe
	if penv['runtests']:
		#Build test results list which will serve as our RunTest targets
		results = []
		for test in mytestexe:
			test_name = os.path.basename( str(test) )
			results.append( os.path.join( penv['build_dir'], test_name ) )
		
		test_result = testenv.RunTest(results, mytestexe)
		testenv.AlwaysBuild(test_result)
		targets += test_result
	
	return targets


#-----------------------------------------------------------------------------
# Export the minimum set of symbols
#-----------------------------------------------------------------------------
__all__ = ["MakeMyModule", "RunMyTests", "kBuildModule", "kBuildMPI"]

