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
#
#-----------------------------------------------------------------------------
# Logic for various "type" build targets:
#
#   Build Type    | Embedded | Emulation | Dbg/Rel | Runtests | DeployDir
#   --------------+----------+-----------+---------+----------+-----------
#   checkheaders  |          |     X     |    D    |    N     | N/A
#   embedded      |    X     |           |    D    |    N     | Build/<Platform>
#   emulation     |          |     X     |    D    |    Y     | Build/<Platform>_emulation
#   xembedded     |    X     |           |    D    |    N     | XBuild/Libs/<Platform>
#   xemulation    |          |     X     |    D    |    N     | XBuild/Libs/<Platform>_emulation
#   publish       |    X     |     X     |    R    |    Y     | Publish_<NNN>/Libs
#
# FIXME/tp: Turn of debug flags for publish builds
# FIXME/tp: Add Doxygen support
# FIXME/tp: Add version number to release notes
#
# Note: Embedded builds deploy the Module libraries to the "rootfs" folder only.
# Note: Embedded builds deploy the MPI libraries to both the "DeployDir" folder
#       in the table above and the "rootfs" folder only.
# 
# In addition "publish" builds will:
#   Version the Publush_<NNN> folder using the Brio2 repository revision number
#   Add that version to various release notes docs
#   Run Doxygen go generate documentation
#
#=============================================================================
import os
import fnmatch
import SCons.Defaults
import Etc.Tools.SConsTools.Priv.LfUtils	# LeapFrog utility functions


#-----------------------------------------------------------------------------
# Setup help options and get command line arguments
#-----------------------------------------------------------------------------
opts = Options('CmdLine.py')
opts.Add('platform', 'Set platform to use', 'Lightning')
opts.Add(EnumOption('platform_variant', 'Use in the place of "platform" to specify a bring-up board\n   ', 'Lightning_LF2530BLUE', 
						allowed_values=('Lightning_LF2530RED', 'Lightning_LF2530BLUE', 'Lightning_LF1000')))
opts.Add(BoolOption('runtests', 'Default is to run unit tests, set "runtests=f" to disable', 1))
opts.Add('setup', 'Set to "TRUNK" or branch name to setup source tree for a platform', '')
opts.Add(EnumOption('type', '"publish" creates an RC\n    "xembedded" and "xemulation" export headers, libs & build scripts\n    for external app linkage\n    "checkheaders" uncovers inclusion dependencies\n   ',
					'embedded', 
					allowed_values=('checkheaders', 'embedded', 'emulation', 'xembedded', 'xemulation', 'publish')))

platform			= ARGUMENTS.get('platform', '')
platform_variant	= ARGUMENTS.get('platform_variant', 'Lightning_LF2530BLUE')
source_setup		= ARGUMENTS.get('setup', '')
is_runtests			= ARGUMENTS.get('runtests', 1)
type				= ARGUMENTS.get('type', 'embedded')
variant				= ''

if platform != '' and platform_variant != '':
	print '*** Exiting: Error!  Do not set the "platform" parameter if "platform_variant" is set'
	Exit(-1)
	
if platform == '' and platform_variant == '':
	print '*** Exiting: Error!  Need to set either "platform" or "platform_variant"'
	Exit(-1)
	
if platform_variant != '':
	idx = platform_variant.find('_')
	if idx == -1:
		print '*** Exiting: Error!  "platform_variant" needs to be for form <platform>_<variant>'
	platform = platform_variant[:idx]
	variant = platform_variant[idx+1:]

branch				= source_setup != '' and source_setup or 'TRUNK'


#-----------------------------------------------------------------------------
# Constant path values
#-----------------------------------------------------------------------------

root_dir				= Dir('#').abspath
export_root				= Dir('#XBuild').abspath
adjust_to_source_dir	= '../../../'

rootfs = os.getenv('ROOTFS_PATH')
if rootfs == None:
	rootfs = Dir('#../../nfsroot').abspath


#-----------------------------------------------------------------------------
# Build one or more target variants
#-----------------------------------------------------------------------------
is_checkheaders		= type == 'checkheaders'
is_publish			= type == 'publish'
is_export			= is_publish or type == 'xembedded' or type == 'xemulation'
is_runtests         = (is_runtests == 1) and (type == 'emulation' or is_publish)
publish_root		= ''
if is_publish:
	version = Etc.Tools.SConsTools.Priv.LfUtils.GetRepositoryVersion(platform, branch)
	publish_root			= Dir('#Publish_' + version).abspath

targets = ['emulation']
if type == 'publish':
	targets = ['emulation', 'embedded']
elif type == 'embedded' or type == 'xembedded':
	targets = ['embedded']

