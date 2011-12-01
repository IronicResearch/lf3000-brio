import os
import Etc.Tools.SConsTools.Priv.LfUtils

#--------------------------------------------------------
# Setup command line options
#--------------------------------------------------------

# SCons 1.2.0 has a nasty bug that messes with linker dependency order for some embedded libs
# The bug is fixed in 1.3.0 (if you HAVE to use 1.2.0, you can by removing this line,
# but beware that link order may differ from a proper build)
# EnsureSConsVersion(1, 3, 0)

opts = Variables()
opts.Add('platform', 'Set platform to use', 'LF2000')
opts.Add(BoolVariable('monolithic', 'Set "monolithic=t" to link EXEs against .a files rather than .so files', False))
opts.Add(BoolVariable('debug', 'Builds libraries and unit tests with symbols', False))
opts.Add(BoolVariable('samples', 'Include samples if building an SDK target', True))
opts.Add(BoolVariable('runtests', 'Builds and runs unit tests (implies buildtests)', False))
opts.Add(BoolVariable('buildtests', 'Builds unit tests but does not run them', False))

opts.Add('setup', 'Set to "TRUNK" or branch name to setup source tree for a platform', '')
opts.Add('host', 'The architecture that will run the embedded binaries', 'arm-linux')
opts.Add('libc', 'The c runtime library to use for exporting and linking libraries', 'uclibc')

opts.Add('staging_dir', 'Root where we find linux standard libs and headers, usually nfsroot', '')
opts.Add('embedded_root', 'Where to place Brio arm libraries',  '')
opts.Add('emulation_root', 'Where to place Brio x86 libraries', '')
opts.Add('sdk_root', 'Where to build SDK', '')

opts.Add(EnumVariable('type', 'Legacy support for the old style of targets', '', 
		['', 'embedded', 'emulation', 'xembedded', 'xemulation', 'publish']))

#Setup Toolpaths since they are shared across embedded/emulation
toolpath1 = os.path.join('Etc', 'Tools', 'SConsTools')
toolpath2 = os.path.join(toolpath1, 'Priv')

global_tools = ['cxxtest', 'runtest', 'metainf']

master_env = Environment(variables = opts, toolpath = [toolpath1, toolpath2], tools = global_tools)
master_env.PrependENVPath( 'PATH', os.environ['PATH'] )
master_env.Tool('default')

Help(opts.GenerateHelpText(master_env))

#Specifying runtests implies buildtests
if master_env['runtests']:
	master_env['buildtests'] = True

#Fix up usrlib_dir
#TODO: It would be good to have this based on embedded_root if it's not specified
if not master_env['staging_dir']:
	if 'ROOTFS_PATH' in os.environ:
		master_env['staging_dir'] = os.environ['ROOTFS_PATH']
	else:
		master_env['staging_dir'] = os.path.join(os.environ['HOME'], 'nfsroot')
	
master_env['staging_dir'] = Dir(master_env['staging_dir'])

#Fix up root paths
default_subdir = os.path.join('LF', 'Base', 'Brio')
if not master_env['embedded_root']:
	if 'ROOTFS_PATH' in os.environ:
		master_env['embedded_root'] = os.environ['ROOTFS_PATH']
	else:
		master_env['embedded_root'] = os.path.join(os.environ['HOME'], 'nfsroot')

master_env['embedded_root'] = Dir(master_env['embedded_root']).Dir(default_subdir)

if not master_env['emulation_root']:
	if 'EMUROOTFS_PATH' in os.environ:
		master_env['emulation_root'] = os.environ['EMUROOTFS_PATH']
	else:
		master_env['emulation_root'] = os.path.join(os.environ['HOME'], 'emuroot')

master_env['emulation_root'] = Dir(master_env['emulation_root']).Dir(default_subdir)

#Get svn repo numbering
master_env['version'] = Etc.Tools.SConsTools.Priv.LfUtils.GetRepositoryVersion(master_env['platform'], master_env['setup'])

#Fix up SDK path and samples flag based on legacy targets or not
if not master_env['sdk_root']:
	if master_env['type'] == 'publish' or master_env['type'] == '':
		master_env['sdk_root'] = Dir('#Publish_'+master_env['version'])
	else:
		master_env['sdk_root'] = Dir('#XBuild')
		master_env['samples'] = False
else:
	master_env['sdk_root'] = Dir(master_env['sdk_root'])

#Setup directories
master_env['build_dir'] = None
master_env['intermediate_dir'] = None
master_env['base_dir'] = None
master_env['install_dir'] = None
master_env['hdr_deploy_dir'] = None
master_env['tools_deploy_dir'] = None
master_env['cpu'] = ''
master_env['cpu_bare'] = ''
master_env['is_sdk'] = False

if master_env['monolithic']:
	master_env.Append(CPPDEFINES = 'LF_MONOLITHIC_DEBUG')

if master_env['platform'] == 'Lightning':
	master_env.Append(CPPDEFINES = ['LF1000'])

#Allow CPPFLAGS to bleed through. This allows for native builds in oe-core	
if 'CPPFLAGS' in os.environ:
	master_env.Append(CPPFLAGS = os.environ['CPPFLAGS'].split())

#--------------------------------------------------------
# Setup our build environments
#--------------------------------------------------------
environments = {}

