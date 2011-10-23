#!/usr/bin/env python
# -*- coding: us-ascii -*-
# vim:ts=4:sw=4:softtabstop=4:smarttab:expandtab
#
"""
Requires py2exe from http://py2exe.org

Issue:

    c:\Python24\python setup.py py2exe
    setup.py py2exe

# TODO consider using http://pypi.python.org/pypi/innosetup/
# TODO consider including openhardwaremonitor-v0.3.2-beta and libusb (and .inf) files
"""

import sys
import glob
from distutils.core import setup

import py2exe


if len(sys.argv) == 1:
    print 'defaulting to creating py2exe'
    sys.argv += ['py2exe']



# disable optimizeration- we _may_ need docs strings, specifically "copyright"

setup(
    options = {"py2exe": {
                            #"includes": ["decimal"],
                            "optimize": 1,  ## 1 and NOT 2 because I use the __doc__ string as the usage string. 2 optimises out the doc strings
                            'bundle_files': 1,
                            
                            # Exclude encodings - do not use this if encoding/decoding is done. e.g. get errors 
                            # LookupError: no codec search functions registered: can't find encoding
                            # LookupError: unknown encoding: ABCD....
                            'ascii': True,
                            
                            'excludes':[
                                        '_ssl',  # Exclude _ssl
                                        'pyreadline', 'difflib', 'doctest', 'locale',
                                        'optparse', 'pickle', 'calendar',# Exclude standard library
                                        "Tkconstants","Tkinter","tcl",
                                        #'re',  # used by wmi module
                                        ],  
                            }},
    zipfile = None, ## try and make a single exe, if do not want this loose this and the 'bundle_files' option
    console=['event_display.py']
    )



def main(argv=None):
    if argv is None:
        argv = sys.argv
    
    if 'py2exe' in argv:
        # Assume zip wanted too
        zipfilename = 'asusdisplay.zip'
        zipfilelist = glob.glob('dist/*')
        zipfilelist.append('320x240_hal256.png')
        # TODO consider including openhardwaremonitor-v0.3.2-beta and libusb (and .inf) files

        import zipfile
        z = zipfile.ZipFile(zipfilename, 'w')
        for x in zipfilelist:
            z.write(x)
        z.close()

        print 'Created:', zipfilename
    
    return 0


if __name__ == "__main__":
    sys.exit(main())

