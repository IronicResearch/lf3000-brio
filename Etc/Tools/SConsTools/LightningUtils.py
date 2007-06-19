#-----------------------------------------------------------------------------
# LightningUtils.py
#
# Common utilities for use by top level SConstruct files
#-----------------------------------------------------------------------------
import os
import sys
import SCons
import csv
import shutil
import glob
import SCons.Options
import SCons.Script
import SCons.Util
	

#-----------------------------------------------------------------------------
# Setup common command line options for a Lightning project
#-----------------------------------------------------------------------------
def SetupOptions():
	opts = SCons.Options.Options('Lightning.py')
	opts.Add(SCons.Options.BoolOption('runtests', 'Default is to run unit tests, set "runtests=f" to disable', 1))
	opts.Add(SCons.Options.EnumOption('type', '"publish" creates an RC\n    "checkheaders" uncovers inclusion dependencies\n   ',
					'embedded', 
					allowed_values=('checkheaders', 'embedded', 'emulation', 'publish')))
	return opts


#-----------------------------------------------------------------------------
# Retrieve common command line options for a Lightning project
#-----------------------------------------------------------------------------
def RetrieveOptions(args, root_dir):

	is_runtests		= args.get('runtests', 1)
	type			= args.get('type', 'embedded')
	platform		= 'Lightning'
	
	is_emulation 			= type == 'emulation' or type == 'checkheaders'
	target_subdir			= platform + (is_emulation and '_emulation' or '')
	intermediate_build_dir	= os.path.join(root_dir, 'Temp', target_subdir)
	
	#FIXME/tp: Is this the best mechanism for allowing alternate nfsroot locations?
	rootfs = os.getenv('ROOTFS_PATH')
	if rootfs == None:
		rootfs = os.path.normpath(os.path.join(root_dir, '..', '..', 'nfsroot'))

	#FIXME/tp: add mods for type == 'publish' here
	if is_emulation:
		bin_deploy_dir	= os.path.join(root_dir, 'Build', target_subdir)
	else:
		bin_deploy_dir	= os.path.join(rootfs, 'usr', 'local', 'bin')
		SCons.Script.Default(bin_deploy_dir)
		
	if type == 'publish':
		print '*** type=publish is a placeholder (a noop at this time).'
	
	vars = {'type'					: type,
			'is_emulation'			: is_emulation,
			'is_runtests'			: is_runtests,
			'platform'				: platform,
			'target_subdir'			: target_subdir,
			'intermediate_build_dir': intermediate_build_dir,
			'bin_deploy_dir'		: bin_deploy_dir,
			'rootfs'				: rootfs,
	}
	
	return vars


#-----------------------------------------------------------------------------
# Setup the construction environment object to support Lightning defaults
# and CxxTest unit testing.
#-----------------------------------------------------------------------------
def CreateEnvironment(opts, vars):

	target_subdir	= vars['target_subdir']
	this_dir		= os.path.split(__file__)[0]
	cdevkit_dir		= os.path.normpath(os.path.join(this_dir, '..'))
	toolpath		= os.path.join(cdevkit_dir, 'Tools')
	
	platform_toolset = vars['platform'] + (vars['is_emulation'] and '_emulation' or '_embedded')

	env = SCons.Environment.Environment(options  = opts,
							tools = ['default', platform_toolset, 'cxxtest', 'runtest', 'checkheader'], 
							toolpath = [toolpath],
					 )
	env.Append(CPPDEFINES = ['LF2530BLUE'])
	#FIXME/tp: Keep this 'variant' field up to date!
	
	cdevkit_incpath	= os.path.join(cdevkit_dir, 'Include')
	ogl_incpath		= os.path.join(cdevkit_incpath, 'OpenGL', target_subdir)
	cpppaths  = [cdevkit_incpath, ogl_incpath]
	
	if vars['is_emulation']:
		cpppaths += [os.path.join(ogl_incpath, 'LinuxPC'), '/usr/X11R6/include']
	else:
		cpppaths += [os.path.join(cdevkit_incpath, 'ThirdParty', 'ustring')]
	
	
	libpaths = [os.path.join(cdevkit_dir, 'Libs', target_subdir, 'MPI'),
				os.path.join(cdevkit_dir, 'Libs', target_subdir, 'OpenGL')]

	env.Append(CPPDEFINES = ['LEAPFROG_CDEVKIT_ROOT=\\"'+cdevkit_dir+'\\"', 'OGLESLITE'])
	env.Append(CPPPATH = cpppaths)
	env.Append(LIBPATH = libpaths)
	env.Append(RPATH = libpaths + [os.path.join(cdevkit_dir, 'Libs', target_subdir, 'PrivMPI')])
	
	return env

