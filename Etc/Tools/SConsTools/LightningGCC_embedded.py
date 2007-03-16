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
	env.Append(CCFLAGS = '-O4')
	


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
