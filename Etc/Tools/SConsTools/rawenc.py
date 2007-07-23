#-----------------------------------------------------------------------------
# Setup the Brio sound file -> Brio tool, which extracts PCM data and 
# wraps a header on it
#-----------------------------------------------------------------------------
import SCons.Builder
import os.path

this_dir	= os.path.split(__file__)[0]
parent_dir	= os.path.split(this_dir)[0]
path1		= os.path.join(this_dir, 'sf2brio')
path2		= os.path.join(parent_dir, 'sf2brio')
rawenc		= os.path.exists(path1) and path1 or path2


#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
# NOTE: Prefix the sf2brio command with '-' to ignore its return status,
# because it incorrectly returns non-zero on success.
#-----------------------------------------------------------------------------
act = SCons.Action.Action([['-' + rawenc, '$SOURCE', '$TARGET']])

def generate(env):
    bld = SCons.Builder.Builder(action = act, single_source=1)
    env['BUILDERS']['RawEnc'] = bld


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
     return env.Detect(rawenc)
