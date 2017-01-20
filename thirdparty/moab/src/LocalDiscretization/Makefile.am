# Don't require GNU-standard files (Changelog, README, etc.)
AUTOMAKE_OPTIONS = foreign

# Things to build
noinst_LTLIBRARIES = libLocalDiscretization.la
         
# Some variables
AM_CPPFLAGS  = -DIS_BUILDING_MB -DSRCDIR=$(srcdir) \
               -I$(srcdir)/.. -I.. -I$(srcdir)/../moab -I$(srcdir)/moab

# The directory in which to install headers
libLocalDiscretization_la_includedir = $(includedir)

# Conditional sources
MOAB_LOCALDISCR_SRCS = \
	ElemEvaluator.cpp \
	LinearHex.cpp \
	LinearQuad.cpp \
	LinearTet.cpp \
	LinearTri.cpp \
	QuadraticHex.cpp

MOAB_LOCALDISCR_HDRS = \
	moab/ElemEvaluator.hpp \
	moab/LinearHex.hpp \
	moab/LinearQuad.hpp \
	moab/LinearTet.hpp \
	moab/LinearTri.hpp \
	moab/QuadraticHex.hpp

# The list of source files, and any header files that do not need to be installed
libLocalDiscretization_la_SOURCES = \
   $(MOAB_LOCALDISCR_SRCS)

# The list of header files which are to be installed
nobase_libLocalDiscretization_la_include_HEADERS = \
  $(MOAB_LOCALDISCR_HDRS)
