#######################################################################################
# Get libtool configuration variable
# Arguments:
#  libtool config tag (e.g CXX)
#  libtool variable name
#  variable in which to store result
#######################################################################################
AC_DEFUN([FATHOM_LIBTOOL_VAR], [
  $3=`./libtool --tag=$1 --config | sed -e 's/^$2=//p' -e 'd' | tr -d '"\n'`
])


#######################################################################################
# Check if the C++ compiler works.
# Arguments: action on success and action on failure
#######################################################################################
AC_DEFUN([FATHOM_CHECK_CXX_WORKS], [
  AC_LANG_PUSH([C++])
  AC_MSG_CHECKING([if $CXX works])
  AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM( [class Cl { int i; public: Cl(int x) : i(x) {} };] [Cl x(5);])],
   [AC_COMPILE_IFELSE( 
      [AC_LANG_PROGRAM( [],
[#ifdef __cplusplus
   choke me
 #endif])],
      [AC_MSG_RESULT([no (accepted invalid input)]); $2], [AC_MSG_RESULT([yes]); $1])],
    [AC_MSG_RESULT([no (failed on trival valid C++ source)]); $2])

  AC_LANG_POP([C++])
])


AC_DEFUN([FATHOM_CHECK_MPI_ENABLED], [
  AC_AFTER([$0], [AC_PROG_CC])dnl
  AC_REQUIRE([AC_PROG_CC])dnl
  AC_LANG_PUSH([C])
  AC_MSG_CHECKING([if $CC works with MPI])
  AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM( [#include <mpi.h>]
   [int i=MPI_SUCCESS;] )],
   [AC_MSG_RESULT([yes]); $1=yes],
    [AC_MSG_RESULT([no]); $1=no])
  AC_LANG_POP([C])
])

########## Helper function for FATHOM_CHECK_COMPILERS #############
# args: compiler variable, compiler list, path
AC_DEFUN([FATHOM_SET_MPI_COMPILER], [
  if test "x" = "x${$1}"; then
    if test "x" = "x$3"; then
      AC_CHECK_PROGS([$1], [$2], [false])
    else
      $1=false
      for prog in $2 ; do
        if test -x "$3/$prog"; then
          $1="$3/$prog"
          AC_SUBST($1)
          export $1
          break
        fi
      done
    fi
    
    if test "x${$1}" = "xfalse"; then
      AC_MSG_ERROR([Cannot find MPI compiler.  Try specifying \$$1])
    fi
  fi
])

#######################################################################################
# Implement checks for C and C++ compilers, with all corresponding options
#
# Sets the following variables:
#  CPP      - The C preprocessor
#  CC       - The C compiler
#  CXX      - The C++ compiler
#  FC       - The Fortran compiler
#  CFLAGS   - C compiler flags
#  CXXFLAGS - C++ compiler flags
#  enablempi- 'yes' if parallel support, 'no' otherwise
#  WITH_MPI - if enablempi=yes, then path to MPI installation
#
# Arguments:  three strings that msut be either "yes" or "no".
#             - test for C compiler
#             - test for C++ compiler
#             - test for Fortran compiler
#######################################################################################
AC_DEFUN([FATHOM_CHECK_COMPILERS], [

CHECK_CC="$1"
CHECK_CXX="$2"
CHECK_FC="$3"

# If not specified or invalid value, change to yes.
test "xno" = "x$CHECK_CC" || CHECK_CC=yes 
test "xno" = "x$CHECK_CXX" || CHECK_CXX=yes 
test "xno" = "x$CHECK_FC" || CHECK_FC=yes 

  # Save these before calling AC_PROG_CC or AC_PROG_CXX
  # because those macros will modify them, and we want
  # the original user values, not the autoconf defaults.
USER_CXXFLAGS="$CXXFLAGS"
USER_CFLAGS="$CFLAGS"

  # Save these before calling AC_PROG_FC or AC_PROG_F77
  # because those macros will modify them, and we want
  # the original user values, not the autoconf defaults.
USER_FCFLAGS="$FCFLAGS"
USER_FFLAGS="$FFLAGS"

# Check for Parallel
# Need to check this early so we can look for the correct compiler
AC_ARG_WITH( [mpi], AS_HELP_STRING([[--with-mpi@<:@=DIR@:>@]], [Enable parallel support]),
             [WITH_MPI=$withval; enablempi=yes],[enablempi=no;WITH_MPI=$MPI_DIR] )

if test "xno" != "x$enablempi"; then

  CC_LIST="mpixlc mpicc mpcc"
  CXX_LIST="mpixlcxx mpicxx mpiCC mpCC"
  FC_LIST="mpixlf95 mpixlf90 mpif90"
  F77_LIST="mpixlf77 mpif77"
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-mpi=\"${WITH_MPI}\""

fi

CC_LIST="$CC $CC_LIST gcc icc clang"
CXX_LIST="$CXX $CXX_LIST g++ icpc clang++"
FC_LIST="$FC $FC_LIST gfortran ifort pgf90 nag xlf"
F77_LIST="$F77 $F77_LIST $FC gfortran ifort pgf77 nag xlf g77 f77"

COMPILERPATHS=""
if test "xno" != "x$enablempi"; then
  COMPILERPATHS="${WITH_MPI}/bin"
fi

FATHOM_SET_MPI_COMPILER([CC],  [$CC_LIST], [$COMPILERPATHS])
FATHOM_SET_MPI_COMPILER([CXX],[$CXX_LIST],[$COMPILERPATHS])

# FATHOM_CHECK_MPI_ENABLED([compiler_supports_mpi])
# AC_SUBST(compiler_supports_mpi)

# C support
AC_PROG_CC
AC_PROG_CPP

# C++ support
AC_PROG_CXX
AC_PROG_CXXCPP

# Fortran support
if (test "x$CHECK_FC" != "xno"); then
  FATHOM_SET_MPI_COMPILER([F77],[$F77_LIST],[$COMPILERPATHS])
  AC_PROG_F77
  FATHOM_SET_MPI_COMPILER([FC],  [$FC_LIST],[$COMPILERPATHS])
  AC_PROG_FC
fi

]) # FATHOM_CHECK_COMPILERS



