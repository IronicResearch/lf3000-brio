#-----------------------------------------------------------------------------
# LightningUtils.py
#
# Common utilities for use by top level SConstruct files
#-----------------------------------------------------------------------------
import os
import sys
import SCons
import SCons.Options
import SCons.Script
import SCons.Util
	

#-----------------------------------------------------------------------------
# Setup common command line options for a Lightning project
#-----------------------------------------------------------------------------
def SetupOptions():
	opts = SCons.Options.Options('Lightning.py')
	opts.Add(SCons.Options.BoolOption('runtests', 'Default is to run unit tests, set "runtests=f" to disable', 1))
	opts.Add(SCons.Options.EnumOption('type', '"publish" creates an RC\n    "checkheaders" uncovers inclusion dependencies\n   ',
					'embedded', 
					allowed_values=('checkheaders', 'embedded', 'emulation', 'publish')))
	return opts


#-----------------------------------------------------------------------------
# Retrieve common command line options for a Lightning project
#-----------------------------------------------------------------------------
def RetrieveOptions(args, root_dir):

	is_runtests		= args.get('runtests', 1)
	type			= args.get('type', 'embedded')
	platform		= 'Lightning'
	
	is_emulation 			= type == 'emulation' or type == 'checkheaders'
	target_subdir			= platform + (is_emulation and '_emulation' or '')
	intermediate_build_dir	= os.path.join(root_dir, 'Temp', target_subdir)
	
	#FIXME/tp: Is this the best mechanism for allowing alternate nfsroot locations?
	rootfs = os.getenv('ROOTFS_PATH')
	if rootfs == None:
		rootfs = os.path.normpath(os.path.join(root_dir, '..', '..', 'nfsroot'))

	#FIXME/tp: add mods for type == 'publish' here
	if is_emulation:
		bin_deploy_dir	= os.path.join(root_dir, 'Build', target_subdir)
	else:
		bin_deploy_dir	= os.path.join(rootfs, 'usr', 'local', 'bin')
		SCons.Script.Default(bin_deploy_dir)
		
	if type == 'publish':
		print '*** type=publish is a placeholder (a noop at this time).'
	
	vars = {'type'					: type,
			'is_emulation'			: is_emulation,
			'is_runtests'			: is_runtests,
			'platform'				: platform,
			'target_subdir'			: target_subdir,
			'intermediate_build_dir': intermediate_build_dir,
			'bin_deploy_dir'		: bin_deploy_dir,
			'rootfs'				: rootfs,
	}
	
	return vars


#-----------------------------------------------------------------------------
# Setup the construction environment object to support Lightning defaults
# and CxxTest unit testing.
#-----------------------------------------------------------------------------
def CreateEnvironment(opts, vars):

	target_subdir	= vars['target_subdir']
	this_dir		= os.path.split(__file__)[0]
	cdevkit_dir		= os.path.normpath(os.path.join(this_dir, '..'))
	toolpath		= os.path.join(cdevkit_dir, 'Tools')
	
	platform_toolset = vars['platform'] + (vars['is_emulation'] and '_emulation' or '_embedded')

	env = SCons.Environment.Environment(options  = opts,
							tools = ['default', platform_toolset, 'cxxtest', 'runtest', 'checkheader'], 
							toolpath = [toolpath],
					 )
	env.Append(CPPDEFINES = ['LF2530BLUE'])
	#FIXME/tp: Keep this 'variant' field up to date!
	
	cdevkit_incpath	= os.path.join(cdevkit_dir, 'Include')
	ogl_incpath		= os.path.join(cdevkit_incpath, 'OpenGL', target_subdir)
	cpppaths  = [cdevkit_incpath, ogl_incpath]
	
	if vars['is_emulation']:
		cpppaths += [os.path.join(ogl_incpath, 'LinuxPC'), '/usr/X11R6/include']
	else:
		cpppaths += [os.path.join(cdevkit_incpath, 'ThirdParty', 'ustring')]
	
	
	libpaths = [os.path.join(cdevkit_dir, 'Libs', target_subdir, 'MPI'),
				os.path.join(cdevkit_dir, 'Libs', target_subdir, 'OpenGL')]

	env.Append(CPPDEFINES = ['LEAPFROG_CDEVKIT_ROOT=\\"'+cdevkit_dir+'\\"', 'OGLESLITE'])
	env.Append(CPPPATH = cpppaths)
	env.Append(LIBPATH = libpaths)
	env.Append(RPATH = libpaths + [os.path.join(cdevkit_dir, 'Libs', target_subdir, 'PrivMPI')])
	
	return env

#-------------------------------------------------------------------------
# Deploy the apprsrc assets for embedded builds
#-------------------------------------------------------------------------
def CopyResources(penv, vars):
	data_root = penv.Dir('#apprsrc').abspath
	root_len = len(data_root) + 1
	rootfs_data = os.path.join(vars['rootfs'], 'Cart1', 'rsrc')
	
	def callback(arg, directory, files):
		base = os.path.basename(directory)
		if base == '.svn':
			del files[:]
		else:
			for file in files:
				full = os.path.join(directory, file)
				if os.path.isfile(full):
					subdir = os.path.dirname(full[root_len:])
					penv.Install(os.path.join(rootfs_data, subdir), full)
					
	os.path.walk(data_root, callback, None)
	penv.Default(rootfs_data)

#-----------------------------------------------------------------------------
# Build a Module (either embedded or emulation target)
#
# Prepend any platform-specific libraries to the library list
# Install the library to the correct installation folder
#-----------------------------------------------------------------------------
def MakeMyApp(penv, ptarget, psources, plibs, vars):

	if len(psources) == 0:
		print '!!! Exiting, no source files specified in build'
		Exit(-2)
	
	# Set the include search paths for this app's folders
	bldenv = penv.Copy()
	source_dir = os.path.dirname(psources[0])
	bldenv.Append(CPPPATH = [source_dir, os.path.join(source_dir, 'Include')])

	# Add additional system libraries for the platform
	if vars['is_emulation']:
		platformlibs = ['gles_cl', 'glibmm-2.4', 'glib-2.0', 'Emulation']
	else:
		platformlibs = ['ogl', 'dl', 'pthread', 'ustring', 'iconv', 'intl', 'sigc-2.0']
		#FIXME/tp: pthread should go away (curr needed by BrioCube?)
		CopyResources(penv, vars)
	
	# Set up targets
	exe		= os.path.join(vars['intermediate_build_dir'], ptarget)
	mapfile = os.path.join(vars['intermediate_build_dir'], ptarget + '.map')
	bldenv.Append(LINKFLAGS = ' -Wl,-Map=' + mapfile)
	
	# Build and deploy the app (build it in two steps, so that the objs
	# can later be fed into a unit test
	bldenv.BuildDir(vars['intermediate_build_dir'], '.', 0)
	srcs =  map(lambda x: os.path.join(vars['intermediate_build_dir'], x), psources)
	
	objs  = bldenv.Object(srcs)
	myapp = bldenv.Program(exe, objs, LIBS = plibs + platformlibs)
	targets = vars['is_emulation'] and [myapp, mapfile] or [myapp]
	penv.Install(vars['bin_deploy_dir'], targets)
	
	return objs

	
#-----------------------------------------------------------------------------
# Export the all of the functions symbols
#-----------------------------------------------------------------------------
__all__ = ["SetupOptions", "RetrieveOptions", "CreateEnvironment", "MakeMyApp"] 

