#-----------------------------------------------------------------------------
# lfutils.py
#
# Common utilities for use by top level SConstruct files
#-----------------------------------------------------------------------------
import os
import SCons.Util

#-----------------------------------------------------------------------------
# Enumerate folders that should be ignored when doing a test compile of headers
# TODO/tp: Provide user override of this list?
#-----------------------------------------------------------------------------
exclude_dirs_for_header_compile = [
	'.svn',
	'Temp',
	'Output',
	'Publish', 
	'tests',
	'disabled',
	'ThirdParty',
	'.metadata',
	'.settings']


#-----------------------------------------------------------------------------
# Compile headers alone to expose order-of-inclusion issues.
#-----------------------------------------------------------------------------
def CheckHeaders(root, deploy_dir, env):

	def CheckInFolder(args, dirname, filenames):
		# Look for known folders that should be excluded from this search.
		# If we find one, make sure none of its subfolders gets searched.
		base = os.path.basename(dirname)
		if base in exclude_dirs_for_header_compile:
			del filenames[:]
		else:
			rootlen = args[0]
			deploy_dir = args[1]
			env = args[2]
			for x in filenames:
				if os.path.splitext(x)[1] == '.h' and os.path.normcase(x) != 'stdafx.h':
					relpath = dirname[rootlen:]
					path = os.path.join(os.getcwd(), dirname, x)
					dest = os.path.join(deploy_dir, relpath, x + '.test.o')
					env.CheckHeader(target = dest, source = path)
	
	os.path.walk(root, CheckInFolder, [len(root)+1, deploy_dir, env])


#-----------------------------------------------------------------------------
# Create source tree for a platform
#-----------------------------------------------------------------------------
def CreateSourceTree(root, platform, branch, env):
	common = __import__(platform + '_common', globals(), locals(), [''])
	mappings = common.GetRepositoryMappings()
	for repo, reldir in mappings.iteritems():
		command = 'svn co ' + os.path.join(repo, branch) + ' ' + os.path.join(root, reldir)
		env.Execute(command)


#-----------------------------------------------------------------------------
# Create source tree for a platform
#-----------------------------------------------------------------------------
def SourceDirFromBuildDir(target, base):
    """
    Return a relative path to the target from either the current dir or an optional base dir.
    Base can be a directory specified either as absolute or relative to current dir.
    """

    base_list = (os.path.abspath(base)).split(os.sep)
    target_list = (os.path.abspath(target)).split(os.sep)

    # On the windows platform the target may be on a completely different drive from the base.
    if os.name in ['nt','dos','os2'] and base_list[0] <> target_list[0]:
        raise OSError, 'Target is on a different drive to base. Target: '+target_list[0].upper()+', base: '+base_list[0].upper()

    # Starting from the filepath root, work out how much of the filepath is
    # shared by base and target.
    for i in range(min(len(base_list), len(target_list))):
        if base_list[i] <> target_list[i]: break
    else:
        # If we broke out of the loop, i is pointing to the first differing path elements.
        # If we didn't break out of the loop, i is pointing to identical path elements.
        # Increment i so that in all cases it points to the first differing path elements.
        i+=1

    rel_list = [os.sep] + base_list + target_list[i:i+1] + target_list[i+3:]
    return os.path.join(*rel_list)

