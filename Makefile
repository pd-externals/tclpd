# Makefile to build loader 'tclpd' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = tclpd

cflags = -std=c99
ldlibs =

###########################################################
# Tcl stuff
PKG_CONFIG = pkg-config
TCL_CFLAGS=$(shell $(PKG_CONFIG) --cflags tcl)
TCL_LIBS=$(shell $(PKG_CONFIG) --libs tcl)

## Dawrin
# TCL_CFLAGS = -I/Library/Frameworks/Tcl.framework/Headers
# TCL_LIBS = -framework Tcl

## MSW
# TCL_CFLAGS =
# TCL_LIBS = -ltcl86 tclpd.def

cflags += -DHASHTABLE_COPY_KEYS $(TCL_CFLAGS)
ldlibs += $(TCL_LIBS)

#
###########################################################

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
	tclpd.tcl \
	tclpd-help.pd \
	tclpd-meta.pd

datadirs = examples

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder


# create the tcl wrapper with 'swig'
tcl_wrap.c: tclpd.i tclpd.h Makefile
	swig -v -tcl -o tcl_wrap.c -I$(PDINCLUDEDIR) tclpd.i
clean-local:
	rm -f tcl_wrap.c
clean: clean-local
