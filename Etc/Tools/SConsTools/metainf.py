#-----------------------------------------------------------------------------
# Setup the MetaInf tool
#
# Looks for SVN_REVISION key in a template meta.inf and replaces
# it with the environment version value
#-----------------------------------------------------------------------------
import SCons.Builder
import string

#-----------------------------------------------------------------------------
# Add the tool(s) to the construction environment object
#-----------------------------------------------------------------------------
def generate(env):
	"""Add a Builder factory function and construction variables for
	creating meta.inf files to an Environment. Source is the template.
	The passed in env must have a version key"""

	def builder_metainf(target, source, env):
		if len(source) == 0:
			return 'Error, no sources given'

		#Taken from UpdateMetaInf.py
		f_template = open(source[0].abspath, 'r')
		s = f_template.read()
		s_template = string.Template(s)
		dct = {'SVN_REVISION':env['version']}
		f_meta = open(target[0].abspath, 'w')
		s_sub = s_template.substitute(dct)
		f_meta.write(s_sub)
		
		return 0

	bld = SCons.Builder.Builder(action = builder_metainf, env = env, single_source = True)
	env['BUILDERS']['MetaInf'] = bld


#-----------------------------------------------------------------------------
# Report the presence of the tool
#-----------------------------------------------------------------------------
def exists(env):
	return True