#-------------------------------------------------------------------------
# Deploy the apprsrc assets for embedded builds
#-------------------------------------------------------------------------
def CopyResources(penv, vars):
	data_root = penv.Dir('#Build/rsrc').abspath
	root_len = len(data_root) + 1
	rootfs_data = os.path.join(vars['rootfs'], 'Cart1', 'rsrc')
	
	def callback(arg, directory, files):
		base = os.path.basename(directory)
		if base == '.svn':
			del files[:]
		else:
			for file in files:
				full = os.path.join(directory, file)
				if os.path.isfile(full):
					subdir = os.path.dirname(full[root_len:])
					penv.Install(os.path.join(rootfs_data, subdir), full)
					
	os.path.walk(data_root, callback, None)
	penv.Default(rootfs_data)

#-----------------------------------------------------------------------------
# Build a Module (either embedded or emulation target)
#
# Prepend any platform-specific libraries to the library list
# Install the library to the correct installation folder
#-----------------------------------------------------------------------------
def MakeMyApp(penv, ptarget, psources, plibs, vars):

	if len(psources) == 0:
		print '!!! Exiting, no source files specified in build'
		Exit(-2)
	
	# Set the include search paths for this app's folders
	bldenv = penv.Copy()
	source_dir = os.path.dirname(psources[0])
	bldenv.Append(CPPPATH = [source_dir, os.path.join(source_dir, 'Include')])

	# Add additional system libraries for the platform
	if vars['is_emulation']:
		platformlibs = ['gles_cl', 'glibmm-2.4', 'glib-2.0', 'Emulation']
	else:
		platformlibs = ['ogl', 'dl', 'pthread', 'ustring', 'iconv', 'intl', 'sigc-2.0']
		#FIXME/tp: pthread should go away (curr needed by BrioCube?)
		CopyResources(penv, vars)
	
	# Set up targets
	exe		= os.path.join(vars['intermediate_build_dir'], ptarget)
	mapfile = os.path.join(vars['intermediate_build_dir'], ptarget + '.map')
	bldenv.Append(LINKFLAGS = ' -Wl,-Map=' + mapfile)
	
	# Build and deploy the app (build it in two steps, so that the objs
	# can later be fed into a unit test
	bldenv.BuildDir(vars['intermediate_build_dir'], '.', 0)
	srcs =  map(lambda x: os.path.join(vars['intermediate_build_dir'], x), psources)
	
	objs  = bldenv.Object(srcs)
	myapp = bldenv.Program(exe, objs, LIBS = plibs + platformlibs)
	targets = vars['is_emulation'] and [myapp, mapfile] or [myapp]
	penv.Install(vars['bin_deploy_dir'], targets)
	
	return objs

#-------------------------------------------------------------------------
# Functions to enumerate packed filenames
#-------------------------------------------------------------------------
valid_chars = range(48,58) + range (64,91)
valid_chars_count = len(valid_chars)
sPkgNameIndex	= [0, -1]
sRsrcNameIndex	= [0, -1]

#-------------------------------------------------------------------------
def GenerateNextName(idx):
	name = ''
	if idx[1] >= 0:
		name += chr(valid_chars[idx[1]])
	name += chr(valid_chars[idx[0]])
	idx[0] = idx[0] + 1
	if idx[0] == valid_chars_count:
		idx[0] = 0
		idx[1] = idx[1] + 1
	if idx[1] == valid_chars_count:
		raise 'Exhausted the Resource FileName enumeration space!'
	return name 

