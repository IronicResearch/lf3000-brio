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
	env.Append(CCFLAGS = '-O4 -mcpu=arm926ej-s')
	env.Append(LIBS = ['libustring','libiconv','libintl','libsigc'])
	root = os.path.normpath(os.path.join(__file__, '../../../../ThirdParty/ustring'))
	env.Append(LIBPATH = [os.path.join(root, 'libs', 'arm')])
	env.Append(CPPPATH = [root])

	rootfs = os.getenv('ROOTFS_PATH')
	if rootfs == None:
		rootfs = os.path.normpath(os.path.join(__file__, '../../../../../../nfsroot'))
	env.Append(CPPPATH = [os.path.join(rootfs, 'usr', 'include')])	
	env.Append(LIBPATH = [os.path.join(rootfs, 'usr', 'lib'),
							os.path.join(rootfs, 'usr', 'local', 'lib')])	

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
