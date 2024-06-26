#!/usr/bin/env python
# -*- encoding: utf8 -*-

"""
kgrep python wrapper using ctypes.
 __author__ = 'Kim Eung Gyun'
 __copyright__ = 'Copyright (C) 2019-, Kakao All rights reserved.'
"""

###########
# imports #
###########
from __future__ import unicode_literals
from __future__ import print_function
import os
import ctypes
from ctypes import c_int, c_uint, c_ubyte, c_char_p, c_void_p, c_size_t, c_char, c_short, c_byte,\
                   POINTER, CDLL, cast, CFUNCTYPE, byref

#############
# constants #
#############
EXTENSION = [".so", ".dylib", ".dll"]
MAX_OUT_SIZE = 10240


#########
# types #
#########

class Ynext(ctypes.Structure):
    __fields__ = [
        ("code", c_ubyte ),
        ("address", c_uint )
    ]


class Node(ctypes.Structure):
    pass

Node.__fields__ = [

    ( "index",  c_uint ),
    ( "str",    POINTER(c_ubyte)),
    ( "code",   c_ubyte ),
    ( "lock",   c_ubyte ),
    ( "size",   c_ubyte ),
    ( "alloc",  c_ubyte ),
    ( "ycount", c_ubyte ),
    ( "xnext",  POINTER( Node )),
    ( "ynext",  POINTER( Node )),
    ( "ynexts", POINTER( Ynext)),
    ( "val",    c_char_p )
]

NONE      = 0
PREFIX    = 1
SUFFIX    = 2
SUBSTRING = 4
INVERSE   = 8
MATCH_PATTERN = 16
PATTERN_FILE  = 32
EXPAND = 64
class FILE(ctypes.Structure):
    """
    stdlib.h의 FILE 대체
    """
    pass

class Dgrep_make_trie(ctypes.Structure):
    __fields__ = [
        ( "root", POINTER( Node )),
        ( "node_num", c_uint ),
        ( "Index_List", POINTER(POINTER( Node ))),
        ( "Index_Seek", POINTER( c_uint ))
    ]

class Dgrep(ctypes.Structure):
    __fields__ = [
    ( "fp", POINTER( FILE ) ),
    ( "op_type", c_int ),
    ( "delimiter", c_char*8 ),
    ( "field", c_int ),
    ( "trie", POINTER( Dgrep_make_trie ) )
]
    

