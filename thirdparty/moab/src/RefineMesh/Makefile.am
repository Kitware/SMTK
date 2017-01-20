# Don't require GNU-standard files (Changelog, README, etc.)
AUTOMAKE_OPTIONS = foreign

# Things to build
noinst_LTLIBRARIES = libRefineMesh.la         
AM_CPPFLAGS =

# Some variables
AM_CPPFLAGS += -DIS_BUILDING_MB -DSRCDIR=$(srcdir) -DLOCDIR=src/RefineMesh/ \
               -I$(srcdir)/..  -I.. -I$(srcdir)/../moab -I$(srcdir)/moab \
	       -I$(srcdir)/../parallel

# The directory in which to install headers
libRefineMesh_la_includedir = $(includedir)

# Conditional sources
MOAB_LOCALDISCR_SRCS = \
	NestedRefine.cpp 

MOAB_LOCALDISCR_HDRS = \
	moab/NestedRefine.hpp \
	moab/Templates.hpp 

# The list of source files, and any header files that do not need to be installed
libRefineMesh_la_SOURCES = \
   $(MOAB_LOCALDISCR_SRCS)

# The list of header files which are to be installed
nobase_libRefineMesh_la_include_HEADERS = \
  $(MOAB_LOCALDISCR_HDRS)
