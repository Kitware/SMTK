# Don't require GNU-standard files (Changelog, README, etc.)
AUTOMAKE_OPTIONS = foreign subdir-objects

noinst_LTLIBRARIES = libmhdf.la
libmhdf_la_LIBADD = $(HDF5_LIBS)

libmhdf_la_SOURCES = include/mhdf.h \
                     src/adjacency.c \
                     src/connectivity.c \
                     src/file-desc.c \
                     src/file-handle.c \
                     src/file-handle.h \
                     src/file.c \
                     src/names-and-paths.h \
                     src/nodes.c \
                     src/sets.c \
                     src/status.c \
                     src/status.h \
                     src/tags.c \
                     src/util.c \
                     src/util.h

if ENABLE_h5mtools
  h5mtools = h5minfo h5mvalidate
else
  h5mtools =
endif

bin_PROGRAMS = $(h5mtools)

h5minfo_SOURCES = include/mhdf.h example/info.c
h5minfo_LDADD = libmhdf.la $(HDF5_LIBS)

h5mvalidate_SOURCES = example/validate.c
h5mvalidate_LDADD = libmhdf.la $(HDF5_LIBS)

DEFS = -I$(srcdir)/include -I$(top_builddir)/src

EXTRA_DIST = doc/Doxyfile example/hexes_to_gmsh.c
