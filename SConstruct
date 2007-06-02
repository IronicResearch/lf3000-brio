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
#	product and toolchain, e.g., FlyFusion, Lightning).  
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
import SCons.Defaults
import Etc.Tools.SConsTools.Priv.LfUtils	# LeapFrog utility functions


#-----------------------------------------------------------------------------
# Setup help options and get command line arguments
#-----------------------------------------------------------------------------
opts = Options('custom.py')
opts.Add(BoolOption('checkheaders', 'Set "checkheaders=t" to uncover order of inclusion issues', 0))
opts.Add(BoolOption('emulation', 'Set "emulation=t" to build the target using emulation', 0))
opts.Add(EnumOption('platform', 'Set platform to use', 'Lightning', 
						allowed_values=('Lightning')))
opts.Add(BoolOption('publish', 'Set "publish=t" to create a pulic release', 0))
opts.Add(BoolOption('runtests', 'Default is to run unit tests, set "runtests=f" to disable', 1))
opts.Add('setup', 'Set to "TRUNK" or branch name to setup source tree for a platform', '')

is_checkheaders	= ARGUMENTS.get('checkheaders', 0)
is_emulation	= ARGUMENTS.get('emulation', 0)
is_publish		= ARGUMENTS.get('publish', 0)
is_runtests		= ARGUMENTS.get('runtests', 1)
platform		= ARGUMENTS.get('platform', 'Lightning')
source_setup	= ARGUMENTS.get('setup', '')
branch			= source_setup != '' and source_setup or 'TRUNK'


#-----------------------------------------------------------------------------
# Only check headers in emulation mode
#-----------------------------------------------------------------------------
if is_checkheaders:
	is_emulation = 1
	is_publish   = 0


#-----------------------------------------------------------------------------
# Constant path values
#-----------------------------------------------------------------------------
version = Etc.Tools.SConsTools.Priv.LfUtils.GetRepositoryVersion(platform, branch)

root_dir				= Dir('#').abspath
publish_root			= Dir('#Publish_' + version).abspath
adjust_to_source_dir	= '../../../'
#FIXME/tp: Want EXTRA_LINUX_HEADER_DIR going forward?  Print when default or when override?
extra_driver_headers	= os.getenv('EXTRA_LINUX_HEADER_DIR')
if extra_driver_headers == None:
	extra_driver_headers  = Dir('#../LinuxDist/packages/drivers/include/linux').abspath


#-----------------------------------------------------------------------------
# Build one or more variants
#-----------------------------------------------------------------------------
variants = is_publish and ['emulation', 'embedded'] or is_emulation and ['emulation'] or ['embedded']
for target in variants:
	
	#-------------------------------------------------------------------------
	# Print banner and set the emulation variable
	#-------------------------------------------------------------------------
	print '***', platform, target, 'build  ( version', version, ') ***'
	is_emulation = (target == 'emulation') and 1 or 0
	
	
	#-------------------------------------------------------------------------
	# Setup intermediate build and deployment target folder names
	#-------------------------------------------------------------------------
	target_subdir			= platform + (is_emulation and '_emulation' or '')
	intermediate_build_dir	= os.path.join('Temp', target_subdir)
	#if is_emulation and is_publish:	#TP: reenable when deliver Release mode emu libs
	#	intermediate_build_dir += '_publish'
	debug_deploy_dir		= Dir(os.path.join('#Build', target_subdir)).abspath
	release_deploy_dir		= Dir(os.path.join('#Build', target_subdir + (is_emulation and '_publish' or ''))).abspath
	checkheaders_deploy_dir	= os.path.join(intermediate_build_dir, 'checkheaders')
	dynamic_deploy_dir		= is_publish and release_deploy_dir or debug_deploy_dir
	
	
	#-------------------------------------------------------------------------
	# Set up and Python dictionary variables that map generic names
	# to specific paths or library names.  This abstraction allows us to make
	# changes only here, not in all the SConscript files.  It also allows us to
	# support compiling with different compilers that need to link against
	# differently named libraries.
	#-------------------------------------------------------------------------
	vars	 = { 'platform'					: platform,
				 'is_emulation'				: is_emulation,
				 'is_publish'				: is_publish,
				 'is_checkheaders'			: is_checkheaders,
				 'is_runtests'				: is_runtests,
				 'adjust_to_source_dir'		: adjust_to_source_dir,
				 'dynamic_deploy_dir'		: dynamic_deploy_dir,
				 'intermediate_build_dir'	: intermediate_build_dir,
				 'publish_root'				: publish_root,
				 'publish_inc_dir'			: os.path.join(publish_root, 'Include'),
				 'publish_lib_dir'			: os.path.join(publish_root, 'Libs', target_subdir),
				 'target_subdir'			: target_subdir,
				 'extra_driver_headers'		: extra_driver_headers,
			   }
	
	
	#------------------------------------------------------------------------
	# Setup the environment object
	#-------------------------------------------------------------------------
	platform_toolset = platform + (is_emulation and '_emulation' or '_embedded')
	toolpath1 = Dir(os.path.join(root_dir, 'Etc', 'Tools', 'SConsTools')).abspath
	toolpath2 = os.path.join(toolpath1, 'Priv')
	env = Environment(	options  = opts,
						tools = ['default', platform_toolset, 
								'checkheader', 'cxxtest', 'runtest'], 
						toolpath = [toolpath1, toolpath2],
						LIBPATH = [os.path.join(dynamic_deploy_dir, 'MPI')],
					 )
	
	
	#-------------------------------------------------------------------------
	# Allow per-user overrides of global settings
	#-------------------------------------------------------------------------
	override = __import__('UserOverrides', globals(), locals(), [''])
	if override.__dict__.has_key(platform_toolset):
		override.__dict__[platform_toolset](env)
	
	#print env.Dump()
	
	
	#-------------------------------------------------------------------------
	# Attach the help to the Environment object
	#-------------------------------------------------------------------------
	Help(opts.GenerateHelpText(env))
	
	
	#-------------------------------------------------------------------------
	# Export environment variables to the SConscript files
	#-------------------------------------------------------------------------
	Export('env vars')
	
	
	#-------------------------------------------------------------------------
	# Setup build targets for modules
	#
	# Use "duplicate=0" to prevent SCons from copying the source files.
	# If you don't do this, the problems pane in Eclipse opens up the copied file,
	# not the true source, so you end up fixing problems in a file that will get
	# overwritten on the next build.
	#-------------------------------------------------------------------------
	SConscript(os.path.join(root_dir, 'System', 'SConscript'), duplicate=0)
	SConscript(os.path.join(root_dir, 'ThirdParty', 'SConscript'), duplicate=0)
	SConscript(os.path.join(root_dir, 'Etc', 'SConscript'), duplicate=0)
	SConscript(os.path.join(root_dir, platform, 'SConscript'), duplicate=0)

# END OF VARIANT LOOP



#-----------------------------------------------------------------------------
# Handle normal and special targets (default build all)
#-----------------------------------------------------------------------------
Default('..')
if is_checkheaders:
	env.Append(CPPPATH = ['ThirdParty/PowerVR/Include', 'ThirdParty/PowerVR/Include/LinuxPC'])
	Etc.Tools.SConsTools.lfutils.CheckHeaders(root_dir, checkheaders_deploy_dir, env)
	Default(None)
	Default(checkheaders_deploy_dir)
elif source_setup:
	Etc.Tools.SConsTools.lfutils.CreateSourceTree(root_dir, platform, source_setup, env)
