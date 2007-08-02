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
import filecmp
import glob
import SCons.Options
import SCons.Script
import SCons.Util
import SCons.Defaults
	

this_dir		= os.path.split(__file__)[0]
cdevkit_dir		= os.path.normpath(os.path.join(this_dir, '..'))
toolpath		= os.path.join(cdevkit_dir, 'Tools')

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
	
	platform_toolset = vars['platform'] + (vars['is_emulation'] and '_emulation' or '_embedded')

	env = SCons.Environment.Environment(options  = opts,
							tools = ['default', platform_toolset, 'cxxtest', 'runtest', 'checkheader', 'oggenc'], 
							toolpath = [toolpath],
					 )
	env.Append(CPPDEFINES = ['LF2530BROWN'])
	#FIXME/tp: Keep this 'variant' field up to date! Remove at final hardware.
	
	cdevkit_incpath	= os.path.join(cdevkit_dir, 'Include')
	ogl_incpath		= os.path.join(cdevkit_incpath, 'OpenGL', target_subdir)
	cpppaths  = [cdevkit_incpath, ogl_incpath]
	
	if vars['is_emulation']:
		cpppaths += [os.path.join(ogl_incpath, 'LinuxPC'), '/usr/X11R6/include']
	else:
		cpppaths += [os.path.join(cdevkit_incpath, 'ThirdParty', 'ustring')]
	
	
	libpaths = [os.path.join(cdevkit_dir, 'Libs', target_subdir, 'MPI'),
				os.path.join(cdevkit_dir, 'Libs', target_subdir, 'OpenGL')]
	if vars['is_emulation']:
		libpaths += [os.path.join(cdevkit_dir, 'Libs', target_subdir, 'ThirdParty')]

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
	rootfs_data = os.path.join(vars['rootfs'], 'Cart1')
	if not os.path.exists(rootfs_data):
		os.mkdir(rootfs_data)
	rootfs_data = os.path.join(rootfs_data, 'rsrc')
	if not os.path.exists(rootfs_data):
		os.mkdir(rootfs_data)
	
	def callback(arg, directory, files):
		base = os.path.basename(directory)
		if base == '.svn':
			del files[:]
		else:
			for file in files:
				full = os.path.join(directory, file)
				if os.path.isfile(full):
					subdir = os.path.dirname(full[root_len:])
					target = penv.Install(os.path.join(rootfs_data, subdir), full)
					if not os.path.exists(target[0].abspath) or not filecmp.cmp(full, target[0].abspath):
						penv.AlwaysBuild(target)
					
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
		#FIXME/tp: pthread should go away
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



#=========================================================================
# *** RESOURCE PACKING ***
#=========================================================================
#-------------------------------------------------------------------------
# Hooks to encoders
#-------------------------------------------------------------------------
sEnv		= SCons.Environment.Environment(tools = ['oggenc', 'rawenc'], 
							toolpath = [toolpath])

#-------------------------------------------------------------------------
# Cache state to avoid duplicate packing and report multiple definitions
#-------------------------------------------------------------------------
sSourceToDestMap = {}
sResourceURIMap  = {}

#-------------------------------------------------------------------------
# Functions to enumerate packed filenames
#-------------------------------------------------------------------------
valid_chars		= range(48,58) + range (65,91)
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
	types = { 'mid' 	: 1024 *  1 + 1,	#Audio Group
			  'midi' 	: 1024 *  1 + 1,
			  'S'		: 1024 *  1 + 1,		# for ACME
			  'aogg'	: 1024 *  1 + 2,
			  'wav'		: 1024 *  1 + 3,
			  'R'		: 1024 *  1 + 3,		# for ACME
			  'raw'		: 1024 *  1 + 3,		# for ACME
			  'txt'		: 1024 *  4 + 1,	# Common Group
			  'bin'		: 1024 *  4 + 2,
			  'json'	: 1024 *  4 + 3,
			  'xml'		: 1024 *  4 + 4,
			  'bmp'		: 1024 *  5 + 1,	# Display Group
			  'gif'		: 1024 *  5 + 2,
			  'jpg'		: 1024 *  5 + 3,
			  'jpeg'	: 1024 *  5 + 3,
			  'png'		: 1024 *  5 + 4,
			  'font'	: 1024 *  7 + 1,	# Font Group
			  'ttf'		: 1024 *  7 + 1,
			  'ogg'		: 1024 * 14 + 1,	# Video Group
			  }
	return types
		
#-------------------------------------------------------------------------
# Track duplicate URIs
#-------------------------------------------------------------------------
def AddURILocation(uri, file, line):
	key = uri.upper()
	locations = []
	if key in sResourceURIMap:
		locations = sResourceURIMap[key]
	locations += [[file, line]]
	sResourceURIMap[key] = locations

