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
	kBrioLightningRepositoryURL	= 'http://source.leapfrog.com/Software/Brio/Lightning/'
	return {
#			kBrioSystemRepositoryURL : 'System',
#			kBrioLightningRepositoryURL : 'Lightning'
			}

#-----------------------------------------------------------------------------
# Set all default build parameters that are common to both embedded and
# emulation builds here.
#-----------------------------------------------------------------------------
def SetPlatformFlags(env):
	env.Append(CCFLAGS = '')
	env.Append(CPPDEFINES = 'LIGHTNING')

def SetPlatformIncludePaths(env):
	env.Append(CPPPATH = ['#System/Include', 
						'#System/IncludePriv'])

def SetPlatformLibraryPaths(env):
	env.Append(LIBPATH = '')
	