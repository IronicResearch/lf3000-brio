#-----------------------------------------------------------------------------
# LightningUtils.py
#
# Common utilities for use by top level SConstruct files
#-----------------------------------------------------------------------------
import os
import sys
import SCons
import SCons.Util
import SCons.Options

global_dir = os.path.normpath(os.path.join(__file__, '../../../..', 'Global'))
sys.path.append(global_dir)
	

#-----------------------------------------------------------------------------
# Setup common command line options for a Chorus project
#-----------------------------------------------------------------------------
def SetupOptions():
	opts = SCons.Options.Options('Lightning.py')
	opts.Add(SCons.Options.BoolOption('emulation', 'Set "emulation=t" to build the target using emulation', 0))
	opts.Add(SCons.Options.BoolOption('publish', 'Set "publish=t" to create a pulic release', 0))
	opts.Add(SCons.Options.BoolOption('runtests', 'Default is to run unit tests, set "runtests=f" to disable', 1))
	opts.Add('deploy_dir', '''Default deployment directory is your project's "Build" folder''', '')
	return opts


#-----------------------------------------------------------------------------
# Compile headers alone to expose order-of-inclusion issues.
#-----------------------------------------------------------------------------
def RetrieveOptions(args, root_dir):

	is_emulation	= args.get('emulation', 0)
	is_publish		= args.get('publish', 0)
	is_runtests		= args.get('runtests', 1)
	platform		= 'Lightning'
	
	#FIXME/tp: want a separate deploy dir argument???
	deploy_dir		= args.get('deploy_dir', '')
	
	target_subdir			= platform + (is_emulation and '_emulation' or '')
	intermediate_build_dir	= os.path.join(root_dir, 'Temp', target_subdir)
	if deploy_dir == '':
		deploy_dir = os.path.join(root_dir, 'Build', target_subdir)
	
	vars = {'is_emulation'			: is_emulation,
			'is_publish'			: is_publish,
			'is_runtests'			: is_runtests,
			'platform'				: platform,
			'target_subdir'			: target_subdir,
			'intermediate_build_dir': intermediate_build_dir,
			'deploy_dir'			: deploy_dir,
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
							tools = ['default', platform_toolset, 'cxxtest', 'runtest'], 
							toolpath = [toolpath],
					 )
	
	cdevkit_incpath	= os.path.join(cdevkit_dir, 'Include')
	ogl_incpath		= os.path.join(cdevkit_incpath, 'OpenGL', target_subdir)
	cpppaths  = [cdevkit_incpath, ogl_incpath]
	
	if vars['is_emulation']:
		cpppaths += [os.path.join(ogl_incpath, 'LinuxPC'), '/usr/X11R6/include']
	
	libpaths = [os.path.join(cdevkit_dir, 'Libs', target_subdir, 'MPI'),
				os.path.join(cdevkit_dir, 'Libs', target_subdir, 'OpenGL')]

	env.Append(CPPDEFINES = ['LEAPFROG_CDEVKIT_ROOT=\\"'+cdevkit_dir+'\\"', 'OGLESLITE'])
	env.Append(CPPPATH = cpppaths)
	env.Append(LIBPATH = libpaths)
	env.Append(RPATH = libpaths)
	
	return env


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
		platformlibs = ['ogl', 'dl', 'pthread']
		#FIXME/tp: pthread should go away (curr needed by BrioCube?)
	
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
	penv.Install(vars['deploy_dir'], [myapp, mapfile])
	
	return objs

	
#-----------------------------------------------------------------------------
# Export the all of the functions symbols
#-----------------------------------------------------------------------------
__all__ = ["SetupOptions", "RetrieveOptions", "CreateEnvironment", "MakeMyApp"] 

