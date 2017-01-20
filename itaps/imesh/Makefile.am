MESHDIR = $(abs_top_srcdir)/MeshFiles/unittest

DEFS += -DSRCDIR=$(srcdir) \
        -DMESHDIR=$(MESHDIR)

FCDEFS += $(FC_DEFINE)SRCDIR=$(srcdir) \
          $(FC_DEFINE)MESHDIR=$(MESHDIR) 

AM_CPPFLAGS  = -I$(srcdir)/.. \
               -I$(top_srcdir)/src \
               -I$(top_builddir)/src  \
               -I$(top_builddir)/itaps  \
               -I$(top_srcdir)/test

# stuff for c test program
check_PROGRAMS = testc_cbind MOAB_iMesh_unit_tests MOAB_iMesh_extensions_tests

if ENABLE_FORTRAN
  check_PROGRAMS += ScdMeshF90 ScdMeshF77 FindAdjacencyF90
endif

EXTRA_DIST = brick.vtk quad9.cub

libiMesh_la_SOURCES = \
	iMesh_MOAB.cpp \
	iMesh_MOAB.hpp \
	MBIter.hpp

libiMesh_la_include_HEADERS = iMesh.h \
                              iMesh_extensions.h \
                              iMesh_protos.h \
                              iMesh_f.h \
			      MBiMesh.hpp \
			      imesh_export.h

if PARALLEL
  libiMesh_la_SOURCES += iMeshP_MOAB.cpp
  libiMesh_la_include_HEADERS += iMeshP.h iMeshP_extensions.h iMeshP_f.h iMeshP_protos.h
  AM_CPPFLAGS += -I$(top_srcdir)/src/parallel -I$(top_srcdir)/src/parallel/moab

#  check_PROGRAMS += partest iMeshP_unit_tests moabtest
if HAVE_HDF5_PARALLEL
  check_PROGRAMS +=  partest
endif

if PARALLEL
  check_PROGRAMS +=  MOAB_iMeshP_unit_tests
endif 
#  moabtest_SOURCES = moabtest.F

#  check_PROGRAMS += ftest
#  ftest_SOURCES = ftest.F
#  ftest_DEPENDENCIES = libiMesh.la $(top_builddir)/libMOAB.la 
endif

TESTS = $(check_PROGRAMS)
LDADD = ${IMESH_LIBS}
TESTDEPS = libiMesh.la $(top_builddir)/src/libMOAB.la

testc_cbind_SOURCES = testc_cbind.cpp
testc_cbind_DEPENDENCIES = $(TESTDEPS)
partest_SOURCES = partest.cpp
partest_DEPENDENCIES = $(TESTDEPS)
MOAB_iMeshP_unit_tests_SOURCES = MOAB_iMeshP_unit_tests.cpp
MOAB_iMeshP_unit_tests_DEPENDENCIES = $(TESTDEPS)
MOAB_iMesh_unit_tests_SOURCES = MOAB_iMesh_unit_tests.cpp
MOAB_iMesh_unit_tests_DEPENDENCIES = $(TESTDEPS)
MOAB_iMesh_extensions_tests_SOURCES = MOAB_iMesh_extensions_tests.cpp
MOAB_iMesh_extensions_tests_DEPENDENCIES = $(TESTDEPS)

ScdMeshF90_SOURCES = ScdMeshF90.F90
ScdMeshF77_SOURCES = ScdMeshF77.F
FindAdjacencyF90_SOURCES = FindAdjacencyF90.F90
lib_LTLIBRARIES = libiMesh.la

libiMesh_la_includedir = $(includedir)

libiMesh_la_LIBADD = $(top_builddir)/src/libMOAB.la

# Automake doesn't seem to have a directory defined for
# platform-dependent data (or include) files. So put 
# in $(libdir).  Define a $(cfgdir) to get around automake's
# check that only libraries are going in $(libdir)
cfgdir = $(libdir)
cfg_DATA = iMesh-Defs.inc

# By default, moab.make will define these to $(srcdir).  We
# want to override that during the INSTALL of the file so
# that the correct values are set (e.g. if someone does 
# 'make prefix=/foo install', we don't know the correct install
# directory until we're doing the install.
install-data-hook:
	$(AM_V_at)echo "IMESH_INCLUDEDIR=${includedir}" >> $(DESTDIR)$(cfgdir)/iMesh-Defs.inc
	$(AM_V_at)echo "IMESH_INCLUDEDIR2=" >> $(DESTDIR)$(cfgdir)/iMesh-Defs.inc
	$(AM_V_at)echo "IMESH_LIBDIR=${libdir}" >> $(DESTDIR)$(cfgdir)/iMesh-Defs.inc
	$(AM_V_at)echo "MOAB_LIBDIR=${libdir}" >> $(DESTDIR)$(cfgdir)/iMesh-Defs.inc

FORTDEFS = $(FCDEFS) 
PPFCCOMPILE = $(FC) $(FORTDEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
    $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_FCFLAGS) $(FCFLAGS)
LTPPFCCOMPILE = $(LIBTOOL) $(AM_V_lt) --tag=FC $(AM_LIBTOOLFLAGS) \
    $(LIBTOOLFLAGS) --mode=compile $(FC) $(FCDEFS) \
    $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) \
    $(AM_FCFLAGS) $(FCFLAGS)
PPF77COMPILE = $(F77) $(FORTDEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
   $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_FFLAGS) $(FFLAGS)
LTPPF77COMPILE = $(LIBTOOL) $(AM_V_lt) --tag=F77 $(AM_LIBTOOLFLAGS) \
   $(LIBTOOLFLAGS) --mode=compile $(F77) $(FCDEFS) \
   $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) \
   $(AM_FFLAGS) $(FFLAGS)

include $(abs_top_builddir)/itaps/imesh/iMesh-Defs.inc

DISTCLEANFILES = imesh_export.h