#######################################################################################
# Implement checks for C and C++ compiler options
#
#  CFLAGS   - C compiler flags
#  CXXFLAGS - C++ compiler flags
#  DEBUG - yes if specified, no otherwise
#
#######################################################################################
AC_DEFUN([FATHOM_COMPILER_FLAGS], [

CHECK_CC="$1"
CHECK_CXX="$2"
CHECK_FC="$3"

# If not specified or invalid value, change to yes.
test "xno" = "x$CHECK_CC" || CHECK_CC=yes 
test "xno" = "x$CHECK_CXX" || CHECK_CXX=yes 
test "xno" = "x$CHECK_FC" || CHECK_FC=yes 

# Check for debug flags
AC_ARG_ENABLE( debug, AS_HELP_STRING([--enable-debug],[Debug symbols (-g)]),
               [enable_debug=$enableval], [enable_debug="no"] )
AC_ARG_ENABLE( optimize, AS_HELP_STRING([--enable-optimize],[Compile optimized (-O2)]),
               [enable_optimize=$enableval; enable_cxx_optimize=$enableval; enable_cc_optimize=$enableval; enable_fc_optimize=$enableval;],
               [enable_optimize=""; enable_cxx_optimize="no"; enable_cc_optimize="no"; enable_fc_optimize="no";	]
             )

EXTRA_PGI_ONLY_FCFLAGS="-Mfree"
if (test "x$enable_debug" != "xno"); then # debug flags
# GNU
EXTRA_GNU_CXXFLAGS="-Wall -Wno-long-long -pipe -pedantic -Wshadow -Wunused-parameter -Wpointer-arith -Wformat -Wformat-security -Wextra -Wno-variadic-macros -Wno-unknown-pragmas"
EXTRA_GNU_FCFLAGS="-pipe -pedantic -ffree-line-length-0"
# CLANG
EXTRA_CLANG_CXXFLAGS="$EXTRA_GNU_CXXFLAGS"
EXTRA_CLANG_FCFLAGS="$EXTRA_GNU_FCFLAGS"
# Intel
EXTRA_INTEL_CXXFLAGS="-pipe -C"
EXTRA_INTEL_FCFLAGS="-C"
# PGI
EXTRA_PGI_CXXFLAGS="-traceback --diag_suppress 236 --diag_suppress=unrecognized_gcc_pragma -C"
EXTRA_PGI_FCFLAGS="-traceback -Mbounds -Ktrap=inv,divz,ovf"
# XLC
EXTRA_BG_CXXFLAGS="-qarch=qp -qpic=large -qdebug=except"
EXTRA_BG_FCFLAGS="-qarch=qp -qpic=large -qdebug=except"
fi

if (test "x$enable_cxx_optimize" != "xno"); then  # optimization flags
#GNU
EXTRA_GNU_CXXFLAGS="$EXTRA_GNU_CXXFLAGS -fprefetch-loop-arrays -finline-functions -march=native"
EXTRA_GNU_FCFLAGS="$EXTRA_GNU_FCFLAGS -ffree-line-length-0 -finline-functions"
#CLANG
EXTRA_CLANG_CXXFLAGS="$EXTRA_CLANG_CXXFLAGS -march=native"
EXTRA_CLANG_FCFLAGS="$EXTRA_CLANG_FCFLAGS -ffree-line-length-0 -finline-functions"
# Intel
EXTRA_INTEL_CXXFLAGS="$EXTRA_INTEL_CXXFLAGS -xHost -ip -no-prec-div" # -fast
EXTRA_INTEL_FCFLAGS="$EXTRA_INTEL_FCFLAGS -xHost -ip -no-prec-div" # -fast
# PGI
EXTRA_PGI_CXXFLAGS="$EXTRA_PGI_CXXFLAGS -fast"
EXTRA_PGI_FCFLAGS="$EXTRA_PGI_FCFLAGS -fast"
# XLC
EXTRA_BG_CXXFLAGS="$EXTRA_BG_CXXFLAGS -qarch=qp -qtune=auto -qpic=large -qenablevmx"
EXTRA_BG_FCFLAGS="$EXTRA_BG_FCFLAGS -qarch=qp -qtune=auto -qpic=large -qenablevmx"
fi

if (test "x$GXX" = "xyes"); then # G++ specific flags
  if (test "x$enable_debug" != "xno"); then
    EXTRA_GNU_ONLY_CXXFLAGS="$EXTRA_GNU_CXXFLAGS"
    EXTRA_GNU_ONLY_FCFLAGS="$EXTRA_GNU_FCFLAGS"
  else
    EXTRA_GNU_ONLY_CXXFLAGS="$EXTRA_GNU_CXXFLAGS -ftree-vectorize"
    EXTRA_GNU_ONLY_FCFLAGS="$EXTRA_GNU_FCFLAGS -ftree-vectorize"
  fi
fi

# this is just a test comment
if test "xno" != "x$CHECK_CC"; then
  FATHOM_CC_FLAGS
fi
if test "xno" != "x$CHECK_CXX"; then
  FATHOM_CXX_FLAGS
fi

# Try to determine compiler-specific flags.  This must be done
# before setting up libtool so that it can override libtool settings.
CFLAGS="$USER_CFLAGS $FATHOM_CC_SPECIAL"
CXXFLAGS="$USER_CXXFLAGS $FATHOM_CXX_SPECIAL"
FFLAGS="$USER_FFLAGS $FATHOM_F77_SPECIAL"
FCFLAGS="$USER_FCFLAGS $FATHOM_FC_SPECIAL"
FLIBS=""
FCLIBS=""

# Do enable_optimize by default, unless user has specified
# custom CXXFLAGS or CFLAGS
DEBUG=no
if (test "x$enable_debug" != "xyes"); then
  if (test "x$enable_optimize" == "x"); then
    enable_cxx_optimize=yes
    enable_cc_optimize=yes
    enable_fc_optimize=yes
  fi
fi
enable_f77_optimize=$enable_fc_optimize

# Choose compiler flags from CLI args
if test "xyes" = "x$enable_debug"; then
  DEBUG=yes
  CXXFLAGS="$CXXFLAGS -g"
  CFLAGS="$CFLAGS -g"
  if (test "x$CHECK_FC" != "xno"); then
    FCFLAGS="$FCFLAGS -g"
    FFLAGS="$FFLAGS -g"
  fi
  # Add -fstack-protector-all option for g++ in debug mode
  if (test "x$cxx_compiler" = "xGNU"); then
    CXXFLAGS="$CXXFLAGS -fstack-protector-all"
    CFLAGS="$CFLAGS -fstack-protector-all"
    LDFLAGS="$LDFLAGS -fstack-protector-all"
    if (test "x$CHECK_FC" != "xno"); then
      FCFLAGS="$FCFLAGS -fstack-protector-all"
      FFLAGS="$FFLAGS -fstack-protector-all"
    fi
  fi
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-debug=yes"
else
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-debug=no"
fi
if (test "xno" != "x$enable_cxx_optimize"); then
  CXXFLAGS="$CXXFLAGS -O2 -DNDEBUG"
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-optimize=yes"
else
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-optimize=no"
fi
if (test "xno" != "x$enable_cc_optimize"); then
  CFLAGS="$CFLAGS -O2 -DNDEBUG"
fi
if (test "x$ENABLE_FORTRAN" != "xno"); then
  if (test "xno" != "x$enable_fc_optimize"); then
    FCFLAGS="$FCFLAGS -O2"
  fi
  if (test "xno" != "x$enable_f77_optimize"); then
    FFLAGS="$FFLAGS -O2"
  fi
  AC_FC_PP_DEFINE
fi

# Check for 32/64 bit.
# This requires FATHOM_CXX_FLAGS and FATHOM_CC_FLAGS to have been called first
AC_ARG_ENABLE(32bit, AS_HELP_STRING([--enable-32bit],[Force 32-bit objects]),
[
  if (test "xyes" != "x$enableval" && test "xno" != "x$enableval"); then
    AC_MSG_ERROR([Unknown argument --enable-32bit=$enableval])
  fi
  if (test "xno" != "x$enableval"); then
    if test "x" = "x$FATHOM_CXX_32BIT"; then
      AC_MSG_ERROR([Don't know how to force 32-bit C++ on this platform.  Try setting CXXFLAGS manually])
    elif test "x" = "x$FATHOM_CC_32BIT"; then
      AC_MSG_ERROR([Don't know how to force 32-bit C on this platform.  Try setting CFLAGS manually])
    fi
  fi
  enable_32bit=$enableval
], [enable_32bit=no])

# This requires FATHOM_CXX_FLAGS and FATHOM_CC_FLAGS to have been called first
AC_ARG_ENABLE(64bit, AS_HELP_STRING([--enable-64bit],[Force 64-bit objects]),
[
  if (test "xyes" != "x$enableval" && test "xno" != "x$enableval"); then
    AC_MSG_ERROR([Unknown argument --enable-64bit=$enableval])
  fi
  if (test "xno" != "x$enableval"); then
    if test "x" = "x$FATHOM_CXX_64BIT"; then
      AC_MSG_ERROR([Don't know how to force 64-bit C++ on this platform.  Try setting CXXFLAGS manually])
    elif test "x" = "x$FATHOM_CC_64BIT"; then
      AC_MSG_ERROR([Don't know how to force 64-bit C on this platform.  Try setting CFLAGS manually])
    elif test "xyes" = "x$enable_32bit"; then
      AC_MSG_ERROR([Cannot do both --enable-32bit and --enable-64bit])
    fi
  fi
  enable_64bit=$enableval
], [enable_64bit=no])

if (test "xno" != "x$enable_32bit"); then
  CXXFLAGS="$CXXFLAGS $FATHOM_CXX_32BIT"
  CFLAGS="$CFLAGS $FATHOM_CC_32BIT"
  LDFLAGS="$LDFLAGS $FATHOM_CC_32BIT"
fi
if (test "xno" != "x$enable_64bit"); then
  CXXFLAGS="$CXXFLAGS $FATHOM_CXX_64BIT"
  CFLAGS="$CFLAGS $FATHOM_CC_64BIT"
  LDFLAGS="$LDFLAGS $FATHOM_CC_64BIT"
fi
# Distcheck flags for 32-bit and 64-bit builds
DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-32bit=$enable_32bit --enable-64bit=$enable_64bit"

# Check if platform is BlueGene
AC_MSG_CHECKING([if platform is IBM BlueGene])
FATHOM_TRY_COMPILER_DEFINE([__bg__],
  [MB_DEFS="$MB_DEFS -DBLUEGENE"
   MB_BLUEGENE_CONF=yes],
  [MB_BLUEGENE_CONF=no])
AC_MSG_RESULT([$MB_BLUEGENE_CONF])

# Special overrides flags for BG/Q
if (test "x$enable_static" != "xno" && test "x$MB_BLUEGENE_CONF" != "xno" && test "x$GXX" != "xyes"); then
  LDFLAGS="$LDFLAGS -qnostaticlink=libgcc"
fi

# Check if we are using new Darwin kernels with Clang -- needs libc++ instead of libstdc++
if (test "x$ENABLE_FORTRAN" != "xno" && test "x$CHECK_FC" != "xno"); then

  # check how to link against C++ runtime for fortran programs correctly
  AC_F77_MAIN
  AC_FC_MAIN
  FAC_FC_WRAPPERS
  fcxxlinkage=no

  # Check if we are on IBM ANL BG/Q system
  # Default location on Vesta/Mira: /soft/compilers/ibmcmp-feb2015/vacpp/bg/12.1/bglib64/libibmc++.a
  # Other location for stdc++ libraries: /bgsys/drivers/ppcfloor/gnu-linux/powerpc64-bgq-linux/lib/libstdc++.a
  case "`hostname`" in
    *vesta*)  LIBS="$LIBS /soft/compilers/ibmcmp-feb2015/vacpp/bg/12.1/bglib64/libibmc++.a"; fcxxlinkage=yes ;;
    *mira*)  LIBS="$LIBS /soft/compilers/ibmcmp-feb2015/vacpp/bg/12.1/bglib64/libibmc++.a"; fcxxlinkage=yes ;;
  esac

  if (test "$cxx_compiler" == "Intel"); then
    my_save_ldflags="$LDFLAGS"
    LDFLAGS="$LDFLAGS -cxxlib"
    AC_MSG_CHECKING([whether $FC supports -cxxlib])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
        [AC_MSG_RESULT([yes])]
        [fcxxlinkage=yes; FFLAGS="$FFLAGS -cxxlib"; FCFLAGS="$FCFLAGS -cxxlib"; FLIBS="$FLIBS -cxxlib"; FCLIBS="$FCLIBS -cxxlib"],
        [AC_MSG_RESULT([no])]
    )
    LDFLAGS="$my_save_ldflags"
  elif (test "$cxx_compiler" == "PortlandGroup"); then
    my_save_ldflags="$LDFLAGS"
    LDFLAGS="$LDFLAGS -pgc++libs"
    AC_MSG_CHECKING([whether $FC supports -pgc++libs])
    AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
        [AC_MSG_RESULT([yes])]
        [fcxxlinkage=yes; FLIBS="$FLIBS -pgc++libs"; FCLIBS="$FCLIBS -pgc++libs"],
        [AC_MSG_RESULT([no])]
    )
    LDFLAGS="$my_save_ldflags"
    if (test "$fcxxlinkage" != "yes"); then
      LDFLAGS="$LDFLAGS -pgcpplibs -lstd -lC"
      AC_MSG_CHECKING([whether $FC supports -pgcpplibs -lstd -lC])
      AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
          [AC_MSG_RESULT([yes])]
          [fcxxlinkage=yes; FLIBS="$FLIBS -pgcpplibs -lstd -lC"; FCLIBS="$FCLIBS -pgcpplibs -lstd -lC"],
          [AC_MSG_RESULT([no])]
      )
      LDFLAGS="$my_save_ldflags"
    fi
  else

    if (test "$fcxxlinkage" != "yes"); then
      # With Clang compilers, we specifically look at two cases: OSX and Ubuntu
      # On OSX (Mavericks and beyond), -lc++ provides the standard C++ library definitions
      # But on Ubuntu, we need -lstdc++, so "fcxxlinkage" will not be set to "yes" below
      if (test "$cc_compiler" == "Clang"); then
        my_save_ldflags="$LDFLAGS"
        LDFLAGS="$LDFLAGS -lc++"
        AC_MSG_CHECKING([whether $FC supports -stdlib=libc++])
        AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
            [AC_MSG_RESULT([yes])]
            [fcxxlinkage=yes; FLIBS="$FLIBS -lc++"; FCLIBS="$FCLIBS -lc++"],
            [AC_MSG_RESULT([no])]
        )
        LDFLAGS="$my_save_ldflags"
      fi

      # GNU and other non-intel compilers will use the standard -lstdc++ linkage
      # This case also includes the Ubuntu+Clang combination as mentioned before
      if (test "$cxx_compiler" != "Clang" || test "$fcxxlinkage" != "yes"); then
        my_save_ldflags="$LDFLAGS"
        LDFLAGS="$LDFLAGS -lstdc++"
        AC_MSG_CHECKING([whether $FC supports -stdlib=libstdc++])
        AC_LINK_IFELSE([AC_LANG_PROGRAM([])],
            [AC_MSG_RESULT([yes])]
            [fcxxlinkage=yes; FLIBS="$FLIBS -lstdc++"; FCLIBS="$FCLIBS -lstdc++"],
            [AC_MSG_RESULT([no])]
        )
        LDFLAGS="$my_save_ldflags"
      fi
    fi

  fi

  if (test "$fcxxlinkage" != "yes"); then
    AC_MSG_WARN([Unrecognized C++ linkage for C/Fortran programs])
  fi

fi

AC_SUBST(FLIBS)
AC_SUBST(FCLIBS)

]) # FATHOM_COMPILER_FLAGS

#######################################################################################
# *******************************************************************************
# **************************** INTERNAL STUFF ***********************************
# *******************************************************************************
#######################################################################################

#################################################################################
# Check if the compiler defines a specific preprocessor macro
# Arguments:
#  - preprocessor define to check for
#  - action upon success
#  - action upon failure
#################################################################################
AC_DEFUN([FATHOM_TRY_COMPILER_DEFINE], [
 AC_COMPILE_IFELSE([
 AC_LANG_PROGRAM( [[#ifndef $1
   choke me
 #endif]], []) ],
 [$2],[$3])
])

#################################################################################
# Check if the compiler defines a specific preprocessor macro with an integer
# value greater than or equal to the passed value
# Arguments:
#  - preprocessor define to check for
#  - numeric value to test
#  - action upon success
#  - action upon failure
#################################################################################
AC_DEFUN([FATHOM_TRY_COMPILER_DEFINE_GE], [
 AC_COMPILE_IFELSE([
 AC_LANG_PROGRAM( [[#if !defined($1) || $1 < $2
   choke me
 #endif]], []) ],
 [$3],[$4])
])

# FAC_FC_NAME_MANGLING
# ---------------------
# Test for the name mangling scheme used by the Fortran compiler.
#
# Sets ac_cv_{f77,fc}_mangling. The value contains three fields, separated
# by commas:
#
# lower case / upper case:
#    case translation of the Fortran symbols
# underscore / no underscore:
#    whether the compiler appends "_" to symbol names
# extra underscore / no extra underscore:
#    whether the compiler appends an extra "_" to symbol names already
#    containing at least one underscore
#
AC_DEFUN([FAC_FC_NAME_MANGLING],
[_AC_FORTRAN_ASSERT()dnl
AC_CACHE_CHECK([for Fortran name-mangling scheme],
               ac_cv_[]_AC_LANG_ABBREV[]_mangling,
[AC_COMPILE_IFELSE(
[      subroutine foobar()
      return
      end
      subroutine foo_bar()
      return
      end],
[mv conftest.$ac_objext cfortran_test.$ac_objext

  ac_save_LIBS=$LIBS
  LIBS="cfortran_test.$ac_objext $LIBS $[]_AC_LANG_PREFIX[]LIBS $ac_cv_fc_libs"

  AC_LANG_PUSH(C)dnl
  ac_success=no
  for ac_foobar in foobar FOOBAR; do
    for ac_underscore in "" "_"; do
      ac_func="$ac_foobar$ac_underscore"
      AC_LINK_IFELSE([AC_LANG_CALL([], [$ac_func])],
         [ac_success=yes; break 2])
    done
  done
  AC_LANG_POP(C)dnl

  if test "$ac_success" = "yes"; then
     case $ac_foobar in
  foobar)
     ac_case=lower
     ac_foo_bar=foo_bar
     ;;
  FOOBAR)
     ac_case=upper
     ac_foo_bar=FOO_BAR
     ;;
     esac

     AC_LANG_PUSH(C)dnl
     ac_success_extra=no
     for ac_extra in "" "_"; do
  ac_func="$ac_foo_bar$ac_underscore$ac_extra"
  AC_LINK_IFELSE([AC_LANG_CALL([], [$ac_func])],
           [ac_success_extra=yes; break])
     done
     AC_LANG_POP(C)dnl

     if test "$ac_success_extra" = "yes"; then
  ac_cv_[]_AC_LANG_ABBREV[]_mangling="$ac_case case"
        if test -z "$ac_underscore"; then
           ac_cv_[]_AC_LANG_ABBREV[]_mangling="$ac_cv_[]_AC_LANG_ABBREV[]_mangling, no underscore"
  else
           ac_cv_[]_AC_LANG_ABBREV[]_mangling="$ac_cv_[]_AC_LANG_ABBREV[]_mangling, underscore"
        fi
        if test -z "$ac_extra"; then
           ac_cv_[]_AC_LANG_ABBREV[]_mangling="$ac_cv_[]_AC_LANG_ABBREV[]_mangling, no extra underscore"
  else
           ac_cv_[]_AC_LANG_ABBREV[]_mangling="$ac_cv_[]_AC_LANG_ABBREV[]_mangling, extra underscore"
        fi
      else
  ac_cv_[]_AC_LANG_ABBREV[]_mangling="unknown"
      fi
  else
     ac_cv_[]_AC_LANG_ABBREV[]_mangling="unknown"
  fi

  LIBS=$ac_save_LIBS
  rm -f cfortran_test* conftest*],
  [AC_MSG_FAILURE([cannot compile a simple Fortran program])])
])
])# FAC_FC_NAME_MANGLING

# FAC_FC_WRAPPERS
# ---------------
# Defines C macros {F77,FC}_FUNC(name,NAME) and {F77,FC}_FUNC_(name,NAME) to
# properly mangle the names of C identifiers, and C identifiers with
# underscores, respectively, so that they match the name mangling
# scheme used by the Fortran compiler.
AC_DEFUN([FAC_FC_WRAPPERS],
[
AC_LANG_PUSH([Fortran])
_AC_FORTRAN_ASSERT()dnl
FAC_FC_NAME_MANGLING
AH_TEMPLATE(_AC_FC[_FUNC],
    [Define to a macro mangling the given C identifier (in lower and upper
     case), which must not contain underscores, for linking with Fortran.])dnl
AH_TEMPLATE(_AC_FC[_FUNC_],
    [As ]_AC_FC[_FUNC, but for C identifiers containing underscores.])dnl
case $ac_cv_[]_AC_LANG_ABBREV[]_mangling in
  "lower case, no underscore, no extra underscore")
          AC_DEFINE(_AC_FC[_FUNC(name,NAME)],  [name])
          AC_DEFINE(_AC_FC[_FUNC_(name,NAME)], [name]) ;;
  "lower case, no underscore, extra underscore")
          AC_DEFINE(_AC_FC[_FUNC(name,NAME)],  [name])
          AC_DEFINE(_AC_FC[_FUNC_(name,NAME)], [name ## _]) ;;
  "lower case, underscore, no extra underscore")
          AC_DEFINE(_AC_FC[_FUNC(name,NAME)],  [name ## _])
          AC_DEFINE(_AC_FC[_FUNC_(name,NAME)], [name ## _]) ;;
  "lower case, underscore, extra underscore")
          AC_DEFINE(_AC_FC[_FUNC(name,NAME)],  [name ## _])
          AC_DEFINE(_AC_FC[_FUNC_(name,NAME)], [name ## __]) ;;
  "upper case, no underscore, no extra underscore")
          AC_DEFINE(_AC_FC[_FUNC(name,NAME)],  [NAME])
          AC_DEFINE(_AC_FC[_FUNC_(name,NAME)], [NAME]) ;;
  "upper case, no underscore, extra underscore")
          AC_DEFINE(_AC_FC[_FUNC(name,NAME)],  [NAME])
          AC_DEFINE(_AC_FC[_FUNC_(name,NAME)], [NAME ## _]) ;;
  "upper case, underscore, no extra underscore")
          AC_DEFINE(_AC_FC[_FUNC(name,NAME)],  [NAME ## _])
          AC_DEFINE(_AC_FC[_FUNC_(name,NAME)], [NAME ## _]) ;;
  "upper case, underscore, extra underscore")
          AC_DEFINE(_AC_FC[_FUNC(name,NAME)],  [NAME ## _])
          AC_DEFINE(_AC_FC[_FUNC_(name,NAME)], [NAME ## __]) ;;
  *)
          AC_MSG_WARN([unknown Fortran name-mangling scheme])
          ;;
esac
AC_LANG_POP([Fortran])
])# FAC_FC_WRAPPERS

#######################################################################################
# Check for compiler-specific flags.
# Sets the following environmental variables:
#   FATHOM_CXX_SPECIAL : Any compiler-specific flags which must be specified
#   FATHOM_CXX_32BIT   : Flag to force compilation of 32-bit code
#   FATHOM_CXX_64BIT   : Flag to force compilation of 64-bit code
#######################################################################################
AC_DEFUN([FATHOM_CXX_FLAGS], [
AC_LANG_PUSH([C++])

# Detect compiler 
AC_MSG_CHECKING([for known c++ compilers])
# Autoconf does G++ for us
if test x$GXX = xyes; then
  cxx_compiler=GNU
  # Intel and Clang claims to be GCC, check for it here
  FATHOM_TRY_COMPILER_DEFINE([__INTEL_COMPILER],[cxx_compiler=Intel])
  FATHOM_TRY_COMPILER_DEFINE([__clang__],[cxx_compiler=Clang])
  FATHOM_TRY_COMPILER_DEFINE([__PGI],[cxx_compiler=PortlandGroup])
# Search for other compiler types
# For efficiency, limit checks to relevant OSs
else
  cxx_compiler=unknown
  case "$target_os" in
    aix*)
      FATHOM_TRY_COMPILER_DEFINE_GE([__IBMCPP__],[800],[cxx_compiler=VisualAge8],
        [FATHOM_TRY_COMPILER_DEFINE([__IBMCPP__],[cxx_compiler=VisualAge])])
      ;;
    solaris*|sunos*)
      FATHOM_TRY_COMPILER_DEFINE([__SUNPRO_CC],[cxx_compiler=SunWorkshop])
      ;;
    darwin*)
      FATHOM_TRY_COMPILER_DEFINE([__clang__],[cxx_compiler=Clang])
      ;;
    irix*)
      FATHOM_TRY_COMPILER_DEFINE([__sgi],[cxx_compiler=MIPSpro])
      ;;
    linux*)
      FATHOM_TRY_COMPILER_DEFINE([__INTEL_COMPILER],[cxx_compiler=Intel])
      FATHOM_TRY_COMPILER_DEFINE_GE([__IBMCPP__],[800],[cxx_compiler=VisualAge8],
        [FATHOM_TRY_COMPILER_DEFINE([__IBMCPP__],[cxx_compiler=VisualAge])])
      FATHOM_TRY_COMPILER_DEFINE([__DECCXX_VER],[cxx_compiler=Compaq])
      FATHOM_TRY_COMPILER_DEFINE([__SUNPRO_CC],[cxx_compiler=SunWorkshop])
      FATHOM_TRY_COMPILER_DEFINE([__PGI],[cxx_compiler=PortlandGroup])
      ;;
    hpux*)
      FATHOM_TRY_COMPILER_DEFINE([__HP_aCC],[cxx_compiler=HP])
      ;;
    windows*)
      FATHOM_TRY_COMPILER_DEFINE([__MSC_VER],[cxx_compiler=VisualStudio])
      FATHOM_TRY_COMPILER_DEFINE([__INTEL_COMPILER],[cxx_compiler=Intel])
      FATHOM_TRY_COMPILER_DEFINE([__DECCXX_VER],[cxx_compiler=Compaq])
      FATHOM_TRY_COMPILER_DEFINE([__BORLANDC__],[cxx_compiler=Borland])
      FATHOM_TRY_COMPILER_DEFINE([__CYGWIN__],[cxx_compiler=Cygwin])
      FATHOM_TRY_COMPILER_DEFINE([__MINGW32__],[cxx_compiler=MinGW])
      ;;
    *)
      FATHOM_TRY_COMPILER_DEFINE([__PGI],[cxx_compiler=PortlandGroup])
      ;;
  esac
fi

AC_LANG_POP([C++])
AC_MSG_RESULT([$cxx_compiler])
if test "x$cxx_compiler" = "xunknown"; then
  AC_MSG_WARN([Unrecognized C++ compiler: $CXX])
fi

# Now decide special compiler flags using compiler/cpu combination
AC_MSG_CHECKING([for known compiler/CPU/OS combinations])
case "$cxx_compiler:$host_cpu" in
  GNU:sparc*)
    FATHOM_CXX_32BIT=-m32
    FATHOM_CXX_64BIT=-m64
    FATHOM_CXX_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    ;;
  GNU:powerpc*)
    FATHOM_CXX_32BIT=-m32
    FATHOM_CXX_64BIT=-m64
    FATHOM_CXX_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    ;;
  GNU:i?86|GNU:x86_64)
    FATHOM_CXX_32BIT=-m32
    FATHOM_CXX_64BIT=-m64
    FATHOM_CXX_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    ;;
  GNU:mips*)
    FATHOM_CXX_32BIT="-mips32 -mabi=32"
    FATHOM_CXX_64BIT="-mips64 -mabi=64"
    FATHOM_CXX_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    ;;
  GNU:*)
    FATHOM_CXX_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    ;;
  Intel:*)
    FATHOM_CXX_32BIT=-m32
    FATHOM_CXX_64BIT=-m64
    FATHOM_CXX_SPECIAL="$EXTRA_INTEL_CXXFLAGS -wd981 -wd279 -wd1418 -wd383 -wd1572 -wd2259"
    ;;
  VisualAge:*)
    FATHOM_CXX_32BIT=-q32
    FATHOM_CXX_64BIT=-q64
    FATHOM_CXX_SPECIAL="$EXTRA_BG_CXXFLAGS -qminimaltoc -qmaxmem=-1 -qlanglvl=variadictemplates"
    AR="ar"
    NM="nm -B -X 32_64"
    ;;
  VisualAge8:*)
    FATHOM_CXX_32BIT=-q32
    FATHOM_CXX_64BIT=-q64
    FATHOM_CXX_SPECIAL="$EXTRA_BG_CXXFLAGS -qminimaltoc -qmaxmem=-1 -qlanglvl=variadictemplates"
    NM="nm -B -X 32_64"
    ;;
  MIPSpro:mips)
    FATHOM_CXX_32BIT=-n32
    FATHOM_CXX_64BIT=-64
    FATHOM_CXX_SPECIAL=-LANG:std
    ;;
  MIPSpro:*)
    FATHOM_CXX_SPECIAL=-LANG:std
    ;;
  SunWorkshop:sparc*)
    FATHOM_CXX_32BIT=-xarch=generic
    FATHOM_CXX_64BIT=-xarch=generic64
    ;;
  PortlandGroup:*)
    FATHOM_CXX_SPECIAL="$EXTRA_PGI_CXXFLAGS"
    ;;
  Clang:*)
    FATHOM_CXX_SPECIAL="$EXTRA_CLANG_CXXFLAGS"
    FATHOM_CXX_32BIT=-m32
    FATHOM_CXX_64BIT=-m64
    ;;
  SunWorkshop:i?86|SunWorkshop:x86_64)
    FATHOM_CXX_32BIT=-m32
    FATHOM_CXX_64BIT=-m64
    ;;
  *)
    ;;