#Embedded environment (Builds arm libraries and installs to nfsroot)
environments['embedded'] = master_env.Clone()
environments['embedded'].Tool(environments['embedded']['platform']+'_embedded')
environments['embedded']['intermediate_dir'] = Dir('#System').Dir('Temp').Dir(environments['embedded']['platform'])
environments['embedded']['build_dir'] = Dir('#Build').Dir(environments['embedded']['platform'])
environments['embedded']['install_dir'] = master_env['embedded_root']
environments['embedded']['cpu'] = 'arm-' + environments['embedded']['libc']
environments['embedded']['cpu_bare'] = 'arm'
environments['embedded'].Prepend(LIBPATH = [environments['embedded']['build_dir'].Dir('MPI')])
#TODO: This should point at staging and all required libs should be populated by their respective builds
environments['embedded'].Append(LIBPATH = [environments['embedded']['embedded_root'].Dir('lib') ] )

#Emulation environement(Builds x86 libraries and installs to emuroot)
environments['emulation'] = master_env.Clone()
environments['emulation'].Tool(environments['emulation']['platform']+'_emulation')
environments['emulation']['intermediate_dir'] = Dir('#System').Dir('Temp').Dir(environments['emulation']['platform']+'_emulation')
environments['emulation']['build_dir'] = Dir('#Build').Dir(environments['emulation']['platform']+'_emulation')
environments['emulation']['install_dir'] = master_env['emulation_root']
environments['emulation']['cpu'] = 'x86'
environments['emulation'].Prepend(LIBPATH = [environments['emulation']['build_dir'].Dir('MPI')])

#SDK headers and tools environment(Installs SDK headers and tools into SDK dir)
environments['sdk_headers'] = master_env.Clone()
environments['sdk_headers']['base_dir'] = master_env['sdk_root']
environments['sdk_headers']['hdr_deploy_dir'] = environments['sdk_headers']['base_dir'].Dir('Include')
environments['sdk_headers']['is_sdk'] = True

#XBuild embedded environment(Installs arm libraries into XBuild dir)
environments['sdk_embedded'] = environments['embedded'].Clone()
environments['sdk_embedded']['base_dir'] = master_env['sdk_root']
environments['sdk_embedded']['install_dir'] = environments['sdk_embedded']['base_dir'].Dir('Libs').Dir(environments['sdk_embedded']['cpu_bare'])
environments['sdk_embedded']['build_dir'] = None
environments['sdk_embedded']['is_sdk'] = True

#XBuild emulation environment(Installs x86 libraries into XBuild dir)
environments['sdk_emulation'] = environments['emulation'].Clone()
environments['sdk_emulation']['base_dir'] = master_env['sdk_root']
environments['sdk_emulation']['install_dir'] = environments['sdk_emulation']['base_dir'].Dir('Libs').Dir(environments['sdk_emulation']['cpu'])
environments['sdk_emulation']['build_dir'] = None
environments['sdk_emulation']['is_sdk'] = True

environments['sdk_tools'] = master_env.Clone()
environments['sdk_tools']['base_dir'] = master_env['sdk_root']
environments['sdk_tools']['tools_deploy_dir'] = environments['sdk_tools']['base_dir'].Dir('Tools')
environments['sdk_tools']['is_sdk'] = True

#--------------------------------------------------------
# Export Samples to Publish directory
#--------------------------------------------------------

env = environments['sdk_headers']
Export('env')
publish_samples = SConscript( os.path.join(master_env['platform'], 'SConscript'), duplicate=False)
publish_samples += SConscript( os.path.join('Samples', 'SConscript'), duplicate=False)
Alias('sdk_headers', publish_samples)

#--------------------------------------------------------
# Export Tools to export and publish targets
#--------------------------------------------------------

#Run through each environment and run it through SConscripts
for target, env in environments.iteritems():
	Export('env')
	Alias(target, SConscript( os.path.join('Etc', 'SConscript') ) )
	Alias(target, SConscript( os.path.join('ThirdParty', 'SConscript') ) )
	Alias(target, SConscript( os.path.join('System', 'SConscript') ) )

#-------------------------------------------------------------------------
# Deploy meta.inf file for embedded builds
#-------------------------------------------------------------------------

templatePath = os.path.join(master_env['platform'], 'meta.inf')
metaInfPath = master_env['embedded_root'].File('meta.inf').abspath
Alias('embedded', master_env.MetaInf(metaInfPath, templatePath) )

#--------------------------------------------------------
# Inter-target dependencies
#--------------------------------------------------------

Alias('sdk_embedded', 'sdk_headers')
Alias('sdk_embedded', 'sdk_tools')

Alias('sdk_emulation', 'sdk_headers')
Alias('sdk_emulation', 'sdk_tools')

Alias('sdk_embedded', 'embedded')
Alias('sdk_emulation', 'emulation')

Alias('sdk', 'embedded')
Alias('sdk', 'emulation')
Alias('sdk', 'sdk_headers')
Alias('sdk', 'sdk_embedded')
Alias('sdk', 'sdk_emulation')
Alias('sdk', 'sdk_tools')

#Setup default targets based on legacy target, or embedded if none given
if not env['type'] or env['type'] == 'embedded':
	Default('embedded')
elif env['type'] == 'emulation':
	Default('emulation')
elif env['type'] == 'xembedded':
	Default('sdk_embedded')
elif env['type'] == 'xemulation':
	Default('sdk_emulation')
elif env['type'] == 'publish':
	Default('sdk')
