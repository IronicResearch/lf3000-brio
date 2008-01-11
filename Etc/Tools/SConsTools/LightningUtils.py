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
import time
import SCons.Options
import SCons.Script
import SCons.Util
import SCons.Defaults
	

this_dir		= os.path.split(__file__)[0]
cdevkit_dir		= os.path.normpath(os.path.join(this_dir, '..'))
toolpath		= os.path.join(cdevkit_dir, 'Tools')


#-----------------------------------------------------------------------------
# Get date and time for logging.
#-----------------------------------------------------------------------------
class datetime(object):
    def __init__(self, *argv):
        self.t = time.struct_time(argv+(0,)*(9-len(argv)))    # append to length 9 
    def __getattr__(self, name):
        try:
            i = ['year', 'month', 'day', 'hour', 'minute', 'second', 'weekday'].index(name)
            return self.t[i]
        except:
            return getattr(self.t, name)
    def __len__(self): return len(self.t)
    def __getitem__(self, key): return self.t[key]
    def __repr__(self): return repr(self.t)
    def now(self=None):
        return datetime(*time.localtime())
    now = staticmethod(now)
    def strftime(self, fmt="%Y-%m-%d %H:%M:%S"):
        return time.strftime(fmt, self.t)

#-----------------------------------------------------------------------------------------
# Easily extract data from microsoft excel files using this wrapper class for xlrd 
# (http://www.lexicon.net/sjmachin/xlrd.htm). The class allows you to create a generator 
# which returns excel data one row at a time as either a list or dictionary. I found this 
# very handy for easily pulling in a variety of excel files without having to deal with 
# COM calls or even needing to have windows. 
#-----------------------------------------------------------------------------------------
class readexcel(object):
    """ Simple OS Independent Class for Extracting Data from Excel Files 
        the using xlrd module found at http://www.lexicon.net/sjmachin/xlrd.htm
        
        Versions of Excel supported: 2004, 2002, XP, 2000, 97, 95, 5, 4, 3
        xlrd version tested: 0.5.2
        
        Data is extracted by creating a iterator object which can be used to 
        return data one row at a time. The default extraction method assumes 
        that the worksheet is in tabular format with the first nonblank row
        containing variable names and all subsequent rows containing values.
        This method returns a dictionary which uses the variables names as keys
        for each piece of data in the row.  Data can also be extracted with 
        each row represented by a list.
        
        Extracted data is represented fairly logically. By default dates are
        returned as strings in "yyyy/mm/dd" format or "yyyy/mm/dd hh:mm:ss",
        as appropriate.  However, dates can be return as a tuple containing
        (Year, Month, Day, Hour, Min, Second) which is appropriate for usage
        with mxDateTime or DateTime.  Numbers are returned as either INT or 
        FLOAT, whichever is needed to support the data.  Text, booleans, and
        error codes are also returned as appropriate representations.
        
        Quick Example:
        xl = readexcel('testdata.xls')
        sheetnames = xl.worksheets()
        for sheet in sheetnames:
            print sheet
            for row in xl.getiter(sheet):
                # Do Something here
        """ 
    def __init__(self, filename):
        """ Returns a readexcel object of the specified filename - this may
        take a little while because the file must be parsed into memory """
        import xlrd
        import os.path
        if not os.path.isfile(filename):
            raise NameError, "%s is not a valid filename" % filename
        self.__filename__ = filename
        self.__book__ = xlrd.open_workbook(filename)
        self.__sheets__ = {}
        self.__sheetnames__ = []
        for i in self.__book__.sheet_names():
            uniquevars = []
            firstrow = 0
            sheet = self.__book__.sheet_by_name(i)
            for row in range(sheet.nrows):
                types,values = sheet.row_types(row),sheet.row_values(row)
                nonblank = False
                for j in values:
                    if j != '':
                        nonblank=True
                        break
                if nonblank:
                    # Generate a listing of Unique Variable Names for Use as
                    # Dictionary Keys In Extraction. Duplicate Names will
                    # be replaced with "F#"
                    variables = self.__formatrow__(types,values,False)
                    unknown = 1
                    while variables:
                        var = variables.pop(0)
                        if var in uniquevars or var == '':
                            var = 'F' + str(unknown)
                            unknown += 1
                        uniquevars.append(str(var))
                    firstrow = row + 1
                    break
            self.__sheetnames__.append(i)
            self.__sheets__.setdefault(i,{}).__setitem__('rows',sheet.nrows)
            self.__sheets__.setdefault(i,{}).__setitem__('cols',sheet.ncols)
            self.__sheets__.setdefault(i,{}).__setitem__('firstrow',firstrow)
            self.__sheets__.setdefault(i,{}).__setitem__('variables',uniquevars[:])
    def getiter(self, sheetname, returnlist=False, returntupledate=False):
        """ Return an generator object which yields the lines of a worksheet;
        Default returns a dictionary, specifing returnlist=True causes lists
        to be returned.  Calling returntupledate=True causes dates to returned
        as tuples of (Year, Month, Day, Hour, Min, Second) instead of as a
        string """
        if sheetname not in self.__sheets__.keys():
            raise NameError, "%s is not present in %s" % (sheetname,\
                                                          self.__filename__)
        if returnlist:
            return __iterlist__(self, sheetname, returntupledate)
        else:
            return __iterdict__(self, sheetname, returntupledate)
    def worksheets(self):
        """ Returns a list of the Worksheets in the Excel File """
        return self.__sheetnames__
    def nrows(self, worksheet):
        """ Return the number of rows in a worksheet """
        return self.__sheets__[worksheet]['rows']
    def ncols(self, worksheet):
        """ Return the number of columns in a worksheet """
        return self.__sheets__[worksheet]['cols']
    def variables(self,worksheet):
        """ Returns a list of Column Names in the file,
            assuming a tabular format of course. """
        return self.__sheets__[worksheet]['variables']
    def __formatrow__(self, types, values, wanttupledate):
        """ Internal function used to clean up the incoming excel data """
        ##  Data Type Codes:
        ##  EMPTY 0
        ##  TEXT 1 a Unicode string 
        ##  NUMBER 2 float 
        ##  DATE 3 float 
        ##  BOOLEAN 4 int; 1 means TRUE, 0 means FALSE 
        ##  ERROR 5 
        import xlrd
        returnrow = []
        for i in range(len(types)):
            type,value = types[i],values[i]
            if type == 2:
                if value == int(value):
                    value = int(value)
            elif type == 3:
                datetuple = xlrd.xldate_as_tuple(value, self.__book__.datemode)
                if wanttupledate:
                    value = datetuple
                else:
                    # time only no date component
                    if datetuple[0] == 0 and datetuple[1] == 0 and \
                       datetuple[2] == 0: 
                        value = "%02d:%02d:%02d" % datetuple[3:]
                    # date only, no time
                    elif datetuple[3] == 0 and datetuple[4] == 0 and \
                         datetuple[5] == 0:
                        value = "%04d/%02d/%02d" % datetuple[:3]
                    else: # full date
                        value = "%04d/%02d/%02d %02d:%02d:%02d" % datetuple
            elif type == 5:
                value = xlrd.error_text_from_code[value]
            returnrow.append(value)
        return returnrow
    