esac
AC_MSG_RESULT([$cxx_compiler:$host_os:$host_cpu])

# Check for specific overrides
CXX_LDFLAGS=""
if (test "$cxx_compiler:${host_os:0:6}" == "Clang:darwin"); then
  CXX_LDFLAGS="$CXX_LDFLAGS -stdlib=libc++"
  LIBS="$LIBS -lc++"
fi
AC_SUBST(CXX_LDFLAGS)

]) # end FATHOM_CXX_FLAGS

#######################################################################################
# Check for compiler-specific flags.
# Sets the following environmental variables:
#   FATHOM_CC_SPECIAL : Any compiler-specific flags which must be specified
#   FATHOM_CC_32BIT   : Flag to force compilation of 32-bit code
#   FATHOM_CC_64BIT   : Flag to force compilation of 64-bit code
#######################################################################################
AC_DEFUN([FATHOM_CC_FLAGS], [
AC_LANG_PUSH([C])

# Detect compiler 

AC_MSG_CHECKING([for known C compilers])
# Autoconf does gcc for us
if test x$GCC = xyes; then
  cc_compiler=GNU
  # Intel claims to be GCC, check for it here
  FATHOM_TRY_COMPILER_DEFINE([__INTEL_COMPILER],[cc_compiler=Intel])
  FATHOM_TRY_COMPILER_DEFINE([__clang__],[cc_compiler=Clang])
  FATHOM_TRY_COMPILER_DEFINE([__PGI],[cc_compiler=PortlandGroup])
# Search for other compiler types
# For efficiency, limit checks to relevant OSs
else
  cc_compiler=unknown
  case "$target_os" in
    aix*)
      FATHOM_TRY_COMPILER_DEFINE([__IBMC__],[cc_compiler=VisualAge])
      ;;
    solaris*|sunos*)
      FATHOM_TRY_COMPILER_DEFINE([__SUNPRO_C],[cc_compiler=SunWorkshop])
      ;;
    darwin*)
      FATHOM_TRY_COMPILER_DEFINE([__clang__],[cc_compiler=Clang])
      ;;
    irix*)
      FATHOM_TRY_COMPILER_DEFINE([__sgi],[cc_compiler=MIPSpro])
      ;;
    linux*)
      FATHOM_TRY_COMPILER_DEFINE([__INTEL_COMPILER],[cc_compiler=Intel])
      FATHOM_TRY_COMPILER_DEFINE([__IBMC__],[cc_compiler=VisualAge])
      FATHOM_TRY_COMPILER_DEFINE([__DECC_VER],[cc_compiler=Compaq])
      FATHOM_TRY_COMPILER_DEFINE([__SUNPRO_C],[cc_compiler=SunWorkshop])
      FATHOM_TRY_COMPILER_DEFINE([__PGI],[cc_compiler=PortlandGroup])
      ;;
    hpux*)
      FATHOM_TRY_COMPILER_DEFINE([__HP_cc],[cc_compiler=HP])
      ;;
    windows*)
      FATHOM_TRY_COMPILER_DEFINE([__MSC_VER],[cc_compiler=VisualStudio])
      FATHOM_TRY_COMPILER_DEFINE([__INTEL_COMPILER],[cc_compiler=Intel])
      FATHOM_TRY_COMPILER_DEFINE([__DECC_VER],[cc_compiler=Compaq])
      FATHOM_TRY_COMPILER_DEFINE([__BORLANDC__],[cc_compiler=Borland])
      FATHOM_TRY_COMPILER_DEFINE([__TURBOC__],[cc_compiler=TurboC])
      FATHOM_TRY_COMPILER_DEFINE([__CYGWIN__],[cc_compiler=Cygwin])
      FATHOM_TRY_COMPILER_DEFINE([__MINGW32__],[cc_compiler=MinGW])
      ;;
    *)
      FATHOM_TRY_COMPILER_DEFINE([__PGI],[cc_compiler=PortlandGroup])
      ;;
  esac
