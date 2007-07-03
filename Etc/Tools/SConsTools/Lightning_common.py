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
#	env.Append(CCFLAGS = ' -nostdinc -Wextra')
	env.Append(CCFLAGS = ' -Werror')
	env.Append(CPPDEFINES = 'LIGHTNING')

def SetPlatformIncludePaths(env):
	env.Append(CPPPATH = ['#System/Include', 
						'#System/IncludePriv',
                        '#ThirdParty/boost'])

def SetPlatformLibraryPaths(env):
	env.Append(LIBPATH = '')
	
