#-----------------------------------------------------------------------------
# Setup the Ogg Vorbis Audio encoder tool
#-----------------------------------------------------------------------------
import SCons.Builder
import os.path

this_dir	= os.path.split(__file__)[0]
parent_dir	= os.path.split(this_dir)[0]
path1		= os.path.join(this_dir, 'oggenc')
path2		= os.path.join(parent_dir, 'oggenc')
oggenc		= os.path.exists(path1) and path1 or path2



#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
act = SCons.Action.Action([['LD_LIBRARY_PATH=' + this_dir, oggenc, '--resample', '$OGGENC_RATE', '-q', '$OGGENC_COMPRESSION', '-o', '$TARGET', '$SOURCE']])

def generate(env):
    bld = SCons.Builder.Builder(action = act, single_source=1)
    env['BUILDERS']['OggEnc'] = bld


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
     return env.Detect(oggenc)