#-------------------------------------------------------------------------
# Transform or copy source file to packed output file
#-------------------------------------------------------------------------
def PackFile(srcfile, pack_root, type, compression, rate):
	# 1) If we have already processed this source file, return the
	#    already processed destination file
	# 2) Create output file path
	# 3) Add the new output file to the Source->Dest map, return it
	# 4) Invoke OGG encoder
	# 5) Invoke utility that converts WAV to RAW PCM w/Brio header
	# A) No encoder needed, just copy the file
	#
	if sSourceToDestMap.has_key(srcfile):							#1
		return sEnv.File(sSourceToDestMap[srcfile])
		
	outname = GenerateNextResourceFileName()						#2
	outfile = os.path.join(pack_root, outname)
	sSourceToDestMap[srcfile] = outfile								#3
	
	if type == 1026 and rate != '':									#3
		enc = sEnv.OggEnc(outfile, srcfile, OGGENC_RATE=rate, OGGENC_COMPRESSION=compression)
		
	elif type == 1027:												#5
		enc = sEnv.RawEnc(outfile, srcfile)
		
	else:
		enc = sEnv.Command(outfile, srcfile, [SCons.Defaults.Copy('$TARGET', '$SOURCE')])	#A
		
	return enc[0]


#-------------------------------------------------------------------------
# Pack contents of an individual ACME CSV file
#-------------------------------------------------------------------------
def ProcessAcme(pkg, types, pack_root, data_root, enumpkg):
	# 1) Setup field mappings (see note below)
	# 2) Read in the mapping of activity code to Base URI path
	# 3) Open the input package
	# 4) The first line contains field descriptions, so skip it.
	#    Subsequent lines define resources.
	# 5) Convert the ACME type info value into the Brio audio type.
	# 6) Pack the file
	# 7) Record the URI & location to later report any duplicates
	# 8) Add a line describing the resource to the "resources" list.
	# 9) Sort the resources list by URI (case-insenstive) and write out
	#    the output package file (see the following URL for an explanation
	#    of this sorting method: http://wiki.python.org/moin/HowTo/Sorting)
	# NOTE: The 'fld' param tells the ProcessPackage() function which
	# fields are of interest in the input file, where:
	#   0: activity code or base URL path
	#   1: "handle" or URL node value
	#   2: data type string 
	#   3: file name
	#	
	fld = [7, 17, 15, 9]	# activity, handle, type, file				#1
	dict = MapAcmeActivitiesToURIs(data_root)							#2
	resources = []
	pack_root_len = len(pack_root) + 1
	
	reader = csv.reader(open(pkg, "rb"))								#3
	pkguri = dict[os.path.basename(pkg)]
	pkgfile = GenerateNextPackageFileName()
	packages += [[pkguri, pkgfile, 1, 1]]
	line = 0
	for row in reader:
		line += 1
		if line == 0:													#4
			# TODO: map 'fld' to 'SHAPE DESCRIPTION', 'FILE', 'AUDIOTYPE', & 'AUDIO HANDLE'
			dummy = line	
		elif row[fld[3]] != '':
			compression = rate = ''										#5
			if types.has_key(row[fld[2]]):
				type = types[row[fld[2]]]
			else:
				type = 1025
			extension = type == 1024 and '.mid' or '.wav'
			srcfile = os.path.join(data_root, row[fld[3]]+extension)
			srcsize = os.path.getsize(srcfile)
			outfile = PackFile(srcfile, pack_root, type, compression, rate)	#6
			outsize = outfile.getsize()
			uri = dict[row[fld[0]]] + row[fld[1]]
			AddURILocation(uri, pkg, line)								#7
			out = outfile.abspath[pack_root_len:]
			resources += [[uri, type, out, outsize, srcsize, version]]	#8

	deco = [ (res[0].upper(), i, res) for i, res in enumerate(resources) ]
	deco.sort()															#9
	resources = [ res for _, _, res in deco ]
	writer = csv.writer(open(os.path.join(pack_root, pkgfile), "w"))
	writer.writerows(resources)