def __iterlist__(excel, sheetname, tupledate):
    """ Function Used To Create the List Iterator """
    sheet = excel.__book__.sheet_by_name(sheetname)
    for row in range(excel.__sheets__[sheetname]['rows']):
        types,values = sheet.row_types(row),sheet.row_values(row)
        yield excel.__formatrow__(types, values, tupledate)

def __iterdict__(excel, sheetname, tupledate):
    """ Function Used To Create the Dictionary Iterator """
    sheet = excel.__book__.sheet_by_name(sheetname)
    for row in range(excel.__sheets__[sheetname]['firstrow'],\
                     excel.__sheets__[sheetname]['rows']):
        types,values = sheet.row_types(row),sheet.row_values(row)
        formattedrow = excel.__formatrow__(types, values, tupledate)
        # Pad a Short Row With Blanks if Needed
        for i in range(len(formattedrow),\
                       len(excel.__sheets__[sheetname]['variables'])):
            formattedrow.append('')
        yield dict(zip(excel.__sheets__[sheetname]['variables'],formattedrow))
#RC-------------------------

#-----------------------------------------------------------------------------
# Setup common command line options for a Lightning project
#-----------------------------------------------------------------------------
def SetupOptions():
	opts = SCons.Options.Options('Lightning.py')
	opts.Add(SCons.Options.BoolOption('runtests', 'Default is to run unit tests, set "runtests=f" to disable', 0))
	opts.Add(SCons.Options.EnumOption('type', '"publish" creates an RC\n    "checkheaders" uncovers inclusion dependencies\n   ',
					'embedded', 
					allowed_values=('checkheaders', 'embedded', 'emulation', 'publish')))
	opts.Add(SCons.Options.BoolOption('resource', 'Default is to process resources, set "resource=f" to disable', 0))
	
	return opts


