#!/usr/bin/env python
'''
A git-friendly alternative to SvnRevision.py

This copy pasted from Lightning with the expectation that 
someday Lightning will be part of Brio.
'''

import sys

import subprocess

def getCommitList(paths, from_commit = None, count = None):
    command = ['git', 'log', '--pretty=format:%H']
    if count: command.append("--max-count={0}".format(count))
    if from_commit: command.append(from_commit)
    command.append('--')
    command.extend([str(path) for path in paths])
    output = subprocess.check_output(command)
    return output.split()

def getCommitCount(paths, from_commit = None):
    commits = getCommitList(paths, from_commit)
    return len(commits)

def getLastCommit(paths):
    return getCommitList(paths, count = 1)[0]

def main():
    paths = sys.argv[1:]
    print getCommitCount(paths)

if __name__=='__main__':
    main()