fi
AC_LANG_POP([C])
AC_MSG_RESULT([$cc_compiler])
if test "x$cc_compiler" = "xunknown"; then
  AC_MSG_WARN([Unrecognized C compiler: $CXX])
fi

# Now decide special compiler flags using compiler/cpu combination
AC_MSG_CHECKING([for known compiler/OS combinations])
case "$cc_compiler:$host_cpu" in
  GNU:sparc*)
    FATHOM_CC_32BIT=-m32
    FATHOM_CC_64BIT=-m64
    FATHOM_CC_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    FATHOM_FC_SPECIAL="$EXTRA_GNU_ONLY_FCFLAGS"
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    ;;
  GNU:powerpc*)
    FATHOM_CC_32BIT=-m32
    FATHOM_CC_64BIT=-m64
    FATHOM_CC_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    FATHOM_FC_SPECIAL="$EXTRA_GNU_ONLY_FCFLAGS"
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    ;;
  GNU:i?86|GNU:x86_64)
    FATHOM_CC_32BIT=-m32
    FATHOM_CC_64BIT=-m64
    FATHOM_CC_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    FATHOM_FC_SPECIAL="$EXTRA_GNU_ONLY_FCFLAGS"
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    ;;
  GNU:mips*)
    FATHOM_CC_32BIT="-mips32 -mabi=32"
    FATHOM_CC_64BIT="-mips64 -mabi=64"
    FATHOM_CC_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    FATHOM_FC_SPECIAL="$EXTRA_GNU_ONLY_FCFLAGS"
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    ;;
  GNU:*)
    FATHOM_CC_SPECIAL="$EXTRA_GNU_ONLY_CXXFLAGS"
    FATHOM_FC_SPECIAL="$EXTRA_GNU_ONLY_FCFLAGS"
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    ;;
  Intel:*)
    FATHOM_CC_32BIT=-m32
    FATHOM_CC_64BIT=-m64
    FATHOM_CC_SPECIAL="$EXTRA_INTEL_CXXFLAGS -wd981 -wd279 -wd1418 -wd383 -wd1572"
    FATHOM_FC_SPECIAL="$EXTRA_INTEL_FCFLAGS"
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    ;;
  VisualAge:*)
    case "$target_vendor" in
      bgp)
        FATHOM_CC_32BIT=-q32
        FATHOM_CC_64BIT=-q64
        AR="ar"
        NM="nm -B"
        ;;
      bgq)
        FATHOM_CC_32BIT=-q32
        FATHOM_CC_64BIT=-q64
        FATHOM_CC_SPECIAL="$EXTRA_BG_CXXFLAGS -qmaxmem=-1 -qminimaltoc"
        FATHOM_FC_SPECIAL="$EXTRA_BG_FCFLAGS -qnoescape -WF,-C! -qddim -qalias=intptr"
        FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
        AR="ar"
        NM="nm -B"
        ;;
      *)
        FATHOM_CC_32BIT=-q32
        FATHOM_CC_64BIT=-q64
        FATHOM_CC_SPECIAL="$EXTRA_BG_CXXFLAGS -qmaxmem=-1 -qminimaltoc"
        FATHOM_FC_SPECIAL="$EXTRA_BG_FCFLAGS -qnoescape -WF,-C! -qddim -qalias=intptr"
        FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
        AR="ar"
        NM="nm -B -X 32_64"
        ;;
    esac
    ;;
  MIPSpro:mips)
    FATHOM_CC_32BIT=-n32
    FATHOM_CC_64BIT=-64
    FATHOM_CC_SPECIAL=-LANG:std
    FATHOM_FC_SPECIAL=-LANG:std
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    ;;
  MIPSpro:*)
    FATHOM_CC_SPECIAL=-LANG:std
    FATHOM_FC_SPECIAL=-LANG:std
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    ;;
  PortlandGroup:*)
    FATHOM_CC_SPECIAL="-traceback -C"
    FATHOM_FC_SPECIAL="$EXTRA_PGI_FCFLAGS $EXTRA_PGI_ONLY_FCFLAGS"
    FATHOM_F77_SPECIAL="$EXTRA_PGI_FCFLAGS"
    ;;
  Clang:*)
    FATHOM_CC_SPECIAL="$EXTRA_CLANG_CXXFLAGS"
    FATHOM_FC_SPECIAL="$EXTRA_CLANG_FCFLAGS"
    FATHOM_F77_SPECIAL="$FATHOM_FC_SPECIAL"
    FATHOM_CC_32BIT=-m32
    FATHOM_CC_64BIT=-m64
    ;;
  SunWorkshop:sparc*)
    FATHOM_CC_32BIT=-xarch=generic
    FATHOM_CC_64BIT=-xarch=generic64
    ;;
  SunWorkshop:i?86|SunWorkshop:x86_64)
    FATHOM_CC_32BIT=-m32
    FATHOM_CC_64BIT=-m64
    ;;
  *)
    ;;
