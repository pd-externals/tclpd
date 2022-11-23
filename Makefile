# Makefile to build loader 'tclpd' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = tclpd

TCL_CFLAGS=$(shell pkg-config --cflags tcl)
TCL_LIBS=$(shell pkg-config --libs tcl)

cflags = -DHASHTABLE_COPY_KEYS $(TCL_CFLAGS)
ldlibs = $(TCL_LIBS)

# input source file (class name == source file basename)
tclpd.class.sources = tclpd.c
tclpd.class.sources += \
	hashtable.c \
	tcl_class.c \
	tcl_loader.c \
	tcl_proxyinlet.c \
	tcl_typemap.c \
	tcl_widgetbehavior.c \
	$(empty)
tclpd.class.sources += tcl_wrap.c

# all extra files to be included in binary distribution of the library
datafiles = \
	LICENSE.txt \
	README.txt \
	tclpd-help.pd \
	tclpd-meta.pd

datadirs = examples

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder


# create the tcl wrapper with 'swig'
tcl_wrap.c: tclpd.i tclpd.h Makefile
	swig -v -tcl -o tcl_wrap.c -I$(PDINCLUDEDIR) tclpd.i
# TODO: delete the tcl_wrap.c