#-----------------------------------------------------------------------------
# Retrieve common command line options for a Lightning project
#-----------------------------------------------------------------------------
def RetrieveOptions(args, root_dir):

	is_runtests		= args.get('runtests', 1)
	type			= args.get('type', 'embedded')
	platform		= 'Lightning'
	variant			= 'LF1000'
	
	is_emulation 	= type == 'emulation' or type == 'checkheaders'
	is_resource		= args.get('resource', 1)	
	target_subdir			= platform + (is_emulation and '_emulation' or '')
	intermediate_build_dir	= os.path.join(root_dir, 'Temp', target_subdir)
	
	#FIXME/tp: Is this the best mechanism for allowing alternate nfsroot locations?
	# Target rootfs path may be USB device mount point instead of NFS path 
	# USB mount point for NAND partition contains implicit /Didj root path
	# NFS mount point needs explicit /Didj root path prepended
	rootfs = os.getenv('ROOTFS_PATH')
	if rootfs == None:
		rootfs = os.path.normpath(os.path.join(root_dir, '..', '..', 'nfsroot'))
	is_nandrootfs 		= rootfs.startswith('/media')
	if not is_nandrootfs:
		rootfs			= os.path.join(rootfs, 'Didj')

	#FIXME/tp: add mods for type == 'publish' here
	if is_emulation:
		bin_deploy_dir	= os.path.join(root_dir, 'Build', target_subdir)
	else:
		bin_deploy_dir	= os.path.join(rootfs, 'Base', 'Brio', 'bin')
		SCons.Script.Default(bin_deploy_dir)
		
	if type == 'publish':
		print '*** type=publish is a placeholder (a noop at this time).'
	
	vars = {'type'					: type,
			'is_emulation'			: is_emulation,
			'is_runtests'			: is_runtests,
			'platform'				: platform,
			'variant'				: variant,
			'target_subdir'			: target_subdir,
			'intermediate_build_dir': intermediate_build_dir,
			'bin_deploy_dir'		: bin_deploy_dir,
			'rootfs'				: rootfs,
			'is_resource'			: is_resource,
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
							tools = ['default', platform_toolset, 'cxxtest', 'runtest', 'checkheader', 'oggenc', 'oggext'], 
							toolpath = [toolpath],
					 )
	env.Append(CPPDEFINES = vars['variant'])
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
def CopyResources(penv, vars, psubfolder):
	
	if vars['is_emulation']:
		return
	data_root = penv.Dir('#Build/rsrc').abspath
	root_len = len(data_root) + 1
	rootfs_data = os.path.join(vars['rootfs'], 'Data')
	if not os.path.exists(rootfs_data):
		os.mkdir(rootfs_data)	
	rootfs_data = os.path.join(rootfs_data, psubfolder)
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
		ogl	= vars['variant'] == 'LF1000' and ['opengles_lite'] or ['ogl']
		platformlibs = ogl + ['dl', 'pthread', 'ustring', 'iconv', 'intl', 'sigc-2.0']
		#FIXME/tp: pthread should go away
		#RC CopyResources(penv, vars, psubfolder)
	
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
sEnv		= SCons.Environment.Environment(tools = ['oggenc', 'rawenc', 'oggext'], 
							toolpath = [toolpath])

