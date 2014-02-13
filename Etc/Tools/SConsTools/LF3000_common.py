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
#	env.Append(CCFLAGS = ' -ansi -Wno-long-long -Werror -pedantic-errors -Wno-variadic-macros -Wformat -Wmissing-format-attribute')
#FIXME/BSK Added the 'Wextra' warning
#	env.Append(CCFLAGS = ' -ansi -Wextra -Wno-long-long -Werror -pedantic-errors -Wno-variadic-macros -Wformat -Wmissing-format-attribute')
	env.Append(CCFLAGS = ' -ansi -Wno-long-long -Werror -Wno-variadic-macros -Wformat -Wmissing-format-attribute')
	env.Append(CPPDEFINES = ['LIGHTNING', 'LF_USE_CPP_NAMESPACES', 'SET_DEBUG_LEVEL_DISABLE', 'LF3000'])

def SetPlatformIncludePaths(env):
	env.Append(CPPPATH = [os.path.join('#', 'System', 'Include'),
			      os.path.join('#', 'System', 'IncludePriv'),
                              os.path.join('#', 'ThirdParty', 'boost', 'Include') ] )

def SetPlatformLibraryPaths(env):
	env.Append(LIBPATH = '')
	