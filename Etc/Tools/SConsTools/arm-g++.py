#-----------------------------------------------------------------------------
# Setup the ARM g++ tool chain
#
# Setup the arm g++ compiler path with any settings that will be common across
# all platforms.  Put platform specific flags (optimization, etc.) under a
# "<Platform>_toolset.py" file.
#-----------------------------------------------------------------------------
import os.path
import re

import SCons.Defaults
import SCons.Tool
import SCons.Util

#-----------------------------------------------------------------------------
# Inherit properties from the following tools
#-----------------------------------------------------------------------------
parent = __import__('SCons.Tool.g++', globals(), locals(), [''])

# FIXME/dm: Should not have to use explicit path to arm-linux-g++ compiler
compiler = '/opt/RidgeRun/arm-eabi-uclibc/bin/arm-linux-g++'

#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):
	"""Add Builders and construction variables for arm-g++ to an Environment.
	The first three lines are boilerplate to inherit from the default g++ implementation.
	"""
	static_obj, shared_obj = SCons.Tool.createObjBuilders(env)
	parent.generate(env)
	env['AS']	= compiler
	env['CC']	= compiler
	env['CXX']	= compiler
	env['LINK']	= compiler
	
	# Platform specific settings
#	env['CCFLAGS']		= SCons.Util.CLVar('$CCFLAGS') + '-Wall -fmessage-length=0'
#	FIXME/BSK Removed the 'Wall' warning
	env['CCFLAGS']		= SCons.Util.CLVar('$CCFLAGS') + '-fmessage-length=0'
	env['SHCXXFLAGS']	= SCons.Util.CLVar('$CXXFLAGS')	
	env['RPATHPREFIX']	= '-Wl,-rpath-link='

	# determine the compiler version
	if env['CXX']:
		line = os.popen(env['CXX'] + ' --version').readline()
		match = re.search(r'[0-9]+(\.[0-9]+)+', line)
		if match:
			env['CXXVERSION'] = match.group(0)


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
	return env.Detect(compiler)