#-------------------------------------------------------------------------
# Cache state to avoid duplicate packing and report multiple definitions
#-------------------------------------------------------------------------
sSourceToDestMap = {}

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
# Set up a map of type names and file extensions to numeric type values
#-------------------------------------------------------------------------
def SetupTypeConversionMap():
	types = { 'mid' 	: 1024 *  1 + 1,	#Audio Group
			  'midi' 	: 1024 *  1 + 1,
			  'S'		: 1024 *  1 + 1,		# for ACME
			  'SYN'		: 1024 *  1 + 1,		# for AudioCsv
			  'aogg'	: 1024 *  1 + 2,
			  'OGG'		: 1024 *  1 + 2,		# for AudioCsv
			  'wav'		: 1024 *  1 + 3,
			  'R'		: 1024 *  1 + 3,		# for ACME
			  'raw'		: 1024 *  1 + 3,		# for ACME
			  'avog'    : 1024 *  1 + 4,
              'relinkbin'	: 1024 *  4 + 1,	# Common Group
			  'dsetbin'		: 1024 *  4 + 2,
			  'json'	: 1024 *  4 + 3,
			  'so'		: 1024 *  4 + 4,
			  'xml'		: 1024 *  4 + 5,	# we need .so to be part of package
			  'bmp'		: 1024 *  5 + 1,	# Display Group
			  'gif'		: 1024 *  5 + 2,
			  'jpg'		: 1024 *  5 + 3,
			  'jpeg'	: 1024 *  5 + 3,
			  'png'		: 1024 *  5 + 4,
			  'rgb'		: 1024 *  5 + 5,
			  'font'	: 1024 *  7 + 1,	# Font Group
			  'ttf'		: 1024 *  7 + 1,
			  'ogg'		: 1024 * 14 + 1,	# Video Group
			  'vogg'	: 1024 * 14 + 2,
			  }
	return types

#-------------------------------------------------------------------------
# Convert the .xls to .audioCsv file. 
#-------------------------------------------------------------------------
def ConvertXlsToCsv(penv):
	#.audioCsv file contains the following fiels extracted from the input .xls files
	#    !Handle, !Type, !Compression, !Source File Location, Phrase Description, Character, Talent, Inflection_num
	#
	data_root = penv.Dir('#apprsrc').abspath
	pack_root = penv.Dir('#Build/rsrc').abspath
	
	xlfiles = glob.glob(os.path.join(data_root, '*.xls'))
	for xlfile in xlfiles:
		lines = []
		missing = []
		xl = readexcel(xlfile)
		xlfile = xlfile.replace(".xls", ".audioCsv")
		writer = csv.writer(open(os.path.join(data_root, xlfile), "w"))
		lines  += [['!Handle', '!Type', '!Compression', '!Source File Location', 'Phrase Description',  'Character', 'Talent', 'Inflection_num']]
		sheetnames = xl.worksheets()
		for sheet in sheetnames:
			for row in xl.getiter(sheet):
				handle = row['AUDIO HANDLE']
				if handle == '':
					continue
				audio_type = row['AUDIOTYPE']
				if (audio_type == 'S'): 
					type = 'SYN'
					compression = '' 
				elif audio_type == 'O-1':
					type = 'OGG'
					compression = 'OGG-1' 
				elif audio_type == 'O':
					type = 'OGG'
					compression = 'OGG0' 
				elif audio_type == 'O1':
					type = 'OGG'
					compression = 'OGG1' 
				elif audio_type == 'O2':
					type = 'OGG'
					compression = 'OGG2'
				elif audio_type == 'O3':
					type = 'OGG'
					compression = 'OGG3' 
				elif audio_type == 'O4':
					type = 'OGG'
					compression = 'OGG4' 
				else:
					print 'ConvertXlsToCsv: %s is an unsupported/invalid audio type' %audio_type
					raise 'Build Failed due to invalid/unsupported audio type'
				
				source = row['FILENAME']	
				if source == '':
					print 'Warning! Missing source for handle = ', handle
						
				phrase 		= row['PHRASE DESCRIPTION']
				character 	= row['CHARACTER']
				talent 		= row['TALENT']
				inflection	= row['INFLECTION']
				
				lines += [[handle, type, compression, source, phrase, character, talent, inflection]]
		
		deco = [(res[0].upper(), i, res) for i, res in enumerate(lines)]
		deco.sort()
		lines = [res for _, _, res in deco]
		writer.writerows(lines)	