#-------------------------------------------------------------------------
def GenerateNextResourceFileName():
	# Generate sequential names:
	#    0, 1, ... 9, A, B, ..., Z, 00, 01, ..., 0Z, 10, 11, ..., ZZ
	#
	global sRsrcNameIndex
	name = GenerateNextName(sRsrcNameIndex)
	return name

#-------------------------------------------------------------------------
def GenerateNextPackageFileName():
	# Generate sequential names:
	#    _0, _1, ... _9, _A, _B, ..., _Z, _00, _01, ..., _0Z, _10, _11, ..., _ZZ
	#
	global sPkgNameIndex
	name = GenerateNextName(sPkgNameIndex)
	return '_' + name 

#-------------------------------------------------------------------------
# Read 'ActivityMapping' file to create map of Activities to base URI paths
#-------------------------------------------------------------------------
def MapAcmeActivitiesToURIs(data_root):
	mappings =  os.path.join(data_root, 'ActivityMapping')
	reader = csv.reader(open(mappings, "rb"))
	dict = {}
	for row in reader:
		dict[row[0].strip()] = row[1].strip() + '/'
	return dict

#-------------------------------------------------------------------------
# Set up a map of type names and file extensions to numeric type values
#-------------------------------------------------------------------------
def SetupTypeConversionMap():
	types = { 'mid' 	: 1025,
			  'midi' 	: 1025,
			  'S'		: 1025,				# for ACME
			  'ogg'		: 1026,
			  'wav'		: 1026,
			  'R'		: 1026,				# for ACME
			  'font'	: 1024 * 7 + 1,
			  'ttf'		: 1024 * 7 + 1,
			  'txt'		: 1024 * 4 + 1,
			  }
	return types

#-------------------------------------------------------------------------
# Pack contents of an individual ACME CSV file
#-------------------------------------------------------------------------
def ProcessAcme(pkg, types, pack_root, data_root, enumpkg):
	# 1) Setup field mappings (see note below)
	# 2) Read in the mapping of activity code to Base URI path
	#    (for converting acme CSV files)
	# NOTE: The 'fld' param tells the ProcessPackage() function which
	# fields are of interest in the input file, where:
	#   0: activity code or base URL path
	#   1: "handle" or URL node value
	#   2: data type string 
	#   3: file name
	#
	fld = [7, 17, 15, 9]	# activity, handle, type, file			#1
	dict = MapAcmeActivitiesToURIs(data_root)						#2
	
	reader = csv.reader(open(pkg, "rb"))
	pkguri = dict[os.path.basename(pkg)]
	pkgfile = GenerateNextPackageFileName()
	enumpkg.writerow([pkguri, pkgfile])
	writer = csv.writer(open(os.path.join(pack_root, pkgfile), "w"))
	linenum = 0
	for row in reader:
		if linenum == 0:
			# TODO: map 'fld' to 'SHAPE DESCRIPTION', 'FILE', 'AUDIOTYPE', & 'AUDIO HANDLE'
			linenum += 1
		elif row[fld[3]] != '':
			if types.has_key(row[fld[2]]):
				type = types[row[fld[2]]]
			else:
				type = 1025
			extension = type == 1024 and '.mid' or '.wav'
			outfile = GenerateNextResourceFileName()
			srcfile = os.path.join(data_root, row[fld[3]]+extension)
			srcsize = os.path.getsize(srcfile)
			writer.writerow([dict[row[fld[0]]] + row[fld[1]], type, outfile, srcsize, srcsize, 1])
			shutil.copyfile(srcfile, os.path.join(pack_root, outfile))

