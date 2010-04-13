#-----------------------------------------------------------------------------
# Setup the CxxTest tool
#
# Invokes the cxxtextgen.py text preprocessor that munges header files into a
# buildable CPP file.
#-----------------------------------------------------------------------------
import SCons.Builder

#FIXME/tp: Add a CXXTEST_EXE construction variable
cxxtestgen = '#ThirdParty/cxxtest/cxxtestgen.py'


#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):
    """Add a Builder factory function and construction variables for
    CxxTest to an Environment."""
    
    def builder_cxxtest(target, source, env, for_signature):
        if len(source) == 0:
            return ''
        path = env.File(cxxtestgen)
        return [[path, '--error-printer', '-o'] + target + source]

    bld = SCons.Builder.Builder(generator = builder_cxxtest, env = env)
    env['BUILDERS']['CxxTest'] = bld


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
     return env.Detect(cxxtestgen)