#-------------------------------------------------------------------------
     
#-------------------------------------------------------------------------
# Transform or copy source file to packed output file
#-------------------------------------------------------------------------
def PackFile(pkg, srcfile, srcname, pack_root, type, compression, rate):
	# 1) If we have already processed this source file, return the
	#    already processed destination file
	# 2) Create output file path
	#    No auto-generated names now that resource manager is gone.
	# 3) Add the new output file to the Source->Dest map, return it
    #    (Not applicable to OGG Theora->Vorbis extraction) 
	# 4) Invoke OGG encoder
	# 5) Invoke utility that converts WAV to RAW PCM w/Brio header
	# 6) Invoke utility that extracts OGG Vorbis from OGG Theora
	# A) No encoder needed, just copy the file
	#
	if sSourceToDestMap.has_key(srcfile):							#1
		return sEnv.File(sSourceToDestMap[srcfile])
		
	## outname = GenerateNextResourceFileName()						#2
	# use outname as-is since there is no resource manager anymore 
	outname = srcname
	
	# this will make sure AboutMe so files are all named App.so
	if pkg.endswith("AboutMe.pkg"):
		if srcfile.endswith(".so"):
			outname = "App.so"
    
	outfile = os.path.join(pack_root, outname)
	if type != (1024*14+1):
		sSourceToDestMap[srcfile] = outfile							#3
	
	if type == 1026 and rate != '':
		outfile = outfile.replace(".wav", ".ogg")
		enc = sEnv.OggEnc(outfile, srcfile, OGGENC_RATE=rate, OGGENC_COMPRESSION=compression)
		
	elif type == 1027:												#5
		outfile = outfile.replace(".wav", ".raw")
		enc = sEnv.RawEnc(outfile, srcfile)
		
	elif type == 1028 and rate != '':							    #6
		outfile = outfile.replace(".ogg", ".aogg")
		enc = sEnv.OggExt(outfile, srcfile, OGGEXT_RATE=rate)

	else:
		enc = sEnv.Command(outfile, srcfile, [SCons.Defaults.Copy('$TARGET', '$SOURCE')])	#A
		
	return enc[0]


#-------------------------------------------------------------------------
# Pack contents of an individual ACME CSV file
#-------------------------------------------------------------------------
def ProcessAudioCsv(pkg, types, pack_root, data_root, enumpkg):
	# 1) Setup field mappings (see note below)
	# 2) Read in the mapping of activity code to Base URI path
	#    (for converting acme CSV files)
	#    No more Base URI path in the pkg input file.
	# NOTE: The 'fld' param tells the ProcessPackage() function which
	# fields are of interest in the input file, where:
	#   0: activity code or base URL path
	#   1: "handle" or URL node value
	#   2: data type string 
	#   3: file name
	#	
	#RC fld = [7, 17, 15, 9]	# activity, handle, type, file			#1
	# dict = MapAcmeActivitiesToURIs(data_root)						#2
	dict = {}	
	pack_root_len = len(pack_root) + 1
	
	log = open(os.path.join(pack_root, 'log.txt'), 'w')
	d = datetime.now()
	log.write("ProcessAudioCsv : %s \n" % d.strftime())

	reader = csv.reader(open(pkg, "rb"))
	line = 0
	statusline = []
	for row in reader:
		line += 1
		if len(row) == 0 or (row[0] and row[0][0] == '!') or (row[0] and row[0][0] == '#'):			
			continue
		if line == 1:
			# TODO: map 'fld' to 'SHAPE DESCRIPTION', 'FILE', 'AUDIOTYPE', & 'AUDIO HANDLE'
			dummy = line	
		elif row[3] != '':
			compression = rate = ''
			
			if types.has_key(row[1]):
				type = types[row[1]]
			else:
				type = 1025
				
			if type == 1026:
				if row[2] == 'OGG-1':
					compression = -1
				elif row[2] == 'OGG0':
					compression = 0
				elif row[2] == 'OGG1':
					compression = 1
				elif row[2] == 'OGG2':
					compression = 2
				elif row[2] == 'OGG1':
					compression = 3
				elif row[2] == 'OGG2':
					compression = 4
				else:	
					log.write('ProcessAudioCsv: Invalid/unsupported audio type: %s\n' %row[2])
					raise 'Error! Invalid audio type!'
				rate = 16000
				
			extension = type == 1024 and '.mid' or '.wav' or '.aif'
			srcname = row[3]+extension
			srcfile = os.path.join(data_root, row[3]+extension)
			
			handle = row[0]
			if dict.has_key(handle):
				log.write('ProcessAudioCsv: Duplicate handle : %s \n' %handle)
				log.write('ProcessAudioCsv: Build fails due to duplicate handles! \n')
				raise 'Error! Build fails due to duplicate handles!'
				
			
			if dict.get(handle) == srcname:
				print 'Warning! %s : referenced multiple times '%srcname
				log.write('ProcessAudioCsv: %s referenced multiple times \n' %srcname)
				
			dict[handle] = srcname	#RC Store handle->source file mapping
			srcname = srcname[:0] + handle + extension
			
			shutil.copyfile(srcfile, os.path.join(data_root, srcname))
			srcfile = os.path.join(data_root, srcname)
			
			outfile = PackFile(pkg, srcfile, srcname, pack_root, type, compression, rate)
			
			
