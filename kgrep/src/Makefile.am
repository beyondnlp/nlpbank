lib_LTLIBRARIES = libkgrep.la

# chaining hash library 
libkgrep_la_SOURCES = kgrep.c
# LDFLAGS: Please use one of -static or -dynamic
libkgrep_la_LDFLAGS = -avoid-version -no-undefined
libkgrep_la_CFLAGS = -g -Wall -fPIC


bin_PROGRAMS = kgrep

kgrep_SOURCES   = kgrep.c
kgrep_CFLAGS    = -g -Wall -fPIC 
kgrep_LDFLAGS   = -static -avoid-version 
