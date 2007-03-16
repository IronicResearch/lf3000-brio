#-----------------------------------------------------------------------------
# Run a unit test
#
# Runs the unit test exe and updates a <test>_passed file on success.
#-----------------------------------------------------------------------------
import SCons.Builder

#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):
    
    def builder_unit_test(target, source, env):
        app = str(source[0].abspath)
        if os.spawnl(os.P_WAIT, app, app)==0:
            open(str(target[0]),'w').write("PASSED\n")
        else:
            return 1

    bld = SCons.Builder.Builder(action = builder_unit_test)
    env['BUILDERS']['RunTest'] = bld


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
     return 1
