#-----------------------------------------------------------------------------
# Setup common embedded/emulation settings for the Lightning platform
#-----------------------------------------------------------------------------
import os.path
import re

import SCons.Defaults
import SCons.Tool
import SCons.Util

#-----------------------------------------------------------------------------
# Set mappings of repository to relative path
#-----------------------------------------------------------------------------
def GetRepositoryMappings():
	kBrioSystemRepositoryURL	= 'http://source.leapfrog.com/Software/Brio/Brio2/'
	return {
			kBrioSystemRepositoryURL : 'System',
	}

#-----------------------------------------------------------------------------
# Set all default build parameters that are common to both embedded and
# emulation builds here.
#-----------------------------------------------------------------------------
def SetPlatformFlags(env):
#	env.Append(CCFLAGS = ' -nostdinc -Wextra -Weffc++ -Wmissing-format-attribute')
	env.Append(CCFLAGS = ' -ansi -Wno-long-long -Werror -pedantic-errors -Wno-variadic-macros -Wformat -Wmissing-format-attribute')
#FIXME/BSK
#	env.Append(CCFLAGS = ' -ansi -Wextra -Wno-long-long -Werror -pedantic-errors -Wno-variadic-macros -Wformat -Wmissing-format-attribute')
	env.Append(CPPDEFINES = ['LIGHTNING', 'LF_USE_CPP_NAMESPACES', 'SET_DEBUG_LEVEL_DISABLE'])

def SetPlatformIncludePaths(env):
	env.Append(CPPPATH = ['#System/Include', 
						'#System/IncludePriv',
                        '#ThirdParty/boost'])

def SetPlatformLibraryPaths(env):
	env.Append(LIBPATH = '')
	
