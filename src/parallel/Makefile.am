# Don't require GNU-standard files (Changelog, README, etc.)
AUTOMAKE_OPTIONS = foreign

# Things to build
noinst_LTLIBRARIES = libMOABpar.la
         
# Some variables
AM_CPPFLAGS  = -DIS_BUILDING_MB -DSRCDIR=$(srcdir) -DLOCDIR=src/parallel/ \
               -I$(srcdir)/.. -I.. -I$(srcdir)/../io \
               -I$(srcdir)/../io/mhdf/include 

# The directory in which to install headers
libMOABpar_la_includedir = $(includedir)

# Run parallel tests in parallel
if USE_MPIEXEC
  OLD_ENVIRONMENT := ${TESTS_ENVIRONMENT}
  TESTS_ENVIRONMENT = ${MPIEXEC} ${MPIEXEC_NP} ${NP} ${OLD_ENVIRONMENT}
endif

libMOABpar_la_LIBADD = $(MPE_LIBS)

# Conditional sources
MOAB_PARALLEL_SRCS =
MOAB_PARALLEL_HDRS =
if PARALLEL
# The list of source files, and any header files that do not need to be installed
  MOAB_PARALLEL_SRCS += \
     ParallelComm.cpp \
     ProcConfig.cpp \
     ParallelData.cpp \
     ParallelMergeMesh.cpp \
     ReadParallel.cpp \
     ReadParallel.hpp \
     SharedSetData.cpp \
     SharedSetData.hpp \
     gs.cpp
     

# The list of header files which are to be installed
  MOAB_PARALLEL_HDRS += \
     moab/gs.hpp \
     moab_mpi.h \
     moab_mpi_config.h \
     moab/ParallelComm.hpp \
     moab/ParallelMergeMesh.hpp \
     moab/ProcConfig.hpp \
     moab/ParallelData.hpp \
     MBParallelConventions.h

if HAVE_HDF5_PARALLEL
#  libMOABpar_la_LIBADD = $(top_builddir)/mhdf/libmhdf.la
  MOAB_PARALLEL_SRCS += WriteHDF5Parallel.cpp WriteHDF5Parallel.hpp
endif

endif

# The list of source files, and any header files that do not need to be installed
libMOABpar_la_SOURCES = \
   $(MOAB_PARALLEL_SRCS)

# The list of header files which are to be installed
nobase_libMOABpar_la_include_HEADERS = \
  $(MOAB_PARALLEL_HDRS)