#-------------------------------------------------------------------------
# Pack contents of an individual package
#-------------------------------------------------------------------------
def ProcessPackage(pkg, types, pack_root, data_root, packages):
	# Package input format is "baseURIPath, URINode, srcFile, (optional)packtype"
	#
	# 1) Open the input package
	# 2) Skip comment lines
	# 3) The first non-comment line of the input package contains default
	#    values so parse and store them.  This info can be used to build
	#    the package URI, so add the URI -> path mapping for the package
	#    to the EnumPkgs file.
	# 4) Subsequent lines contain resources
	# 5) Get the type info either from the type field or the file extension.
	#    Field 4 of the CSV file is a combined type/compression/rate field, 
	#    joined together by underscores.  An example would be "AOGG_3_16000"
	#    for and audio OGG file compressed at quality 3 at a sample rate of
	#    16000.  Parse that into its constituent parts.
	# 6) Get the base URL from the line and use the default if not present
	# 7) Pack the resource file
	# 8) Record the URI & location to later report any duplicates
	# 9) Add a line describing the resource to the "resources" list.
	# 10) Sort the resources list by URI (case-insenstive) and write out
	#    the output package file (see the following URL for an explanation
	#    of this sorting method: http://wiki.python.org/moin/HowTo/Sorting)
	#
	reader = csv.reader(open(pkg, "rb"))							#1
	pkgfile = GenerateNextPackageFileName()
	line = 0
	defaultBase = ''
	defaultVersion = '1'
	isDefaultRow = True
	resources = []
	pack_root_len = len(pack_root) + 1
	
	for row in reader:
		line += 1
		if len(row) == 0 or (row[0] and row[0][0] == '#'):			#2
			continue
			
		if isDefaultRow:											#3
			defaultBase = row[0].strip()
			if len(row) >= 5:
				defaultVersion = row[4].strip()
			pkguri = defaultBase + os.path.splitext(os.path.basename(pkg))[0]
			packages += [[pkguri, pkgfile, 1, 1]]
			isDefaultRow = False
			continue												#4
			
		compression = rate = ''
		if len(row) >= 4 and row[3].strip() != '':					#5
			temp = row[3].strip()
			idx1 = temp.index('_')
			idx2 = temp.index('_', idx1+1)
			type = temp[:idx1]
			compression = temp[idx1+1:idx2]
			rate = temp[idx2+1:]
			type = types[type.lower()]
		else:
			ext = os.path.splitext(row[2].strip())[1]
			type = types[ext[1:].lower()]
			
		base = row[0].strip() and row[0].strip() or defaultBase		#6
		version = len(row) >= 5 and row[5].strip() and row[5].strip() or defaultVersion

		srcfile = os.path.join(data_root, row[2].strip())
		srcsize = os.path.getsize(srcfile)
		outfile = PackFile(srcfile, pack_root, type, compression, rate)	#7
		outsize = outfile.getsize()
		uri = base + row[1].strip()
		AddURILocation(uri, pkg, line)								#8
		out = outfile.abspath[pack_root_len:]
		resources += [[uri, type, out, outsize, srcsize, version]]	#9

	deco = [ (res[0].upper(), i, res) for i, res in enumerate(resources) ]
	deco.sort()														#10
	resources = [ res for _, _, res in deco ]
	writer = csv.writer(open(os.path.join(pack_root, pkgfile), "w"))
	writer.writerows(resources)


#-------------------------------------------------------------------------
# Deploy the apprsrc assets for embedded builds
#-------------------------------------------------------------------------
def PackAllResources(penv):
	# 0) Do nothing for "clean" operations
	# 1) Setup source and destination folders
	# 2) Find all "ACME" input package definition files and process them
	# 3) Find all standard package input files and process them
	# 4) Sort the packages and report any duplicate package URIs
	# 5) Report any duplicate resource URIs
	# 6) Write them to the rsrc/EnumPkgs file
	#
	if penv.GetOption('clean'):										#0
		return
	
	sSourceToDestMap = {}
	sResourceURISet = {}
	packages = []
	data_root = penv.Dir('#apprsrc').abspath						#1
	build_root = penv.Dir('#Build').abspath
	pack_root = penv.Dir('#Build/rsrc').abspath
	if not os.path.exists(build_root):
		os.mkdir(build_root)
	if not os.path.exists(pack_root):
		os.mkdir(pack_root)
	types = SetupTypeConversionMap()
	penv.Default(pack_root)
	
	pkgs = glob.glob(os.path.join(data_root, '*.acme'))				#2
	for pkg in pkgs:
		ProcessAcme(pkg, types, pack_root, data_root, packages)
		
	pkgs = glob.glob(os.path.join(data_root, '*.pkg'))				#3
	for pkg in pkgs:
		ProcessPackage(pkg, types, pack_root, data_root, packages)
	
	enumpkg_path = os.path.join(pack_root, "EnumPkgs")				#4
	deco = [ (pkg[0].upper(), i, pkg) for i, pkg in enumerate(packages) ]
	deco.sort()
	n = len(deco)
	last, _, _ = deco[0]
	i = 1
	while i < n:
		next, _, _ = deco[i]
		if next == last:
			_, _, duplicate = deco[i]
			print 'ERROR: Duplicate package URIs used:', duplicate[0]
			raise 'ERROR: Duplicate package URIs used!'
		last = next
		i += 1

	sawError = False												#5
	for k, v in sResourceURIMap.iteritems():
		if len(v) > 1:
			print 'ERROR: Duplicate Resource URIs found on the following lines:'
			for vals in v:
				print '\t', vals[0], 'line', vals[1]
			sawError = True
	if sawError:
		raise 'Duplicate URIs found, aborting!'
	
	packages = [ pkg for _, _, pkg in deco ]						#6
	writer = csv.writer(file(enumpkg_path, "w"))
	writer.writerows(packages)

#TODO: Deal with greater than 36^2 resources (hierarchical folders).  Currently an exception is raised.
#TODO: Make packing a 'Tool' so it only runs when sources are touched (a biggie!)
#TODO: Define method for app-specific resource type values that don't overlap

#-----------------------------------------------------------------------------
# Export the all of the functions symbols
#-----------------------------------------------------------------------------
__all__ = ["SetupOptions", "RetrieveOptions", "CreateEnvironment", "MakeMyApp", "PackAllResources"] 

