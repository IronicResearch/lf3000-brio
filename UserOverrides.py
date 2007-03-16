#-----------------------------------------------------------------------------
# Provide user overrides of global platform settings.
#-----------------------------------------------------------------------------
import os.path
import re

#-----------------------------------------------------------------------------
# Override settings in the construction environment object
#-----------------------------------------------------------------------------
def LightningGCC_embedded(env):
	env['CCFLAGS']		+= ''
	env['CPPPATH']		+= []
	env['LIBPATH']		+= []

def LightningGCC_emulation(env):
	env['CCFLAGS']		+= ''
	env['CPPPATH']		+= []
	env['LIBPATH']		+= []
