#-----------------------------------------------------------------------------
# Setup the Lightning gcc tool chain
#-----------------------------------------------------------------------------
import os.path
import re

import SCons.Defaults
import SCons.Tool
import SCons.Util

common = __import__(__name__.replace('embedded', 'common'), globals(), locals(), [''])

#-----------------------------------------------------------------------------
# This is the primary function in which to modify global settings for the
# platform.  The rest of the file is boilerplate.
#-----------------------------------------------------------------------------
def PlatformMods(env):
	common.SetPlatformFlags(env)
	common.SetPlatformIncludePaths(env)
	common.SetPlatformLibraryPaths(env)
	gcc_defs 			= env.Split('')
	env.Append(CPPDEFINES = gcc_defs)
	env.Append(CPPDEFINES = ['_FILE_OFFSET_BITS=64', 'KHRONOS', 'linux', 'USTRING'])
	env.Append(CCFLAGS = '-O3 -fno-strict-aliasing -mcpu=cortex-a9')
	##env.Append(LIBS = ['libustring','libiconv','libintl','libsigc-2.0'])

	#TODO: Fixup this relative path
	root = os.path.normpath(os.path.join(__file__, '../../../../ThirdParty/ustring'))
#	env.Append(LIBPATH = [os.path.join(root, 'libs', 'arm')])
#	env.Append(LIBPATH = [os.path.join(root, 'usr', 'local', 'lib')])
#	env.Append(RPATH = [os.path.join(root, 'usr', 'local', 'lib')])
	env.Append(CPPPATH = [root])

	env.Append(CPPPATH = [ env['staging_dir'].Dir('usr').Dir('include') ] )

	extinc = os.getenv('EXTRA_LINUX_HEADER_DIR')
	if extinc != None:
		env.Append(CPPPATH = [extinc])	

#-----------------------------------------------------------------------------
# Inherit properties from the following tools
#-----------------------------------------------------------------------------
parent = __import__('arm-g++', globals(), locals(), [''])

#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):
	static_obj, shared_obj = SCons.Tool.createObjBuilders(env)
	parent.generate(env)
	PlatformMods(env)

#-----------------------------------------------------------------------------
# Report the presence of a tool(s)
#-----------------------------------------------------------------------------
def exists(env):
	return parent.exists(env)
