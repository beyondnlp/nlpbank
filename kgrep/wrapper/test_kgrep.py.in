# -*- encoding: utf8 -*-

import os
import sys
import argparse

import ctypes
from ctypes import *
from kgrep import KGREP
'''
		{"help",         no_argument, 0, 'h'},
		{"file",         no_argument, 0, 'f'},
		{"expand",       no_argument, 0, 'e'},
		{"delimiter",    no_argument, 0, 'd'},
		{"field",        no_argument, 0, 'k'},
		{"prefix",       no_argument, 0,  0 },
		{"suffix",       no_argument, 0,  0 },
		{"substring",    no_argument, 0, 's'},
		{"inverse-match",no_argument, 0, 'v'},
		{"match-pattern",no_argument, 0, 'm'},
'''

if __name__ == "__main__" :

    parser = argparse.ArgumentParser()
    parser.add_argument("-e", "--expand", type=str, dest="expand",      default=0,  help="expand")
    parser.add_argument("-d", "--delimiter", dest="delimiter",     default="\t",  help="delimiter")
    parser.add_argument("-f", "--filename",  dest="filename",  help="filename")
    parser.add_argument("-k", "--field", dest="field",  help="field")
    parser.add_argument(      "--prefix", action='store_true', dest="prefix",  help="prefix")
    parser.add_argument(      "--suffix", action='store_true', dest="suffix",  help="suffix")
    parser.add_argument("-s", "--substring", action='store_true', dest="substring",  help="substring")
    parser.add_argument("-v", "--inverse", action='store_true', dest="inverse",  help="inverse")
    parser.add_argument("-m", "--match", action='store_true', dest="match",  help="match-pattern")
    parser.add_argument("-c", "--color", action='store_true', dest="color",  help="color")
    options = parser.parse_args()
    print(options)
    kgrep = KGREP( '/data1/jason/kgrep/install' )
    kgrep.init( options )


    while 1:
        try: line = sys.stdin.readline()
        except KeyboardInterrupt: break;
        if not line: break
        try : line=line.rstrip();
        except : continue;
        kgrep.match( line )
    kgrep.free()