#-------------------------------------------------------------------------
# Pack contents of an individual package
#-------------------------------------------------------------------------
def ProcessPackage(pkg, types, pack_root, data_root, enumpkg):
	# Package input format is "baseURIPath, URINode, srcFile, (optional)packtype"
	#
	# 1) Open input and output packages
	# 2) Skip comment lines
	# 3) The first non-comment line of the input package contains default
	#    values so parse and store them.  This info can be used to build
	#    the package URI, so
	#    add the URI -> path mapping for the package to the EnumPkgs file
	# 4) Subsequent lines contain resources
	# 5) Get the type info either from the type field or the file extension
	# 6) Get the base URL from the line and use the default if not present
	# 7) Output data describing the resource in the package file
	# 8) Either copy or transform & copy the file from the source
	#    folder to the destination folder
	# NOTE: The sf2brio EXE is a temporary "packer" that puts a header
	#		in front of a WAV file.  It will be replaced by the OggVorbis
	#		encoder.
	#
	print pkg
	reader = csv.reader(open(pkg, "rb"))							#1
	pkgfile = GenerateNextPackageFileName()
	writer = csv.writer(open(os.path.join(pack_root, pkgfile), "w"))
	linenum = 0
	defaultBase = ''
	defaultVersion = '1'
	
	for row in reader:
		if len(row) == 0 or (row[0] and row[0][0] == '#'):			#2
			continue
		if linenum == 0:											#3
			defaultBase = row[0].strip()
			if len(row) >= 5:
				defaultVersion = row[4].strip()
			pkguri = defaultBase + os.path.splitext(os.path.basename(pkg))[0]
			enumpkg.writerow([pkguri, pkgfile, 1, 1])
			linenum += 1
			continue												#4
			
		if len(row) >= 4 and row[3].strip() != '':					#5
			type = types[row[3].strip()]
		else:
			ext = os.path.splitext(row[2].strip())[1]
			type = types[ext[1:].lower()]
			
		base = row[0].strip() and row[0].strip() or defaultBase		#6
		version = len(row) >= 5 and row[5].strip() and row[5].strip() or defaultVersion

		outfile = GenerateNextResourceFileName()					#7
		outpath = os.path.join(pack_root, outfile)
		srcfile = os.path.join(data_root, row[2].strip())
		srcsize = os.path.getsize(srcfile)
		writer.writerow([base + row[1].strip(), type, outfile, srcsize, srcsize, version])
		
		this_dir		= os.path.split(__file__)[0]
		if type == 1026:											#8
			os.system(os.path.join(this_dir, 'sf2brio') + ' ' + srcfile + ' ' + outpath)
		else:
			shutil.copyfile(srcfile, outpath)


#-------------------------------------------------------------------------
# Deploy the apprsrc assets for embedded builds
#-------------------------------------------------------------------------
def PackAllResources(penv):
	# 1) Setup source and destination folders
	# 2) Open the rsrc/EnumPkgs file for writing
	# 3) Find all "ACME" input package defintion files and process them
	# 4) Find all standard packag input files and process them
	#
	data_root = penv.Dir('#apprsrc').abspath						#1
	build_root = penv.Dir('#Build').abspath
	pack_root = penv.Dir('#Build/rsrc').abspath
	if not os.path.exists(build_root):
		os.mkdir(build_rootbuild_root)
	if not os.path.exists(pack_root):
		os.mkdir(pack_root)
	types = SetupTypeConversionMap()
	
	enumpkg_path = os.path.join(pack_root, "EnumPkgs")				#2
	enumpkg = csv.writer(file(enumpkg_path, "w"))
		
	packages =  glob.glob(os.path.join(data_root, '*.acme'))		#3
	for pkg in packages:
		ProcessAcme(pkg, types, pack_root, data_root, enumpkg)
		
	packages =  glob.glob(os.path.join(data_root, '*.pkg'))			#4
	for pkg in packages:
		ProcessPackage(pkg, types, pack_root, data_root, enumpkg)
	

#TODO: Deal with greater than 36^2 resources (hierarchical folders)
#TODO: Improve error handling and reporting!
#TODO: Make packing a 'Tool' so it only runs when sources are touched

#-----------------------------------------------------------------------------
# Export the all of the functions symbols
#-----------------------------------------------------------------------------
__all__ = ["SetupOptions", "RetrieveOptions", "CreateEnvironment", "MakeMyApp", "PackAllResources"] 

