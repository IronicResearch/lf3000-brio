#!/usr/bin/env python
"""
Update the meta.inf files from the template.
"""

import sys
import os
import optparse
import subprocess
import string
import re

def getArgs():
    str_usage = "usage:  %prog template_path metainf_path [options]"
    parser = optparse.OptionParser(usage=str_usage)
    parser.add_option("-r", "--revision", action="store", type="int", dest="revision", 
                      help="SVN revision number of the last build.")
    parser.add_option("-p", "--path", action="store", type="string", dest="path",
                      default='.',
                      help="Specify the path for svnversion to check the revision.  Default is '.'")
    options, args = parser.parse_args()
    if not len(args) == 2:
        parser.print_help()
    return options, args

def createMetaInf(template_path, metainf_path, revision):
    f_template = open(template_path, 'r')
    s = f_template.read()
    s_template = string.Template(s)
    dct = {'SVN_REVISION':revision}
    f_meta = open(metainf_path, 'w')
    s_sub = s_template.substitute(dct)
    f_meta.write(s_sub)

def getSvnRevisionForFile(path):
    cmdList = ['svn', 'info', '--non-interactive', path]
    output = subprocess.Popen(cmdList, stdout=subprocess.PIPE).communicate()[0]
    output = output.splitlines()
    for line in output:
        line = str(line)
        #print(line)
        if line.startswith("Last Changed Rev:"):
            msg, rev = line.split(':', 1)
            return int(rev)
    # TODO: raise an error
    return None
    
def getSvnRevision(path):
    """Return the svn revision number of the resource at path using svnversion.
    This works for directories.
    TODO: This was copied and pasted from LightningCore/CoreTools/WhatPackagesChanged.py
    This should be imported and shared rather than duplicated.
    """
    if os.path.isfile(path):
        return getSvnRevisionForFile(path)
    rawstr = r"""(?P<rev>\d+)   # greedy match of at least one digit
    \:?(?P<mixedrev>\d*)        # in the case of a mixed revision, this is the latest revision 
                                # in the range and rev is the earliest
    (?P<modified>M?)            # if present, the working copy contains modifications
    (?P<switched>S?)            # if present, the working copy is switched""".encode('latin1')
    cmdList = ['svnversion', '--committed', path]
    output = subprocess.Popen(cmdList, stdout=subprocess.PIPE).communicate()[0]
    output = output.strip()
    compile_obj = re.compile(rawstr,  re.VERBOSE)
    match_obj = compile_obj.search(output)
    if match_obj:
        rev = int(match_obj.group('rev'))
        try:
            mixedrev = int(match_obj.group('mixedrev'))
        except ValueError:
            mixedrev = None
        modified = match_obj.group('modified') and True
        switched = match_obj.group('switched') and True
        if modified:
            print("Warning:  modified working copy:", path)
        if switched:
            print("Warning:  switched working copy:", path)
        if mixedrev:
            print("Warning:  mixed svn revision:", rev, "for path:", path)
            return max(rev, mixedrev)
        return rev
    else:
        print("ERROR:  Could not parse the output of svnversion")
        print("\tcommand:", " ".join(cmdList))
        return output

def main():
    options, args = getArgs()
    if options.revision:
        rev = options.revision
    else:
        rev = getSvnRevision(options.path)
    print("revision:", rev)
    try:
        template_path, metainf_path = args
#        print("template_path:", template_path)
#        print("metainf_path: ", metainf_path)
    except ValueError:
        # getArgs already prints a helpful usage message
        return
    createMetaInf(template_path, metainf_path, rev)

if __name__=='__main__':
    main()
