BUILT_SOURCES = 

MESHDIR = $(abs_top_srcdir)/MeshFiles/unittest/fbigeom

AM_CPPFLAGS = -DSRCDIR=$(srcdir) \
              -DMESHDIR=$(MESHDIR) \
              -I$(top_srcdir)/src \
              -I$(top_builddir)/src \
              -I.. -I$(srcdir)/.. \
              -I$(top_srcdir)/itaps/imesh \
              -I$(top_builddir)/itaps/imesh 


lib_LTLIBRARIES = libFBiGeomMOAB.la

libFBiGeomMOAB_la_includedir = $(includedir)

libFBiGeomMOAB_la_LIBADD = $(top_builddir)/src/libMOAB.la

libFBiGeomMOAB_la_SOURCES = \
	FBiGeom_MOAB.hpp \
	FBiGeom_MOAB.cpp 

libFBiGeomMOAB_la_include_HEADERS = \
	FBiGeom.h \
	FBiGeom_protos.h \
	FBiGeom_f.h

# stuff for test program
check_PROGRAMS =
if HAVE_HDF5
  check_PROGRAMS += testgeom  testSmoothGeom testSmooth2

testgeom_SOURCES = testgeom.cc
testgeom_DEPENDENCIES = libFBiGeomMOAB.la \
                        $(top_builddir)/itaps/imesh/libiMesh.la \
                        $(top_builddir)/src/libMOAB.la 
testgeom_LDADD = libFBiGeomMOAB.la \
                 $(top_builddir)/itaps/imesh/libiMesh.la \
                 $(top_builddir)/src/libMOAB.la \
                 ${MOAB_CXX_LINKFLAGS} ${MOAB_CXX_LIBS}

testSmoothGeom_SOURCES = testSmooth.cc 
testSmoothGeom_DEPENDENCIES = libFBiGeomMOAB.la \
                        $(top_builddir)/itaps/imesh/libiMesh.la \
                        $(top_builddir)/src/libMOAB.la 
testSmoothGeom_LDADD = libFBiGeomMOAB.la \
                 $(top_builddir)/itaps/imesh/libiMesh.la \
                 $(top_builddir)/src/libMOAB.la \
                 ${MOAB_CXX_LINKFLAGS} ${MOAB_CXX_LIBS}
                 
testSmooth2_SOURCES = testSmooth2.cc
testSmooth2_DEPENDENCIES = libFBiGeomMOAB.la \
                        $(top_builddir)/itaps/imesh/libiMesh.la \
                        $(top_builddir)/src/libMOAB.la 
testSmooth2_LDADD = libFBiGeomMOAB.la \
                 $(top_builddir)/itaps/imesh/libiMesh.la \
                 $(top_builddir)/src/libMOAB.la \
                 ${MOAB_CXX_LINKFLAGS} ${MOAB_CXX_LIBS}

endif 
               
TESTS = $(check_PROGRAMS)

# Automake doesn't seem to have a directory defined for
# platform-dependent data (or include) files. So put 
# in $(libdir).  Define a $(cfgdir) to get around automake's
# check that only libraries are going in $(libdir)
cfgdir = $(libdir)
cfg_DATA = FBiGeom-Defs.inc

# By default, moab.make will define these to $(srcdir).  We
# want to override that during the INSTALL of the file so
# that the correct values are set (e.g. if someone does 
# 'make prefix=/foo install', we don't know the correct install
# directory until we're doing the install.
install-data-hook:
	$(AM_V_at)echo "FBIGEOM_INCLUDEDIR=${includedir}" >> $(DESTDIR)$(cfgdir)/FBiGeom-Defs.inc
	$(AM_V_at)echo "FBIGEOM_INCLUDEDIR2=" >> $(DESTDIR)$(cfgdir)/FBiGeom-Defs.inc
	$(AM_V_at)echo "FBIGEOM_LIBDIR=${libdir}" >> $(DESTDIR)$(cfgdir)/FBiGeom-Defs.inc
	$(AM_V_at)echo "MOAB_LIBDIR=${libdir}" >> $(DESTDIR)$(cfgdir)/FBiGeom-Defs.inc
