#-----------------------------------------------------------------------------
# Provide user overrides of global platform settings.
#
# This file is ONLY for TEMPORARY testing.  For example, you might want to
# turn up your default debug flag settings and rebuild all, or add profiling
# flags and rebuild all.
#
# If you want to make a platform-wide change, do that in the
# Etc/Tools/SConsTools/<platform>_<type>.py files, where <type> is
# "embedded", "emulation", or "common" if it applies to both emulation and
# embedded builds.
#-----------------------------------------------------------------------------
import os.path
import re

#-----------------------------------------------------------------------------
# Override settings in the construction environment object
# Provide a single function here for platform, and modify the construction
# environment for that platform.  Using Python string parsing and list 
# manipulations, you can add, remove and modify the switches sent to the 
# compiler
#-----------------------------------------------------------------------------
def Lightning_embedded(env):
	env['CCFLAGS']		+= ''
	env['CPPPATH']		+= []
	env['LIBPATH']		+= []

def Lightning_emulation(env):
	env['CCFLAGS']		+= ''
	env['CPPPATH']		+= []
	env['LIBPATH']		+= []