esac
AC_MSG_RESULT([$cc_compiler:$host_cpu])

]) # end FATHOM_CC_FLAGS


#######################################################################################
# Check for Fortran compiler-specific flags.
#######################################################################################

dnl PAC_PROG_F77_CRAY_POINTER - Check if Fortran 77 supports Cray-style pointer.
dnl                             If so, set pac_cv_prog_f77_has_pointer to yes
dnl                             and find out if any extra compiler flag is
dnl                             needed and set it as CRAYPTR_FFLAGS.
dnl                             i.e. CRAYPTR_FFLAGS is meaningful only if
dnl                             pac_cv_prog_f77_has_pointer = yes.
dnl
dnl Synopsis:
dnl   PAC_PROG_F77_CRAY_POINTER([action-if-true],[action-if-false])
dnl D*/
AC_DEFUN([PAC_PROG_F77_CRAY_POINTER],[
AC_CACHE_CHECK([whether Fortran 77 supports Cray-style pointer],
pac_cv_prog_f77_has_pointer,[
AC_LANG_PUSH([Fortran 77])
AC_LANG_CONFTEST([
    AC_LANG_PROGRAM([],[
        integer M
        pointer (MPTR,M)
        data MPTR/0/
    ])
])
saved_FFLAGS="$FFLAGS"
pac_cv_prog_f77_has_pointer=no
CRAYPTR_FFLAGS=""
for ptrflag in '' '-fcray-pointer' ; do
    FFLAGS="$saved_FFLAGS $ptrflag"
    AC_COMPILE_IFELSE([], [
        pac_cv_prog_f77_has_pointer=yes
        CRAYPTR_FFLAGS="$ptrflag"
        break
    ])
done
dnl Restore FFLAGS first, since user may not want to modify FFLAGS
FFLAGS="$saved_FFLAGS"
dnl remove conftest after ac_lang_conftest
rm -f conftest.$ac_ext
AC_LANG_POP([Fortran 77])
])

if test "$pac_cv_prog_f77_has_pointer" = "yes" ; then
    AC_MSG_CHECKING([for Fortran 77 compiler flag for Cray-style pointer])
    if test "X$CRAYPTR_FFLAGS" != "X" ; then
        AC_MSG_RESULT([$CRAYPTR_FFLAGS])
    else
        AC_MSG_RESULT([none])
    fi
    ifelse([$1],[],[:],[$1])
else
    ifelse([$2],[],[:],[$2])
fi
])

