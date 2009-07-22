#-----------------------------------------------------------------------------
# LfUtils.py
#
# Common utilities for use by top level SConstruct files
#-----------------------------------------------------------------------------
import os
import glob
import SCons.Util
#import pysvn
import imp

root_dir = os.path.normpath(os.path.join(__file__, '../../../../..'))

#-----------------------------------------------------------------------------
# Public constants
#-----------------------------------------------------------------------------
kBuildModule	= 0
kBuildMPI		= 1
kBuildPrivMPI	= 2


#-----------------------------------------------------------------------------
# Enumerate folders that should be ignored when doing a test compile of headers
# TODO/tp: Provide user override of this list?
#-----------------------------------------------------------------------------
exclude_dirs_for_header_compile = [
	'.svn',
	'Temp',
	'Build',
	'XBuild',
	'Publish', 
	'tests',
	'disabled',
	'ThirdParty',
	'.metadata',
	'.settings']


#-----------------------------------------------------------------------------
# Compile headers alone to expose order-of-inclusion issues.
#-----------------------------------------------------------------------------
def CheckHeaders(root, deploy_dir, env):

	def CheckInFolder(args, dirname, filenames):
		# Look for known folders that should be excluded from this search.
		# If we find one, make sure none of its subfolders gets searched.
		base = os.path.basename(dirname)
		if base in exclude_dirs_for_header_compile:
			del filenames[:]
		else:
			rootlen = args[0]
			deploy_dir = args[1]
			env = args[2]
			for x in filenames:
				if os.path.splitext(x)[1] == '.h' and os.path.normcase(x) != 'stdafx.h':
					relpath = dirname[rootlen:]
					path = os.path.join(os.getcwd(), dirname, x)
					dest = os.path.join(deploy_dir, relpath, x + '.test.o')
					env.CheckHeader(target = dest, source = path)
	
	os.path.walk(root, CheckInFolder, [len(root)+1, deploy_dir, env])


#-----------------------------------------------------------------------------
# Create source tree for a platform
#-----------------------------------------------------------------------------
def CreateSourceTree(root, platform, branch, env):
	common = __import__(os.path.join('Etc/Tools/SConsTools', platform + '_common'), globals(), locals(), [''])
	mappings = common.GetRepositoryMappings()
	for repo, reldir in mappings.iteritems():
		command = 'svn co ' + os.path.join(repo, branch) + ' ' + os.path.join(root, reldir)
		env.Execute(command)


#-----------------------------------------------------------------------------
# Create source tree for a platform
#-----------------------------------------------------------------------------
def SourceDirFromBuildDir(target, base):
    """
    Return a relative path to the target from either the current dir or an optional base dir.
    Base can be a directory specified either as absolute or relative to current dir.
    """

    base_list = (os.path.abspath(base)).split(os.sep)
    target_list = (os.path.abspath(target)).split(os.sep)

    # On the windows platform the target may be on a completely different drive from the base.
    if os.name in ['nt','dos','os2'] and base_list[0] <> target_list[0]:
        raise OSError, 'Target is on a different drive to base. Target: '+target_list[0].upper()+', base: '+base_list[0].upper()

    # Starting from the filepath root, work out how much of the filepath is
    # shared by base and target.
    for i in range(min(len(base_list), len(target_list))):
        if base_list[i] <> target_list[i]: break
    else:
        # If we broke out of the loop, i is pointing to the first differing path elements.
        # If we didn't break out of the loop, i is pointing to identical path elements.
        # Increment i so that in all cases it points to the first differing path elements.
        i+=1

	# Make sure we only begin with the OS separator if we are non-Windows
	if os.sep == '\\':
		rel_list = [base_list[0]] + [os.sep]
		base_list = base_list[1:]
	else:
 		rel_list = [os.sep]
    rel_list += base_list + target_list[i:i+1] + target_list[i+3:]
    return os.path.join(*rel_list)


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
# Dynamically find sources for a module
#
# The "map(lambda ...)" stuff is required because of how SCons deals with
# separating source folders from build folders.  When you specify a separation
# (see "build_target" in the "SConscript()" command), SCons copies all the
# source files into the target build folder.  The "sources" file list below
# needs to point at the COPIED-TO files in the build folder, but it needs to
# search (glob.glob in the actual SOURCE folder).  Unintuitive
#-----------------------------------------------------------------------------
def FindModuleSources(pdir, vars):
	source_dir = os.path.normpath(SourceDirFromBuildDir(pdir.abspath, root_dir))
	sources =  map(lambda x: os.path.join(pdir.abspath, os.path.split(x)[1]), 
					glob.glob(os.path.join(source_dir, '*.c')))
	sources += map(lambda x: os.path.join(pdir.abspath, os.path.split(x)[1]), 
					glob.glob(os.path.join(source_dir, '*.cpp')))
	temp = (vars['is_emulation'] and 'Emulation' or vars['platform']) 
	platform_dir = os.path.join(source_dir, temp)
	sources += map(lambda x: os.path.join(pdir.abspath, temp, os.path.split(x)[1]), 
					glob.glob(os.path.join(platform_dir, '*.c')))
	sources += map(lambda x: os.path.join(pdir.abspath, temp, os.path.split(x)[1]), 
					glob.glob(os.path.join(platform_dir, '*.cpp')))
	return sources