for target in targets:
	
	#-------------------------------------------------------------------------
	# Setup intermediate build and deployment target folder names
	#-------------------------------------------------------------------------
	is_emulation			= (target == 'emulation') and 1 or 0
	target_subdir			= platform + (is_emulation and '_emulation' or '')
	intermediate_build_dir	= os.path.join('Temp', target_subdir)
	publish_lib_dir			= os.path.join(publish_root, 'Libs', target_subdir)
	build_base				= Dir(os.path.join('#Build', target_subdir)).abspath
	xbuild_base				= os.path.join(export_root, 'Libs', target_subdir)
	hdr_deploy_dir			= ''

	if is_publish:
		mpi_deploy_dir		= os.path.join(publish_lib_dir, 'MPI')
		priv_mpi_deploy_dir	= os.path.join(publish_lib_dir, 'PrivMPI')
		lib_deploy_dir		= os.path.join(publish_lib_dir, 'Module')
		hdr_deploy_dir 	 	= os.path.join(publish_root, 'Include')
	elif is_export:
		mpi_deploy_dir		= os.path.join(xbuild_base, 'MPI')
		priv_mpi_deploy_dir	= os.path.join(xbuild_base, 'PrivMPI')
		lib_deploy_dir		= os.path.join(xbuild_base, 'Module')
		hdr_deploy_dir 		= os.path.join(export_root, 'Include')
	else:
		mpi_deploy_dir		= os.path.join(build_base, 'MPI')
		priv_mpi_deploy_dir	= os.path.join(build_base, 'PrivMPI')
		lib_deploy_dir		= os.path.join(build_base, 'Module')
		
	bin_deploy_dir	= mpi_deploy_dir
		
	if not is_emulation:
		bin_deploy_dir		= os.path.join(rootfs, 'usr', 'local', 'bin')
		lib_deploy_dir		= os.path.join(rootfs, 'usr', 'local', 'lib')
		priv_mpi_deploy_dir	= lib_deploy_dir
		Default(bin_deploy_dir)
		Default(lib_deploy_dir)

	
	#-------------------------------------------------------------------------
	# NOTE: When "publish=t" the SConscript files get invoked multiple times, 
	# and we will get SCons warnings it we perform Install() actions multiple
	# times on the same files, so turn off the export variable after the first
	# pass.
	#-------------------------------------------------------------------------
	if targets.index(target) > 0:
		is_export = 0
	if is_publish and not is_emulation:
		is_runtests = 0;

	#-------------------------------------------------------------------------
	# Set up and Python dictionary variables that map generic names
	# to specific paths or library names.  This abstraction allows us to make
	# changes only here, not in all the SConscript files.  It also allows us
	# to support compiling with different compilers that need to link against
	# differently named libraries.
	#-------------------------------------------------------------------------
	vars	 = { 'platform'					: platform,
				 'is_emulation'				: is_emulation,
				 'is_publish'				: is_publish,
				 'is_export'				: is_export,
				 'is_checkheaders'			: is_checkheaders,
				 'is_runtests'				: is_runtests,
				 'adjust_to_source_dir'		: adjust_to_source_dir,
				 'mpi_deploy_dir'			: mpi_deploy_dir,
				 'priv_mpi_deploy_dir'		: priv_mpi_deploy_dir,
				 'lib_deploy_dir'			: lib_deploy_dir,
				 'bin_deploy_dir'			: bin_deploy_dir,
				 'hdr_deploy_dir'			: hdr_deploy_dir,
				 'intermediate_build_dir'	: intermediate_build_dir,
				 'export_root'				: is_publish and publish_root or export_root,
				 'target_subdir'			: target_subdir,
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
						LIBPATH = [mpi_deploy_dir, priv_mpi_deploy_dir],
					 )
	if variant != '':
		env.Append(CPPDEFINES = [variant])
	
	
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

	
	#-------------------------------------------------------------------------
	# Deploy the unit test data for embedded builds
	#-------------------------------------------------------------------------
	if not is_emulation:
		unit_test_data_root = Dir('UnitTestData').abspath
		root_len = len(unit_test_data_root) + 1
		rootfs_data = os.path.join(rootfs, 'Base', 'rsrc')
		
		def callback(arg, directory, files):
			base = os.path.basename(directory)
			if base == '.svn':
				del files[:]
			else:
				for file in files:
					full = os.path.join(directory, file)
					if os.path.isfile(full):
						subdir = os.path.dirname(full[root_len:])
 						env.Install(os.path.join(rootfs_data, subdir), full)

		os.path.walk(unit_test_data_root, callback, None)
		Default(rootfs_data)
			
# END OF VARIANT LOOP



#-----------------------------------------------------------------------------
# Handle normal and special targets (default build all)
#-----------------------------------------------------------------------------
Default('..')
if is_checkheaders:
	env.Append(CPPPATH = ['ThirdParty/PowerVR/Include', 'ThirdParty/PowerVR/Include/LinuxPC'])
	checkheaders_deploy_dir	= os.path.join(intermediate_build_dir, 'checkheaders')
	Etc.Tools.SConsTools.Priv.LfUtils.CheckHeaders(root_dir, checkheaders_deploy_dir, env)
	Default(None)
	Default(checkheaders_deploy_dir)
elif source_setup:
	Etc.Tools.SConsTools.Priv.LfUtils.CreateSourceTree(root_dir, platform, source_setup, env)