dnl PAC_PROG_FC_CRAY_POINTER - Check if Fortran supports Cray-style pointer.
dnl                            If so, set pac_cv_prog_fc_has_pointer to yes
dnl                            and find out if any extra compiler flag is
dnl                            needed and set it as CRAYPTR_FCFLAGS.
dnl                            i.e. CRAYPTR_FCFLAGS is meaningful only if
dnl                            pac_cv_prog_fc_has_pointer = yes.
dnl
dnl Synopsis:
dnl   PAC_PROG_FC_CRAY_POINTER([action-if-true],[action-if-false])
dnl D*/
AC_DEFUN([PAC_PROG_FC_CRAY_POINTER],[
AC_CACHE_CHECK([whether Fortran 90 supports Cray-style pointer],
pac_cv_prog_fc_has_pointer,[
AC_LANG_PUSH([Fortran])
AC_LANG_CONFTEST([
    AC_LANG_PROGRAM([],[
        integer M
        pointer (MPTR,M)
        data MPTR/0/
    ])
])
saved_FCFLAGS="$FCFLAGS"
pac_cv_prog_fc_has_pointer=no
CRAYPTR_FCFLAGS=""
for ptrflag in '' '-fcray-pointer' ; do
    FCFLAGS="$saved_FCFLAGS $ptrflag"
    AC_COMPILE_IFELSE([],[
        pac_cv_prog_fc_has_pointer=yes
        CRAYPTR_FCFLAGS="$ptrflag"
        break
    ])
done

dnl Restore FCFLAGS first, since user may not want to modify FCFLAGS
FCFLAGS="$saved_FCFLAGS"
dnl remove conftest after ac_lang_conftest
rm -f conftest.$ac_ext
AC_LANG_POP([Fortran])
])

if test "$pac_cv_prog_fc_has_pointer" = "yes" ; then
    AC_MSG_CHECKING([for Fortran 90 compiler flag for Cray-style pointer])
    if test "X$CRAYPTR_FCFLAGS" != "X" ; then
        AC_MSG_RESULT([$CRAYPTR_FCFLAGS])
    else
        AC_MSG_RESULT([none])
    fi
    ifelse([$1],[],[:],[$1])
else
    ifelse([$2],[],[:],[$2])
fi
])