#-------------------------------------------------------------------------
# Pack contents of an individual package
#-------------------------------------------------------------------------
def ProcessPackage(pkg, types, pack_root, data_root, packages):
	# Package input format is "baseURIPath, URINode, srcFile, (optional)packtype"
	#
	# 1) Open input and output packages
	# 2) Skip comment lines
	# 3) No more URI info in the pkg input file.
	# 4) No more default line. 
	# 5) Get the type info either from the type field or the file extension.
	#    Field 4 of the CSV file is a combined type/compression/rate field, 
	#    joined together by underscores.  An example would be "AOGG_3_16000"
	#    for and audio OGG file compressed at quality 3 at a sample rate of
	#    16000.  Parse that into its constituent parts.
	# 6) No more URI info for each line.
	# 7) Get the source file name and pack or copy the source to the output.
	#    Add a line describing the resource to the "resources" list.
	# 8) No longer sort since URI path is gone in the input pkg file.
	reader = csv.reader(open(pkg, "rb"))							#1
	line = 0
	resources = []
	pack_root_len = len(pack_root) + 1
	
	for row in reader:
		line += 1
		if len(row) == 0 or (row[0] and row[0][0] == '#'):			#2
			continue

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
			
		srcname = row[2].strip()
		srcfile = os.path.join(data_root, row[2].strip())			#7
		srcsize = os.path.getsize(srcfile)
		outfile = PackFile(pkg, srcfile, srcname, pack_root, type, compression, rate)


#-------------------------------------------------------------------------
# Deploy the apprsrc assets for embedded builds
#-------------------------------------------------------------------------
def ProcessResources(penv, vars, pappname):
	# 0) Do nothing for "clean" operations
	# 1) Setup source and destination folders
	# 2) Find all "ACME" input package definition files and process them
	# 3) Find all standard package input files and process them
	# 4) Sort the packages and report any duplicate package URIs
	# 5) Report any duplicate package names
	# 6) Write them to the rsrc/EnumPkgs file
	#	No longer doing steps 4 to 6 now that the resource manager is gone.
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
	
	ConvertXlsToCsv(penv)
					
	pkgs = glob.glob(os.path.join(data_root, '*.audioCsv'))				#2
	for pkg in pkgs:
		ProcessAudioCsv(pkg, types, pack_root, data_root, packages)
						
	pkgs = glob.glob(os.path.join(data_root, '*.pkg'))				#3
	for pkg in pkgs:
		ProcessPackage(pkg, types, pack_root, data_root, packages)
					
	CopyResources(penv, vars, pappname)
	
	
					
#TODO: Deal with greater than 36^2 resources (hierarchical folders).  Currently an exception is raised.

#-----------------------------------------------------------------------------
# Export the all of the functions symbols
#-----------------------------------------------------------------------------
__all__ = ["SetupOptions", "RetrieveOptions", "CreateEnvironment", "MakeMyApp", "ProcessResources"] 

