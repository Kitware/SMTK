# Don't require GNU-standard files (Changelog, README, etc.)
AUTOMAKE_OPTIONS = foreign

# Things to build
noinst_LTLIBRARIES = libmoabverdict.la
         
SUBDIRS = 

# The directory in which to install headers
libmoabverdict_la_includedir = $(includedir)

# The list of source files, and any header files that do not need to be installed
libmoabverdict_la_SOURCES = \
        V_EdgeMetric.cpp \
        V_GaussIntegration.cpp \
        V_HexMetric.cpp \
        V_KnifeMetric.cpp \
        V_PyramidMetric.cpp \
        V_QuadMetric.cpp \
        V_TetMetric.cpp \
        V_TriMetric.cpp \
        VerdictVector.cpp \
        V_WedgeMetric.cpp \
        verdict_defines.hpp \
        VerdictVector.hpp \
        v_vector.h \
        V_GaussIntegration.hpp \
        VerdictWrapper.cpp 

# The list of header files which are to be installed
nobase_libmoabverdict_la_include_HEADERS = \
        moab/verdict.h \
        moab/VerdictWrapper.hpp

# Boilerplate stuff that doesn't depend on what the targets are
AM_CPPFLAGS = -DIS_BUILDING_MB -DSRCDIR=$(srcdir) \
               -I$(srcdir)/.. -I.. -I$(srcdir)/../io \
               -I$(srcdir)/../io/mhdf/include
