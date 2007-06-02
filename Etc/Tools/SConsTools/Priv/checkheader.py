#-----------------------------------------------------------------------------
# Setup the CxxTest tool
#
# Invokes the cxxtextgen.py text preprocessor that munges header files into a
# buildable CPP file.
#-----------------------------------------------------------------------------
import SCons.Builder
import SCons.Tool
import SCons.Defaults
import SCons.Util

SourceFileScanner = SCons.Scanner.Base({}, name='SourceFileScanner')

#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):
	"""Replicate normal .cpp->.o actions for .h files to test compile headers"""

	static_obj = SCons.Builder.Builder(action = {},
										emitter = {},
										prefix = '',
										suffix = '.test.o',
										src_builder = ['CFile', 'CXXFile'],
										single_source = 1)
	
	static_obj.add_action('.h', SCons.Defaults.CXXAction)
	env['BUILDERS']['CheckHeader'] = static_obj


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
     return true
