#-----------------------------------------------------------------------------
# Setup the Lightning emulation gcc tool chain
#-----------------------------------------------------------------------------
import os.path
import re

import SCons.Defaults
import SCons.Tool
import SCons.Util
import SCons.Node

common = __import__(__name__.replace('emulation', 'common'), globals(), locals(), [''])

#-----------------------------------------------------------------------------
# This is the primary function in which to modify global settings for the
# platform.  The rest of the file is boilerplate.
#-----------------------------------------------------------------------------
def PlatformMods(env):
	common.SetPlatformFlags(env)
	common.SetPlatformIncludePaths(env)
	common.SetPlatformLibraryPaths(env)
	
	gcc_defs 			= env.Split('')
	emulation_defs		= env.Split('EMULATION')
	env.Append(CPPDEFINES = gcc_defs + emulation_defs)
	env.Append(CPPDEFINES = ['KHRONOS'])
	
#	env.Append(CCFLAGS = '-g -Wextra')
	env.Append(CCFLAGS = '-g' )

	env.Append(LINKFLAGS = '-g')
	env.ParseConfig('pkg-config --cflags --libs glib-2.0')
	env.ParseConfig('pkg-config --cflags --libs glibmm-2.4')
	
	#FIXME: Should probably be in an emulation toolchain file that's shared...
	env.Append(CXXFLAGS = SCons.Util.CLVar( os.getenv('EMULATION_CXXFLAGS') ) )
	env.Append(LDFLAGS = SCons.Util.CLVar( os.getenv('EMULATION_LDFLAGS') ) )
	#Don't optimize in emulation environment
	env.Append(CXXFLAGS = '-O0')

	
#-----------------------------------------------------------------------------
# Inherit properties from the following tools
#-----------------------------------------------------------------------------
parent = __import__('SCons.Tool.g++', globals(), locals(), [''])

#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):
	"""NOTE: We can default to the standard Linux g++ toolchain here, and only
	override specific compiler and linker flags."""
	PlatformMods(env)


#-----------------------------------------------------------------------------
# Report the presence of the tool(s)
#-----------------------------------------------------------------------------
def exists(env):
	return parent.exists(env)
