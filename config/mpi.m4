# Macros for MPI configuration.

# NOTE: The macro to enable/disable MPI support entirely is 
#       in compilers.m4, as it also affects the choice of compiler.
#       That code is expected to have set $enablempi to either 'yes',
#       'no' or the directory where MPI is installed BEFORE these 
#       macros are called.

# Options and configuration for mpiexec.  Sets
# the following variables:
#   USE_MPIEXEC - AM_CONDITIONAL, true if MPIEXEC is valid
#   MPIEXEC     - mpiexec binary
#   MPIEXEC_NP  - mpiexec CLI flag to specify number of procs (e.g. '-np')
#   NP          - number of procs to use during testing
AC_DEFUN([FATHOM_CONFIG_MPI_EXEC],[
  AC_ARG_VAR(MPIEXEC,[Program to use to run parallel tests (default: mpiexec or mpirun)])
  AC_ARG_VAR(MPIEXEC_NP,[Command line flag to specify number of processors to use (default: -np)])
  AC_ARG_VAR(NP,[Number of processors to on which to run parallel tests (default: 2)])
  if test "x$MPIEXEC" = "x"; then
    if test "x$enablempi" != "xyes"; then
      AC_CHECK_PROGS([MPIEXEC],[mpiexec mpirun],[NOTFOUND],[$COMPILERPATHS])
    else
      AC_CHECK_PROGS([MPIEXEC],[mpiexec mpirun],[NOTFOUND])
    fi
  fi
  if test "x$MPIEXEC_NP" = "x"; then
    MPIEXEC_NP="-np"
  fi
  if test "x$NP" = "x"; then
    NP=2
  fi
  AM_CONDITIONAL(USE_MPIEXEC, [test "xNOTFOUND" != "x$MPIEXEC"])
])

# Check for OpenMPI: #ifdef OPEN_MPI
# Check for MPICH:   #ifdef MPICH
# Check for MPI library type (MPICH, OpenMPI)
AC_DEFUN([FATHOM_CHECK_MPITYPE], [
  if test "x$enablempi" != "xno"; then
    AC_MSG_CHECKING([for known MPI family and version])
    MPIFAMILY="GENERIC"
    mpifile="$WITH_MPI/include/mpi.h"
    if (test -f $mpifile); then
      ismpich="`grep MPICH $mpifile`"
      if (test "x$ismpich" != "x"); then
        MPIFAMILY="MPICH"
      else
        isopenmpi="`grep OPEN_MPI $mpifile`"
        if (test "x$isopenmpi" != "x"); then
          MPIFAMILY="OPENMPI"
        fi # openmpi
      fi # mpich
    fi # mpifile
    if (test "$MPIFAMILY" != "GENERIC"); then
      MPI_VER=`grep "MPI_VERSION" $mpifile | sed -e "s/#define MPI_VERSION //g" | tr ' ' '\0'`
      MPITYPE="$MPIFAMILY-$MPI_VER"
    fi
    AC_MSG_RESULT([$MPITYPE])
    AC_SUBST(MPITYPE)
  fi #enablempi
])


# Check for MPI library and MPI.h
# Error if not found.
# Defines MPI_CXX_HELP_NEEDED to yes if, when parsed with a c++ compiler,
# mpi.h conflicts with the C standard library for the definition of SEEK_SET,
# etc. and defining MPICH_IGNORE_CXX_SEEK is not sufficient to work around
# the problem.
AC_DEFUN([FATHOM_CHECK_MPI], [
  if test "x$enablempi" != "xno"; then
    AC_LANG_PUSH([C])
    AC_CHECK_HEADER([mpi.h],[],[AC_MSG_ERROR([mpi.h not found or not working])])
    AC_LANG_POP([C])
    
    AC_MSG_CHECKING([If mpi.h conflicts with C headers in C++])
    AC_LANG_PUSH([C++])
    AC_PREPROC_IFELSE(
      [AC_LANG_PROGRAM([#include <stdio.h>
                        #include <mpi.h>],[])],
      [MPI_CXX_HELP_NEEDED=no; AC_MSG_RESULT([no])],
      [AC_PREPROC_IFELSE(
        [AC_LANG_PROGRAM([#include <stdio.h>
                          #define MPICH_IGNORE_CXX_SEEK
                          #include <mpi.h>],[])],
        [MPI_CXX_HELP_NEEDED=no; AC_MSG_RESULT([MPICH_IGNORE_CXX_SEEK])],
        [MPI_CXX_HELP_NEEDED=yes
         AC_MSG_RESULT([yes])

         AC_MSG_CHECKING([value of SEEK_SET])
         FATHOM_MACRO_VALUE([stdio.h],[SEEK_SET],[SEEK_SET],[16])
         AC_MSG_RESULT([$SEEK_SET])

         AC_MSG_CHECKING([value of SEEK_CUR])
         FATHOM_MACRO_VALUE([stdio.h],[SEEK_CUR],[SEEK_CUR],[16])
         AC_MSG_RESULT([$SEEK_CUR])

         AC_MSG_CHECKING([value of SEEK_END])
         FATHOM_MACRO_VALUE([stdio.h],[SEEK_END],[SEEK_END],[16])
         AC_MSG_RESULT([$SEEK_END])
       ])
    ])
    AC_LANG_POP([C++])
  fi
])