#-----------------------------------------------------------------------------
# Dynamically find sources for an MPI interface module
#
# See notes on "map(lambda ...)" stuff above.
#-----------------------------------------------------------------------------
def FindMPISources(pdir, vars):
	mpidir = os.path.join(pdir.abspath, 'PublicMPI')
	source_dir = SourceDirFromBuildDir(mpidir, root_dir)
	cpp_pattern = os.path.normpath(os.path.join(source_dir, '*.cpp'))
	sources =  map(lambda x: os.path.join(mpidir, os.path.split(x)[1]), 
					glob.glob(cpp_pattern))
	return sources


#-----------------------------------------------------------------------------
# Build a Module (either embedded or emulation target)
#
# Prepend any platform-specific libraries to the library list
# Install the library to the correct installation folder
# 'ptype' is 'kBuildModule' for modules and 'kBuildMPI' for mpi libraries
#-----------------------------------------------------------------------------
def MakeMyModule(penv, ptarget, psources, plibs, ptype, vars):
	if len(psources) != 0:
		bldenv = penv.Copy()
		source_dir = SourceDirFromBuildDir(os.path.dirname(psources[0]), root_dir)
		linklibs = plibs
		# TODO/tp: put all map files in a single folder, or keep hierarchy?
		root = source_dir[len(root_dir)+1:]
		while root.count('/') > 0:
			root = os.path.dirname(root)
		mapfile = os.path.normpath(os.path.join(root, vars['intermediate_build_dir'], 'lib' + ptarget + '.map'))
		priv_incs = (ptype == kBuildModule and 'Include' or os.path.join('..', 'Include'))
		bldenv.Append(CPPPATH  = [os.path.join(source_dir, priv_incs)])
		bldenv.Append(LINKFLAGS = ' -Wl,-Map=' + mapfile)
		if vars['is_debug']:
			bldenv.Append(CCFLAGS = '-g -O0')
			bldenv.Append(CPPDEFINES = 'DEBUG')
