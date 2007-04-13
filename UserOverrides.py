#-----------------------------------------------------------------------------
# Provide user overrides of global platform settings.
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
def LightningGCC_embedded(env):
	env['CCFLAGS']		+= ''
	env['CPPPATH']		+= []
	env['LIBPATH']		+= []

def LightningGCC_emulation(env):
	env['CCFLAGS']		+= ''
	env['CPPPATH']		+= []
	env['LIBPATH']		+= []