#############
# functions #
#############
class KGREP(object):
    """
    kgrep wrapper
    """

    obj=None
    libkgrep=None
    libname="libkgrep"

    def __init__(self, so_path=None):
        self.open_object(so_path)
        self.mapping_functions()

    def open_object(self, so_path):
        """
        입력된 경로를 대상으로 so파일을 오픈합니다.
        """
        for ext in EXTENSION:
            if not so_path or not os.path.isdir(so_path):
                curr_path = os.path.dirname(os.path.abspath(__file__))
                lib_file = os.path.join(curr_path, self.libname ) + ext
            else:
                lib_file = os.path.join(so_path, "lib", self.libname) + ext
            if os.path.isfile(lib_file):
                self.libkgrep = CDLL(lib_file)
                break
        if not self.libkgrep:
            raise NameError("cannot find kgrep shared library file")

    def mapping_functions(self):
        """
        so에 있는 함수와 wrapper에서 사용할 함수를 서로 매핑합니다.
        """


        ## kgrep_make_trie
        self.initialize = self.libkgrep.kgrep_make_trie
        self.initialize.restype = POINTER(Dgrep_make_trie)


        #kgrep_t *read_kgrep_opt( int argc, char *argv[] ){
        self.read_kgrep_opt = self.libkgrep.read_kgrep_opt
        self.read_kgrep_opt.argtypes = [ c_int, POINTER( c_char_p )]
        self.read_kgrep_opt.restype = POINTER( Dgrep )

        self.finalize = self.libkgrep.kgrep_free
        self.finalize.argtypes = [POINTER(Dgrep)]

        #int kgrep_matching( kgrep_t * kgrep ){
        self.matching = self.libkgrep.kgrep_matching
        self.matching.argtypes = [POINTER( Dgrep )]
        self.matching.restype = c_int


        #kgrep_make_trie_t *kgrep_make_trie( ){
        self.kgrep_make_trie = self.libkgrep.kgrep_make_trie
        self.kgrep_make_trie.argtypes = None
        self.kgrep_make_trie.restype = POINTER( Dgrep_make_trie )

        #int kgrep_insert_trie( kgrep_make_trie_t *trie, char * key, char *val ){
        self.kgrep_insert_trie = self.libkgrep.kgrep_insert_trie
        self.kgrep_insert_trie.argtypes = [ POINTER( Dgrep_make_trie), c_char_p, c_char_p ]
        self.kgrep_insert_trie.restype = c_int

        #int kgrep_build_trie( kgrep_make_trie_t* trie ){
        self.kgrep_build_trie = self.libkgrep.kgrep_build_trie
        self.kgrep_build_trie.argtypes = [ POINTER( Dgrep_make_trie ) ]
        self.kgrep_build_trie.restype = c_int

        #int kgrep_read_input_group_file( kgrep_t *kgrep, char * pattern_fn, char *group_name ){
        self.kgrep_read_input_groupu_file = self.libkgrep.kgrep_read_input_group_file
        self.kgrep_read_input_groupu_file.argtypes = [ POINTER( Dgrep ), c_char_p, c_char_p ]
        self.kgrep_read_input_groupu_file.restype = c_int

        #void set_opt( op_type_t *op_type , int opt ){
        self.set_opt = self.libkgrep.set_opt
        self.set_opt.argtypes = [ POINTER(Dgrep), c_int ]
        self.set_opt.restype = None

        #void kgrep_load_group( kgrep_t *kgrep, char* pattern, char* group ){
        self.kgrep_load_group = self.libkgrep.kgrep_load_group
        self.kgrep_load_group.argtypes = [ POINTER(Dgrep), c_char_p, c_char_p  ]
        self.kgrep_load_group.restype = None

		#kgrep_t *kgrep_make_kgrep( ){
        self.kgrep_make_kgrep = self.libkgrep.kgrep_make_kgrep
        self.kgrep_make_kgrep.argtypes = None
        self.kgrep_make_kgrep.restype = POINTER(Dgrep)

		#int kgrep_match_in_line( kgrep_t *kgrep, char* buffer ){
        self.kgrep_match_in_line = self.libkgrep.kgrep_match_in_line
        self.kgrep_match_in_line.argtypes = [ POINTER(Dgrep), c_char_p ]
        self.kgrep_match_in_line.restype = c_int


		# void set_field( kgrep_t *kgrep, char *field ){
        self.set_field = self.libkgrep.set_field
        self.set_field.argtypes = [ POINTER(Dgrep), c_char_p ]
        self.set_field.restype = None


		# void set_delimiter( kgrep_t *kgrep, char* delimiter ){
        self.set_delimiter = self.libkgrep.set_field
        self.set_delimiter.argtypes = [ POINTER(Dgrep), c_char_p ]
        self.set_delimiter.restype = None

    def kgrep_set_option( self, kgrep, options ):
        if options.suffix :  self.set_opt( kgrep, SUFFIX )
        if options.prefix :  self.set_opt( kgrep, PREFIX )
        if options.substring:self.set_opt( kgrep, SUBSTRING )
        if options.inverse : self.set_opt( kgrep, INVERSE )
        if options.match :   self.set_opt( kgrep, MATCH_PATTERN )
        if options.color :   self.set_opt( kgrep, COLOR )


        field = "".encode('utf-8')
        if options.field : 
            field = options.field
            if isinstance( field, str ): field = field.encode('utf-8')
            self.set_field( self.obj, field )

        delimiter = "".encode('utf-8')
        if options.delimiter : 
            delimiter = options.delimiter
            if isinstance( delimiter, str ): delimiter = delimiter.encode('utf-8')
            self.set_delimiter( self.obj, field )

        group = "".encode('utf-8')
        if options.expand : 
            self.set_opt( kgrep, EXPAND )
            group = options.expand
            if isinstance( group, str ): group = group.encode('utf-8')

        pattern = "".encode('utf-8')
        if options.filename : 
            self.set_opt( kgrep, PATTERN_FILE )
            pattern = options.filename
            if isinstance( pattern, str ): pattern = pattern.encode('utf-8')

        self.kgrep_load_group( kgrep, pattern, group )
        return kgrep

    def init( self, options ):
        self.obj = self.kgrep_make_kgrep()
        self.obj = self.kgrep_set_option( self.obj, options );
        return self.obj

    def match( self, line ):
        if isinstance( line, str ): line = line.encode('utf-8')
        self.kgrep_match_in_line( self.obj, line )

    def free( self ):
        self.finalize( self.obj )


