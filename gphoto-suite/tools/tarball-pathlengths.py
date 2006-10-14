#!/usr/bin/python
"""
tarball-pathlengths.py - show largest path lengths in tarball
Syntax: %(prog)s <tarballs...>


"""

import sys
import tarfile
import pprint

def check_tarfile(filename):
    tar = tarfile.open(filename, 'r')
    fnames = tar.getnames()
    fnames.sort(lambda a,b:cmp(len(b),len(a)))
    for fname in fnames[:20]:
        print "%3d %s" % (len(fname), fname)
    tar.close()

def main(argv):
    for filename in argv[1:]:
        check_tarfile(filename)

if __name__ == '__main__':
    main(sys.argv)
    