#		else:
#			bldenv.Append(LINKFLAGS = '-Wl,--strip-all')
		if vars['is_monolithic']:
			mylib = bldenv.StaticLibrary(ptarget, psources, LIBS = linklibs)
		else:
			mylib = bldenv.SharedLibrary(ptarget, psources, LIBS = linklibs)
		if ptype == kBuildModule:
			bldenv.Install(vars['mod_deploy_dir'], mylib)
		elif ptype == kBuildPrivMPI:
			bldenv.Install(vars['priv_mpi_deploy_dir'], mylib)
		elif vars['is_emulation']:
			bldenv.Install(vars['mpi_deploy_dir'], mylib)
		else: # mpi lib for embedded build
			bldenv.Install(vars['lib_deploy_dir'], mylib)
			bldenv.Install(vars['mpi_deploy_dir'], mylib)
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
def RunMyTests(ptarget, psources, plibs, penv, vars):

	if vars['is_checkheaders'] or vars['is_export'] or not vars['is_runtests']:
		return

	srcdir = SourceDirFromBuildDir(os.path.dirname(ptarget), root_dir)
	tests = glob.glob(os.path.join(srcdir, 'tests', '*.h'))
	subdir = (vars['is_emulation'] and 'Emulation' or vars['platform']) 
	tests += glob.glob(os.path.join(srcdir, 'tests', subdir, '*.h'))
	if len(tests) == 0:
		return
		
	testenv = penv.Copy()
	if vars['is_debug']:
		testenv.Append(CCFLAGS = '-g')
	testenv.Append(CPPPATH  = ['#ThirdParty/cxxtest', root_dir])
	testenv.Append(CPPDEFINES = 'UNIT_TESTING')
	testenv.Append(RPATH = [vars['mpi_deploy_dir'], vars['priv_mpi_deploy_dir']])
	unit = 'test_' + ptarget + '.cpp'
	mytest = testenv.CxxTest(unit, tests)

	platformlibs = ['DebugMPI']
	if vars['is_emulation']:
		platformlibs += ['glibmm-2.4', 'glib-2.0']
	else:
		platformlibs += ['dl', 'ustring', 'iconv', 'intl', 'sigc-2.0', 'pthread', 'rt']
	fulllibs = plibs + [ptarget + 'MPI']
	if vars['is_emulation']:
		fulllibs += ['Emulation']
		testenv.Append(LIBPATH = ['#ThirdParty/PowerVR/Libs'])
		testenv.Append(RPATH = [os.path.join(root_dir, 'ThirdParty', 'PowerVR', 'Libs')])
	if vars['is_monolithic']:
		hwogllibs = vars['variant'] == 'LF1000' and ['ogl_lf1000'] or ['ogl']
		fulllibs += vars['is_emulation'] and ['gles_cl'] or hwogllibs
		libpaths = [vars['mod_deploy_dir'], vars['lib_deploy_dir'], 
					os.path.join(root_dir, 'ThirdParty/ustring/libs', vars['cpu_subdir']),
					os.path.join(root_dir, 'ThirdParty/MagicEyes/Libs', vars['cpu_subdir']),
					os.path.join(root_dir, 'ThirdParty/FreeType/Libs', vars['cpu_subdir']),
					os.path.join(root_dir, 'ThirdParty/LibSndFile/Libs', vars['cpu_subdir']),
					os.path.join(root_dir, 'ThirdParty/Theora/Libs', vars['cpu_subdir']),
					os.path.join(root_dir, 'ThirdParty/Portaudio/Libs', vars['cpu_subdir']),
					os.path.join(root_dir, 'ThirdParty/Mobileer/Libs', vars['cpu_subdir'])]
		testenv.Append(LIBPATH = libpaths)
		testenv.Append(RPATH = vars['lib_deploy_dir'])
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
		if vars['is_emulation']:
			fulllibs +=  ['X11']
	temp = testenv.Program([mytest] + psources, LIBS = fulllibs + platformlibs)
	mytestexe = testenv.Install(vars['bin_deploy_dir'], temp)
	if vars['is_runtests'] and vars['is_emulation']:
		testenv.RunTest(str(mytestexe[0]) + '_passed', mytestexe)


#-----------------------------------------------------------------------------
# Export the minimum set of symbols
#-----------------------------------------------------------------------------
__all__ = ["FindModuleSources", "FindMPISources", "MakeMyModule", "RunMyTests", 
			"kBuildModule", "kBuildMPI", "kBuildPrivMPI"]

