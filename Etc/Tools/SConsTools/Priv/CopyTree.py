import os
import fnmatch
import unittest

"""
relpath and helpers
http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/208993
"""
def pathsplit(pth):
	""" This version, in contrast to the original version, permits trailing
	slashes in the pathname (in the event that it is a directory).
	It also uses no recursion
	"""
	return os.path.normpath(pth).split(os.path.sep)

#~ def pathsplit(p, rest=[]):
	#~ (h,t) = os.path.split(p)
	#~ if len(h) < 1: return [t]+rest
	#~ if len(t) < 1: return [h]+rest
	#~ return pathsplit(h,[t]+rest)

def commonpath(l1, l2, common=[]):
	if len(l1) < 1: return (common, l1, l2)
	if len(l2) < 1: return (common, l1, l2)
	if l1[0] != l2[0]: return (common, l1, l2)
	return commonpath(l1[1:], l2[1:], common+[l1[0]])

def relpath(p1, p2):
	"""Return the relative path from p1 to p2.
	Returns a value p3 such that os.path.join(p1, p3) == p2
	"""
	(common,l1,l2) = commonpath(pathsplit(p1), pathsplit(p2))
	p = []
	if len(l1) > 0:
		p = [os.path.pardir] * len(l1)
	p = p + l2
	if p:
		return os.path.join( *p )
	else:
		return ''

def CopyTree(env, sourceDir, targetDir, masks=('*',), exclude=()):
	"""Recursively mark all assets in sourceDir to be copied to targetDir.
	"""
	excludeList = ['.svn']  # always ignore .svn, plus anything else passed
	excludeList.extend(exclude)
	dataFiles = []
	for dirpath, dirnames, filenames in os.walk(sourceDir):
		#print dirpath, dirnames, filenames
		# prune the directories of excluded directory names
		for dirname in dirnames:
			if dirname in excludeList:
				# ignore
				dirnames.remove(dirname)
		# add the files for copying
		for mask in masks:
			matchnames = tuple(os.path.join(dirpath, filename) for filename in fnmatch.filter(filenames, mask))
			for matchname in matchnames:
				# compute the name of the target directory for this asset
				# strip the parent folder sourceDir from the name of the asset
				targetDest = os.path.join(targetDir, relpath(sourceDir, matchname))
				targetDest = os.path.split(targetDest)[0]
				#print "install source:", matchname
				#print "install dest:  ", targetDest
				dataFiles.append(env.Install(targetDest, matchname))
	return dataFiles


class TestCopyTree(unittest.TestCase):
	def setUp(self):
		pass
		
	def test_relpath(self):
		self.assertEqual(relpath(os.path.join('a','b'), os.path.join('a','b','c')), 'c')
		self.assertEqual(relpath(os.path.join('a','b','c'), os.path.join('a','b','c')), '')
		self.assertEqual(relpath(os.path.join('a'), os.path.join('a','b','c')), os.path.join('b','c'))

if __name__=='__main__':
	#unittest.main()
	copyTree('apprsrc', '/home/lfu/emuroot')
	
import os
import fnmatch
import unittest

"""
relpath and helpers
http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/208993
"""
def pathsplit(pth):
	""" This version, in contrast to the original version, permits trailing
	slashes in the pathname (in the event that it is a directory).
	It also uses no recursion
	"""
	return os.path.normpath(pth).split(os.path.sep)

#~ def pathsplit(p, rest=[]):
	#~ (h,t) = os.path.split(p)
	#~ if len(h) < 1: return [t]+rest
	#~ if len(t) < 1: return [h]+rest
	#~ return pathsplit(h,[t]+rest)

def commonpath(l1, l2, common=[]):
	if len(l1) < 1: return (common, l1, l2)
	if len(l2) < 1: return (common, l1, l2)
	if l1[0] != l2[0]: return (common, l1, l2)
	return commonpath(l1[1:], l2[1:], common+[l1[0]])

def relpath(p1, p2):
	"""Return the relative path from p1 to p2.
	Returns a value p3 such that os.path.join(p1, p3) == p2
	"""
	(common,l1,l2) = commonpath(pathsplit(p1), pathsplit(p2))
	p = []
	if len(l1) > 0:
		p = [os.path.pardir] * len(l1)
	p = p + l2
	if p:
		return os.path.join( *p )
	else:
		return ''

def CopyTree(env, sourceDir, targetDir, masks=('*',), exclude=()):
	"""Recursively mark all assets in sourceDir to be copied to targetDir.
	"""
	excludeList = ['.svn']  # always ignore .svn, plus anything else passed
	excludeList.extend(exclude)
	dataFiles = []
	for dirpath, dirnames, filenames in os.walk(sourceDir):
		#print dirpath, dirnames, filenames
		# prune the directories of excluded directory names
		for dirname in dirnames:
			if dirname in excludeList:
				# ignore
				dirnames.remove(dirname)
		# add the files for copying
		for mask in masks:
			matchnames = tuple(os.path.join(dirpath, filename) for filename in fnmatch.filter(filenames, mask))
			for matchname in matchnames:
				# compute the name of the target directory for this asset
				# strip the parent folder sourceDir from the name of the asset
				targetDest = os.path.join(targetDir, relpath(sourceDir, matchname))
				targetDest = os.path.split(targetDest)[0]
				#print "install source:", matchname
				#print "install dest:  ", targetDest
				dataFiles.append(env.Install(targetDest, matchname))
	return dataFiles


class TestCopyTree(unittest.TestCase):
	def setUp(self):
		pass
		
	def test_relpath(self):
		self.assertEqual(relpath(os.path.join('a','b'), os.path.join('a','b','c')), 'c')
		self.assertEqual(relpath(os.path.join('a','b','c'), os.path.join('a','b','c')), '')
		self.assertEqual(relpath(os.path.join('a'), os.path.join('a','b','c')), os.path.join('b','c'))

if __name__=='__main__':
	#unittest.main()
	copyTree('apprsrc', '/home/lfu/emuroot')
	