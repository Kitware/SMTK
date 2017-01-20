################################################################################
#                           Standard Stuff
################################################################################
AC_INIT([MOAB],[4.9.3pre],[moab-dev@mcs.anl.gov],[moab],[http://sigma.mcs.anl.gov])
AC_CONFIG_SRCDIR([moab.make.in])
AC_CONFIG_SRCDIR([MOABConfig.cmake.in])
AC_CONFIG_SRCDIR([examples/makefile.in])
AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_AUX_DIR([config])
AC_CANONICAL_TARGET
m4_ifdef([AM_SILENT_RULES],
          [m4_define([mb_am_oldtests], [[color-tests] [parallel-tests]])],
          [m4_define([mb_am_oldtests], [])])
AM_INIT_AUTOMAKE([subdir-objects] mb_am_oldtests)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CHECK_PROG( [SED], [sed], [sed], [true] )

# Infer the source directory as the path to the ./configure script
#srcdir=`dirname $0`
#top_srcdir=`dirname $0`
USER_CONFIGURE_CMD="$0 $ac_configure_args"

################################################################################
#                           Compilers
################################################################################
INITIALIZE_EXTERNAL_PACKAGES

MB_CPPFLAGS=""
MB_DEFS=""

ENABLE_FORTRAN=no
AC_ARG_ENABLE([fortran],
              [AS_HELP_STRING([--disable-fortran], [Disable Fortran support and name mangling in ITAPS/MBCN C headers])])

if (test "x$enable_fortran" != "xno"); then
  ENABLE_FORTRAN=yes
fi
AC_SUBST(ENABLE_FORTRAN)
DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-fortran=$ENABLE_FORTRAN"

# Find out the appropriate compiler based on user options
FATHOM_CHECK_COMPILERS([yes],[yes],[$ENABLE_FORTRAN])
AM_CONDITIONAL(PARALLEL,[test "x$enablempi" != "xno"])
AM_CONDITIONAL(ENABLE_FORTRAN,[test "x$ENABLE_FORTRAN" != "xno"])
AC_SUBST(enablempi)

# Invoke and find all necessary flags for C, C++, FC
FATHOM_COMPILER_FLAGS([yes],[yes],[$ENABLE_FORTRAN])

AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_DISABLE_SHARED

# All language options initialized -- now call
# libtool initialization
# AC_PROG_LIBTOOL is deprecated
LT_INIT([dlopen])

AC_C_BIGENDIAN

AC_CHECK_PROG([ZCAT],[gunzip],[gunzip -c],[])
AC_CHECK_PROG([ZCAT],[gzip],[gzip -cd],[])
AC_CHECK_PROG([ZCAT],[zcat],[zcat],[])
if test "x" = "x$ZCAT"; then
  AC_MSG_WARN([[Cannot run tests: no way to uncompress input files.]])
fi
AC_SUBST([ZCAT])
AM_CONDITIONAL([HAVE_ZCAT],[test "x" = "x$ZCAT"])

FATHOM_TEMPLATE_SPECIALIZATION
FATHOM_TEMPLATE_FUNC_SPECIALIZATION
MB_DEFS="$MB_DEFS $TEMPLATE_SPECIALIZATION $TEMPLATE_FUNC_SPECIALIZATION"

FATHOM_CHECK_CXX_WORKS([], [AC_MSG_ERROR([Cannot build without C++ compiler])])

################################################################################
#            Check for need for extra flags to support cray pointers
################################################################################
if test "xyes" = "x$ENABLE_FORTRAN" && test "x" != "x$F77"; then
  PAC_PROG_F77_CRAY_POINTER([
    FFLAGS="$FFLAGS $CRAYPTR_FFLAGS"
  ])
fi

if test "xyes" = "x$ENABLE_FORTRAN" && test "x" != "x$FC"; then
  PAC_PROG_FC_CRAY_POINTER([
    FCFLAGS="$FCFLAGS $CRAYPTR_FCFLAGS"
  ])
fi

################################################################################
#                           Basic Portability Stuff
################################################################################
AC_CHECK_FUNC([vsnprintf],
              AC_DEFINE([HAVE_VSNPRINTF],[1],[Define if vsnprintf is available.]))

FATHOM_VECTOR_TEMPLATE_INSERT
FATHOM_OLD_STD_COUNT
MB_DEFS="$MB_DEFS $NO_VECTOR_TEMPLATE_INSERT $OLD_STD_COUNT"

################################################################################
#                              HANDLE SIZE
################################################################################
AC_CHECK_HEADER([inttypes.h], [AC_DEFINE(HAVE_INTTYPES_H,  [1], [MOAB qualified HAVE_INTTYPES_H]) ])
AC_CHECK_HEADER([stdint.h],   [AC_DEFINE(HAVE_STDINT_H,    [1], [MOAB qualified HAVE_STDINT_H])   ])
AC_CHECK_HEADER([sys/types.h],[AC_DEFINE(HAVE_SYS_TYPES_H, [1], [MOAB qualified HAVE_SYS_TYPES_H])])
AC_CHECK_HEADER([stddef.h],   [AC_DEFINE(HAVE_STDDEF_H,    [1], [MOAB qualified HAVE_STDDEF_H])   ])
AC_CHECK_HEADER([stdlib.h],   [AC_DEFINE(HAVE_STDLIB_H,    [1], [MOAB qualified HAVE_STDLIB_H])   ])
SIZEOF_MBENTITYHANDLE=0
AC_CHECK_SIZEOF([void *])
SIZEOF_VOID_P=$ac_cv_sizeof_void_p
AC_SUBST(SIZEOF_VOID_P)
AC_ARG_ENABLE([64bit-handles],
  [AS_HELP_STRING([--enable-64bit-handles],[Force EntityHandle to be 64 bits])],
  [AC_CHECK_TYPE([uint64_t],
    [AC_DEFINE(FORCE_64_BIT_HANDLES,[1],[Use int64_t for handles])
     SIZEOF_MBENTITYHANDLE=8
     DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-64bit-handles"],
    [AC_MSG_ERROR([Platform does not have uint64_t. Cannot force 64-bit handles])])
])
AC_ARG_ENABLE([32bit-handles],
  [AS_HELP_STRING([--enable-32bit-handles],[Force EntityHandle to be 32 bits])],
  [AC_CHECK_TYPE([uint32_t],
    [AC_DEFINE(FORCE_32_BIT_HANDLES,[1],[Use int32_t for handles])
     SIZEOF_MBENTITYHANDLE=4
     DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-32bit-handles"],
    [AC_MSG_ERROR([Platform does not have uint32_t. Cannot force 32-bit handles])])
])
if test "x$SIZEOF_MBENTITYHANDLE" = "x0"; then
  SIZEOF_MBENTITYHANDLE=$SIZEOF_VOID_P
  AC_CHECK_TYPE([size_t],
   [AC_DEFINE(HAVE_SIZE_T,[1],[System provides size_t typedef])],
   [AC_CHECK_SIZEOF([unsigned long])
    SIZEOF_UNSIGNGED_LONG=$ac_cv_sizeof_unsigned_long
    if test "$SIZEOF_UNSIGNED_LONG" -ne "$SIZEOF_VOID_P"; then
      AC_MSG_ERROR([Cannot define handle with sizeof void*: size_t not found and long is of incorrect size.])
    fi]
  )
  AC_CHECK_TYPE([ptrdiff_t],
   [AC_DEFINE(HAVE_PTRDIFF_T,[1],[System provides ptrdiff_t typedef])],
   [AC_CHECK_SIZEOF([long])
    AC_CHECK_SIZEOF([void *])
    SIZEOF_VOID_P=$ac_cv_sizeof_void_p
    SIZEOF_LONG=$ac_cv_sizeof_long
    if test "$SIZEOF_LONG" -ne "$SIZEOF_VOID_P"; then
      AC_MSG_ERROR([Cannot define handle with sizeof void*: ptrdiff_t not found and long is of incorrect size.])
    fi]
  )
fi

# Use 64-bit off_t value for file offsets.  W/out this, the stat/fseek calls
# in serial_open_file and other places will fail on 32-bit systems if the file
# is larger that 2GB.
if (test "x$enable_32bit_handles" != "x"); then
  MB_DEFS="$MB_DEFS -D_FILE_OFFSET_BITS=64"
  # AC_DEFINE_UNQUOTED([FILE_OFFSET_BITS], [64], [Support for handling files larger than 2GB on 32-bit machines])
fi

AC_CHECK_LIB(z, compress, LIBS="$LIBS -lz")
AC_CHECK_LIB(dl, dlopen, LIBS="$LIBS -ldl")
AC_CHECK_LIB(m, pow, LIBS="$LIBS -lm")

################################################################################
#                           iMesh Part 1 of 2
################################################################################
AC_ARG_ENABLE( [imesh],
[AS_HELP_STRING([--disable-imesh],[Do not build support for iMesh interface.])],
[disable_imesh=$enableval],
[disable_imesh=yes] )

if (test "xyes" != "x$disable_imesh" && test "xno" != "x$disable_imesh"); then
  AC_MSG_ERROR([Unknown argument --enable-imesh=$enableval])
else
  if (test "$disable_imesh" != "no"); then
    ENABLE_imesh=yes
  else
    ENABLE_imesh=no
  fi
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-imesh=$ENABLE_imesh"
fi

################################################################################
#                           FBiGeom Part 1 of 2
################################################################################
AC_ARG_ENABLE( [fbigeom],
[AS_HELP_STRING([--enable-fbigeom],[Build mesh-based (facets) iGeom implementation])],
[enable_fbigeom=$enableval],[enable_fbigeom=no] )

if (test "xyes" != "x$enable_fbigeom" && test "xno" != "x$enable_fbigeom"); then
  AC_MSG_ERROR([Unknown argument --enable-fbigeom=$enableval])
else
  ENABLE_fbigeom=$enable_fbigeom
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-fbigeom=$enable_fbigeom"
fi

################################################################################
#                           iMesh/FBiGeom Part 2 of 2
################################################################################

# The iMesh option must be checked before configuring the compilers
# so that we know if we need Fortran.  This stuff has to be done
# after configuring the compilers so that we know what size of
# various types are.

AC_MSG_CHECKING([size of EntityHandle])
AC_MSG_RESULT([$SIZEOF_MBENTITYHANDLE])
ITAPS_VALID_HANDLE_SIZE=yes
if test $SIZEOF_VOID_P -ne $SIZEOF_MBENTITYHANDLE; then
  ITAPS_VALID_HANDLE_SIZE=no
  AC_MSG_WARN([Cannot build iMesh or FBiGeom API because sizeof(EntityHandle) != sizeof(void*)])
  ENABLE_imesh=no
  ENABLE_fbigeom=no
fi

AC_MSG_CHECKING([if FBiGeom support is to be built])
AC_MSG_RESULT([$ENABLE_fbigeom])
AM_CONDITIONAL([ENABLE_fbigeom],[test "xyes" = "x$ENABLE_fbigeom"])
if test "x$ENABLE_fbigeom" = "xyes"; then
  if test "x$ITAPS_VALID_HANDLE_SIZE" != "xyes"; then
    AC_MSG_ERROR([Cannot build FBiGeom API with sizeof(EntityHandle) != sizeof(void*)])
  fi
  AC_DEFINE([HAVE_FBIGEOM],[1],[Define if configured with FBiGeom interfaces.])

fi

AC_MSG_CHECKING([if iMesh support is to be built])
AC_MSG_RESULT([$ENABLE_imesh])
AM_CONDITIONAL([ENABLE_imesh],[test "xyes" = "x$ENABLE_imesh"])
if test "x$ENABLE_imesh" = "xyes"; then
  if test "x$ITAPS_VALID_HANDLE_SIZE" != "xyes"; then
    AC_MSG_ERROR([Cannot build iMesh API with sizeof(EntityHandle) != sizeof(void*)])
  fi
  AC_DEFINE([HAVE_IMESH],[1],[Define if configured with iMesh interfaces.])
fi

################################################################################
#                           iRel/Lasso Part 1 of 2
################################################################################
AC_ARG_ENABLE( [irel],
[AS_HELP_STRING([--enable-irel],[Build relational geometry-mesh query interface implementation])],
[enable_irel=$enableval],[enable_irel=no] )

if (test "xyes" != "x$enable_irel" && test "xno" != "x$enable_irel"); then
  AC_MSG_ERROR([Unknown argument --enable-irel=$enableval])
else
  ENABLE_irel=$enable_irel
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-irel=$enable_irel"
fi


################################################################################
#                              Version Stuff
################################################################################

AC_DEFINE(VERSION,["AC_PACKAGE_VERSION"],[MOAB Version])
VERSION_MAJOR=`expr AC_PACKAGE_VERSION : '\([[0-9]]*\)'`
VERSION_MINOR=`expr AC_PACKAGE_VERSION : '[[0-9]]*\.\([[0-9]]*\)'`
VERSION_PATCH=`expr AC_PACKAGE_VERSION : '[[0-9]]*\.[[0-9]]*\.\(.*\)'`
test "x" != "x$VERSION_MAJOR" || AC_MSG_ERROR("Invalid version string: AC_PACKAGE_VERSION")
test "x" != "x$VERSION_MINOR" || AC_MSG_ERROR("Invalid version string: AC_PACKAGE_VERSION")
AC_DEFINE_UNQUOTED(VERSION_MAJOR,$VERSION_MAJOR,[MOAB Major Version])
AC_DEFINE_UNQUOTED(VERSION_MINOR,$VERSION_MINOR,[MOAB Minor Version])
if test "x" != "x$VERSION_PATCH"; then
  AC_DEFINE_UNQUOTED(VERSION_PATCH,$VERSION_PATCH,[MOAB Patch Level])
  VERSION_STRING="AC_PACKAGE_STRING"
elif test $VERSION_MINOR -eq 99; then
  VERSION_STRING="AC_PACKAGE_STRING (Alpha)"
else
  VERSION_STRING="AC_PACKAGE_STRING (Beta)"
fi
AC_DEFINE_UNQUOTED(VERSION_STRING,"${VERSION_STRING}",[MOAB Version String])

################################################################################
#                Create libtool script
################################################################################
# We need the libtool script both for the calls to the ITAPS_LIBTOOL_VAR
# macro and for us in testing for libraries.  If we're using a newer
# version of libtool, the script normally isn't generated until AC_OUTPUT
# at the end of the configure script.  Ask that it be created now instead.
m4_ifdef([LT_OUTPUT],[LT_OUTPUT])

################################################################################
#                              Extract libtool config
################################################################################
FATHOM_LIBTOOL_VAR( [CXX], [compiler_lib_search_path], [MOAB_CXX_LINKFLAGS])
FATHOM_LIBTOOL_VAR( [CXX], [postdeps], [MOAB_CXX_LIBS_TMP])
MOAB_CXX_LIBS=
for lib in $MOAB_CXX_LIBS_TMP; do
  case $lib in
    -lgcc_s)
      ;;
    -lm)
      ;;
    -lc)
      ;;
    *)
      MOAB_CXX_LIBS="$MOAB_CXX_LIBS $lib"
      ;;
  esac
done
AC_SUBST(MOAB_CXX_LINKFLAGS)
AC_SUBST(MOAB_CXX_LIBS)


################################################################################
#                             System Headers
################################################################################
HAVE_IEEEFP=0
AC_CHECK_HEADER([ieeefp.h],[HAVE_IEEEFP_H=1] )
AM_CONDITIONAL(HAVE_IEEEFP_H, [test "$HAVE_IEEEFP"!="0"])
if (test "$HAVE_IEEEFP" != "0"); then
  AC_DEFINE(HAVE_IEEEFP, [1], [Defined if configured with IEEE Floating point support])
fi

## Check for some mathematical functions
AX_CXX_HAVE_STDISFINITE
AX_CXX_HAVE_ISFINITE
AX_CXX_HAVE_FINITE

HAVE_VALGRIND=0
if test "xyes" = "x$enable_debug"; then
  AC_CHECK_HEADER( [valgrind/memcheck.h], [HAVE_VALGRIND=1] )
  if (test "$HAVE_IEEEFP" != "0"); then
    AC_DEFINE(HAVE_VALGRIND, [1], [Defined if configured with Valgrind support])
  fi
fi
AM_CONDITIONAL(HAVE_VALGRIND, [test "$HAVE_VALGRIND"!="0"])

################################################################################
#                             BOOST OPTIONS
################################################################################

dnl FATHOM_CHECK_BOOST( [boost/pool/object_pool.hpp] )
# MK_CHECK_UNORDERED_MAP([CPPFLAGS="$CPPFLAGS -DUNORDERED_MAP_NS=$result -DHAVE_UNORDERED_MAP=${incdir}unordered_map -DHAVE_UNORDERED_SET=${incdir}unordered_set"])
MK_CHECK_UNORDERED_MAP([AC_DEFINE_UNQUOTED([UNORDERED_MAP_NS], [$result], [Unordered map namespace])
                        AC_DEFINE_UNQUOTED([HAVE_UNORDERED_MAP], [${incdir}unordered_map], [Specify if unordered map is available])
                        AC_DEFINE_UNQUOTED([HAVE_UNORDERED_SET], [${incdir}unordered_set], [Specify if unordered set is available])])

################################################################################
#                              MPI OPTIONS
################################################################################

FATHOM_CHECK_MPI
FATHOM_CONFIG_MPI_EXEC
if (test "xno" != "x$enablempi"); then
  AC_DEFINE([HAVE_MPI],[1],[Define if configured with support for parallel computations.])
fi
AM_CONDITIONAL(HAVE_MPI, [test "xno" != "x$enablempi"])
# Used to generate moab_mpi.h
if test "xyes" = "x$MPI_CXX_HELP_NEEDED"; then
  AC_DEFINE( [MPI_CXX_CONFLICT], [1],
    [MPICH_IGNORE_CXX_SEEK is not sufficient to avoid conflicts] )
  AC_DEFINE_UNQUOTED([SEEK_SET],[$SEEK_SET],["Value of C SEEK_SET"])
  AC_DEFINE_UNQUOTED([SEEK_CUR],[$SEEK_CUR],["Value of C SEEK_CUR"])
  AC_DEFINE_UNQUOTED([SEEK_END],[$SEEK_END],["Value of C SEEK_END"])
fi
FATHOM_CHECK_MPITYPE

if (test "$MPIFAMILY" = "OPENMPI" && test "x$ENABLE_FORTRAN" != "xno"); then
  FCLIBS="$FCLIBS -lmpi_cxx"
fi

if test "xyes" = "x$enablempi"; then
  FATHOM_CHECK_MPE
  AC_SUBST([WITH_MPE])
  AC_SUBST([MPE_LIBS])
  test "xyes" != "x$WITH_MPE" || MB_DEFS="$MB_DEFS -DUSE_MPE"
fi

################################################################################
#                              HDF5 OPTIONS
################################################################################

FATHOM_CHECK_HDF5

################################################################################
#                             CCMIO OPTIONS
################################################################################

old_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $HDF5_LDFLAGS"
FATHOM_CHECK_CCMIO
LDFLAGS="$old_LDFLAGS"
if test "xno" != "x$HAVE_CCMIO"; then
  AC_DEFINE([HAVE_CCMIO],[1],["Define if configured with CCM I/O support."])
fi
AM_CONDITIONAL(HAVE_CCMIO, [test "xno" != "x$HAVE_CCMIO"])
MB_CPPFLAGS="$CCMIO_CPPFLAGS $MB_CPPFLAGS"
EXPORT_LDFLAGS="$CCMIO_LDFLAGS $EXPORT_LDFLAGS"
AC_SUBST(CCMIO_LIBS)


################################################################################
#                             DAMSEL OPTIONS
################################################################################

old_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $HDF5_LDFLAGS"
FATHOM_CHECK_DAMSEL
LDFLAGS="$old_LDFLAGS"
if (test "xno" != "x$HAVE_DAMSEL" && test "x" != "x$HAVE_DAMSEL"); then
  AC_DEFINE([HAVE_DAMSEL],[1],["Define if configured with Damsel I/O format support."])
fi
AM_CONDITIONAL(HAVE_DAMSEL, [test "xno" != "x$HAVE_DAMSEL" && test "x" != "x$HAVE_DAMSEL"])
MB_CPPFLAGS="$DAMSEL_CPPFLAGS $MB_CPPFLAGS"
EXPORT_LDFLAGS="$DAMSEL_LDFLAGS $EXPORT_LDFLAGS"
AC_SUBST(DAMSEL_LIBS)


################################################################################
#                             NetCDF OPTIONS
################################################################################

  # now pnetcdf
FATHOM_CHECK_PNETCDF([65536],[524288])
if test "xyes" = "x$enablepnetcdf"; then
  AC_DEFINE([HAVE_PNETCDF],[1],["Define if configured with PNetCDF support."])
fi
AM_CONDITIONAL(HAVE_PNETCDF, [test "xyes" = "x$enablepnetcdf"])
MB_CPPFLAGS="$PNETCDF_CPPFLAGS $MB_CPPFLAGS"
EXPORT_LDFLAGS="$PNETCDF_LDFLAGS $EXPORT_LDFLAGS"
AC_SUBST(PNETCDF_LIBS)
AC_SUBST(enablepnetcdf)

old_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $PNETCDF_LDFLAGS $HDF5_LDFLAGS"
FATHOM_CHECK_NETCDF([65536],[524288])
LDFLAGS="$old_LDFLAGS"
if test "xyes" = "x$enablenetcdf"; then
  AC_DEFINE([HAVE_NETCDF],[1],["Define if configured with NetCDF support."])
fi
AM_CONDITIONAL(HAVE_NETCDF, [test "xno" != "x$enablenetcdf"])
MB_CPPFLAGS="$NETCDF_CPPFLAGS $MB_CPPFLAGS"
EXPORT_LDFLAGS="$NETCDF_LDFLAGS $EXPORT_LDFLAGS"
AC_SUBST(NETCDF_LIBS)
AC_SUBST(enablenetcdf)


################################################################################
#                             CGNS OPTIONS
################################################################################

old_LDFLAGS="$LDFLAGS"
LDFLAGS="$LDFLAGS $HDF5_LDFLAGS"
FATHOM_CHECK_CGNS
LDFLAGS="$old_LDFLAGS"
if test "xno" != "x$HAVE_CGNS"; then
  AC_DEFINE([HAVE_CGNS],[1],["Define if configured with CGNS support."])
fi
AM_CONDITIONAL(HAVE_CGNS, [test "xno" != "x$HAVE_CGNS"])
MB_CPPFLAGS="$CGNS_CPPFLAGS $MB_CPPFLAGS"
EXPORT_LDFLAGS="$CGNS_LDFLAGS $EXPORT_LDFLAGS"
AC_SUBST(CGNS_LIBS)


#################################################################################
#                             Documentation
#################################################################################
AC_ARG_ENABLE([docs],
[AS_HELP_STRING([--enable-docs],[indicate to check for doxygen installation])],
                        [ENABLE_DOCS=yes],[ENABLE_DOCS=no] )
AC_ARG_WITH([doxygen],
[AS_HELP_STRING([--with-doxygen=DIR],[Specify directory where Doxygen program is installed. By default, it is /usr/bin])],
[WITH_DOXYGEN="$withval"], [WITH_DOXYGEN=no])

if test "x$WITH_DOXYGEN" != "xno"; then
  AC_PATH_PROGS( [DOXYGEN], [doxygen], [no],[$WITH_DOXYGEN] )
else
  AC_PATH_PROGS( [DOXYGEN], [doxygen], [no],[$PATH])
fi
if test "x$ENABLE_DOCS" = "xyes"; then
  if test "x$DOXYGEN" = "xno"; then
    AC_MSG_ERROR("Doxygen executable not found.")
  fi
fi
AC_SUBST([DOXYGEN])
AM_CONDITIONAL([ENABLE_DOCS],[test "x$ENABLE_DOCS" != "xno"])

################################################################################
#                           Zoltan - Part 1 of 2
################################################################################
WITH_ZOLTAN=no
ZOLTAN_DIR=
ZOLTAN_ARCH=
WITH_PARAMETIS=no
PARMETIS_DIR=
METIS_DIR=
SCOTCH_DIR=
PTSCOTCH_DIR=
SCOTCH_LIBS=
WITH_SCOTCH=no
ZOLTAN_LIBS=
ZOLTAN_LIB_FLAGS=
ZOLTAN_INC_FLAGS=

# If user specified zoltan-arch, but not zoltan, assume
# they want zoltan support, otherwise assume no zoltan
ZOLTAN_DEFAULT=no

# Some versions of Zoltan put libraries in directories such as Obj_Linux,
# where the part after the Obj_ prefix is the zoltan architecture.  Here
# we allow the user to specify this architecture.  We try to avoid requiring
# this option, but we allow the user to specify it in case something is
# going wrong with the logic to choose one automatically.  The 'choose one
# automatically' logic is basically to add everything that matches Obj_*
# to the search path, with the assumption that the linker will ignore libraries
# for other architectures.
AC_ARG_WITH( [zoltan-arch],
             [AS_HELP_STRING([--with-zoltan-arch=OS],[Specify zoltan architecture])],
             [case "x$withval" in
               xyes|xno)
                 AC_MSG_ERROR([Expeted value for --with-zoltan-arch])
                 ;;
               x*)
                 ZOLTAN_ARCH="$withval"
                 ZOLTAN_DEFAULT=yes
                 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-zoltan-arch=\"${withval}\""
                 ;;
               esac])

# Enable support for Zoltan, and optionally specify directory in which
# to search for Zoltan libs and headers.  Specifying this also enables
# mbzoltan unless it is explicitly disabled.
AC_ARG_WITH( [zoltan],
             [AS_HELP_STRING([--with-zoltan=DIR],[Enable Zoltan support, and optionally specify Zoltan location])],
             [case "x$withval" in
               xyes)
                 WITH_ZOLTAN=yes
                 ;;
               xno)
                 WITH_ZOLTAN=no
                 ;;
               x*)
                 WITH_ZOLTAN=yes
                 ZOLTAN_DIR="$withval"
                 test -d "$withval" || AC_MSG_ERROR([Not a directory: with-zoltan=$withval])
                 if test -d "$withval/include"; then
                   ZOLTAN_INC_FLAGS="-I$withval/include"
                 else
                   ZOLTAN_INC_FLAGS="-I$withval"
                 fi
                 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-zoltan=\"${withval}\""
                 ;;
               esac],[WITH_ZOLTAN=$ZOLTAN_DEFAULT] )

FATHOM_CONFIGURE_METIS
FATHOM_CONFIGURE_PARMETIS

# If user specified with-scotch, then assume they are trying
# to add an additional library search directory in which to check
# for the scotch library.
AC_ARG_WITH([scotch],
            [AS_HELP_STRING([--with-scotch=DIR],[Directory containing Scotch library for Zoltan])],
             [SCOTCH_DIR="$withval"; enablescotch=yes],[SCOTCH_DIR=""; enablescotch=no] )

if (test "$enablescotch" != "no"); then
  case "x$SCOTCH_DIR" in
     xyes|xno)
       # error out -- expecting directory path
       AC_MSG_ERROR([Please specify the Scotch installation directory])
       ;;
     x*)
       test -d "$SCOTCH_DIR" || AC_MSG_ERROR([Not a directory: with-scotch=$SCOTCH_DIR])
       if test -d "$SCOTCH_DIR/lib"; then
         ZOLTAN_LIB_FLAGS="$ZOLTAN_LIB_FLAGS -L$SCOTCH_DIR/lib"
         if (test -f "$SCOTCH_DIR/lib/libscotch.a" || test -f "$SCOTCH_DIR/lib/libscotch.so" || test -f "$SCOTCH_DIR/lib/libscotch.dylib"); then
           SCOTCH_LIBS="-lscotch -lscotcherr -lscotcherrexit"
           AC_DEFINE([HAVE_SCOTCH],[1],["Define if configured with Scotch library partitioning support."])
         else
           AC_MSG_ERROR([Scotch libraries not found at $SCOTCH_DIR/lib])
         fi
       else
         ZOLTAN_LIB_FLAGS="$ZOLTAN_LIB_FLAGS -L$SCOTCH_DIR"
         if (test -f "$SCOTCH_DIR/libscotch.a" || test -f "$SCOTCH_DIR/libscotch.so" || test -f "$SCOTCH_DIR/libscotch.dylib"); then
           SCOTCH_LIBS="-lscotch -lscotcherr -lscotcherrexit"
         else
           AC_MSG_ERROR([Scotch libraries not found at $SCOTCH_DIR])
         fi
       fi
       if test -d "$SCOTCH_DIR/include"; then
         ZOLTAN_INC_FLAGS="$ZOLTAN_INC_FLAGS -I$SCOTCH_DIR/include"
       else
         ZOLTAN_INC_FLAGS="$ZOLTAN_INC_FLAGS -I$SCOTCH_DIR"
       fi
       DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-scotch=\"${SCOTCH_DIR}\""
       ;;
   esac
fi

# If user specified with-ptscotch, then assume they are trying
# to add an additional library search directory in which to check
# for the parallel scotch library.
AC_ARG_WITH([ptscotch],
            [AS_HELP_STRING([--with-ptscotch=DIR],[Directory containing PTScotch library for Zoltan])],
             [PTSCOTCH_DIR="$withval"; enableptscotch=yes],[PTSCOTCH_DIR="$SCOTCH_DIR"; enableptscotch=$enablescotch] )

if (test "$enableptscotch" != "no"); then
  case "x$PTSCOTCH_DIR" in
     xyes|xno)
       # error out -- expecting directory path
       AC_MSG_ERROR([Please specify the parallel Scotch installation directory])
       ;;
     x*)
       PTSCOTCH_DIR="$PTSCOTCH_DIR"
       test -d "$PTSCOTCH_DIR" || AC_MSG_ERROR([Not a directory: with-ptscotch=$PTSCOTCH_DIR])
       if test -d "$PTSCOTCH_DIR/lib"; then
         ZOLTAN_LIB_FLAGS="$ZOLTAN_LIB_FLAGS -L$PTSCOTCH_DIR/lib"
         if (test -f "$SCOTCH_DIR/lib/libptscotch.a" || test -f "$SCOTCH_DIR/lib/libptscotch.so" || test -f "$SCOTCH_DIR/lib/libptscotch.dylib"); then
           SCOTCH_LIBS="-lptscotch -lptscotcherr -lptscotcherrexit $SCOTCH_LIBS"
           AC_DEFINE([HAVE_SCOTCH],[1],["Define if configured with Parallel Scotch library partitioning support."])
         else
           AC_MSG_ERROR([Scotch libraries not found at $SCOTCH_DIR/lib])
         fi
       else
         ZOLTAN_LIB_FLAGS="$ZOLTAN_LIB_FLAGS -L$PTSCOTCH_DIR"
         if (test -f "$SCOTCH_DIR/libptscotch.a" || test -f "$SCOTCH_DIR/libptscotch.so" || test -f "$SCOTCH_DIR/libptscotch.dylib"); then
           SCOTCH_LIBS="-lptscotch -lptscotcherr -lptscotcherrexit $SCOTCH_LIBS"
         else
           AC_MSG_ERROR([Scotch libraries not found at $SCOTCH_DIR])
         fi
       fi
       if test -d "$PTSCOTCH_DIR/include"; then
         ZOLTAN_INC_FLAGS="$ZOLTAN_INC_FLAGS -I$PTSCOTCH_DIR/include"
       else
         ZOLTAN_INC_FLAGS="$ZOLTAN_INC_FLAGS -I$PTSCOTCH_DIR"
       fi
       DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-ptscotch=\"${PTSCOTCH_DIR}\""
       ;;
  esac
fi

################################################################################
#                            Optional Libraries
################################################################################

AC_ARG_WITH([vtk],
  [AS_HELP_STRING([--with-vtk=DIR],[Enable VTK support, and specify VTK location])],
  [WITH_VTK=$withval
   DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-vtk=\"${withval}\""
  ],[WITH_VTK=])

VTKMOAB_DEFAULT=no
if (test "x$WITH_VTK" != "xno" && test "x$WITH_VTK" != "x"); then
  VTKMOAB_DEFAULT=yes
fi

AC_ARG_WITH( [vtk-lib-suffix],
             [AS_HELP_STRING([--with-vtk-lib-suffix=SUFFIX],[Specify optional suffix to VTK library names, e.g. -pv3.98])],
             [case "x$withval" in
               xyes)
                 AC_MSG_ERROR([Must specify a non-empty suffix!])
		             VTK_LIB_SUFFIX=
                 ;;
               xno)
                 AC_MSG_ERROR([Must specify a non-empty suffix!])
		             VTK_LIB_SUFFIX=
                 ;;
               x*)
		             VTK_LIB_SUFFIX="$withval"
                 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-vtk-lib-suffix=\"${withval}\""
                 ;;
               esac],[VTK_LIB_SUFFIX=] )

################################################################################
#                           Zoltan - Part 2 of 2
################################################################################

#if test "xyes" = "x$ENABLE_mbpart"; then
#  WITH_ZOLTAN=yes
#else
#  WITH_ZOLTAN=no
#fi

# If user requested Zoltan support, verify that we can
# find the library and headers, and set the necessary
# build variables.
if test "xyes" = "x$WITH_ZOLTAN"; then
  if test "xyes" != "x$enablempi"; then
    AC_MSG_WARN([Cannot include Zoltan support without MPI (--with-mpi). Disabling Zoltan support.])
    WITH_ZOLTAN=no
  fi

  if test "xyes" = "x$WITH_ZOLTAN"; then
      # Test for zoltan headers
    AC_LANG_PUSH([C++])
    old_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $ZOLTAN_INC_FLAGS -DMPICH_IGNORE_CXX_SEEK"
    AC_CHECK_HEADER([zoltan_cpp.h],,
                    [WITH_ZOLTAN=no
                    AC_MSG_WARN([zoltan_cpp.h: header not found in $ZOLTAN_INC_FLAGS .])],
                    [#include <stdlib.h>])
    CPPFLAGS="$old_CPPFLAGS"
    AC_LANG_POP([C++])

    if test "xyes" = "x$WITH_ZOLTAN"; then
      # Build LDFLAGS for Zoltan
      found=no
      if test "x" != "x$ZOLTAN_DIR"; then
        if test "x" != "x$ZOLTAN_ARCH" && test -d "$ZOLTAN_DIR/Obj_$ZOLTAN_ARCH"; then
          ZOLTAN_LIB_FLAGS="$ZOLTAN_LIB_FLAGS -L$ZOLTAN_DIR/Obj_$ZOLTAN_ARCH"
          found=yes
        elif test -d "$ZOLTAN_DIR/lib"; then
          ZOLTAN_LIB_FLAGS="$ZOLTAN_LIB_FLAGS -L$ZOLTAN_DIR/lib"
          found=yes
        else
          for d in $ZOLTAN_DIR/Obj_*; do
            if test -d "$d"; then
              found=yes
              ZOLTAN_LIB_FLAGS="$ZOLTAN_LIB_FLAGS -L$d"
            fi
          done
        fi
        if test "no" = "$found"; then
          ZOLTAN_LIB_FLAGS="$ZOLTAN_LIB_FLAGS -L$ZOLTAN_DIR"
        fi
      fi

      # Test for Zoltan library, possibly requiring parmetis and scotch
      old_LDFLAGS="$LDFLAGS"
      LDFLAGS="$LDFLAGS $ZOLTAN_LIB_FLAGS"
      AC_CHECK_LIB([zoltan],[Zoltan_LB_Partition],[ZOLTAN_LIBS='-lzoltan'],
        [unset ac_cv_lib_zoltan_Zoltan_LB_Partition
        AC_CHECK_LIB([zoltan],[Zoltan_LB_Partition],
                      [ZOLTAN_LIBS="-lzoltan $PARMETIS_LIBS $METIS_LIBS"],
                      [unset ac_cv_lib_zoltan_Zoltan_LB_Partition
                      AC_CHECK_LIB([zoltan],[Zoltan_LB_Partition],[ZOLTAN_LIBS="-lzoltan $SCOTCH_LIBS $PARMETIS_LIBS $METIS_LIBS"],
                      [AC_MSG_ERROR([zoltan library not found or not usable.  Perhaps you need to specify --with-parmetis=@<:@DIR@:>@ or --with-metis=@<:@DIR@:>@ and/or --with-scotch=@<:@DIR@:>@?])],
                      [$SCOTCH_LIBS $PARMETIS_LIBS $METIS_LIBS])],
                      [$PARMETIS_LIBS $METIS_LIBS])])
      LDFLAGS="$old_LDFLAGS"

        # Define macro HAVE_ZOLTAN
      AC_DEFINE([HAVE_ZOLTAN],[1],["Define if configured with Zoltan library partitioning support."])
    fi
  fi
fi

AM_CONDITIONAL( HAVE_ZOLTAN, [test "x$WITH_ZOLTAN" = "xyes"] )
AC_SUBST(ZOLTAN_LIBS)
AC_SUBST(ZOLTAN_LIB_FLAGS)
AC_SUBST(ZOLTAN_INC_FLAGS)
EXPORT_LDFLAGS="$ZOLTAN_LIB_FLAGS $EXPORT_LDFLAGS"

################################################################################
#                           CGM
################################################################################
USING_DEV_CGM=no
AC_ARG_WITH(cgm,
[AS_HELP_STRING([--with-cgm=DIR],[Specify directory containing CGM])],
[CGM_DIR=$withval
 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-cgm=\"${withval}\""
 ], [CGM_DIR=])

case "x$CGM_DIR" in
  xno|x)
    CGM_MISSING=yes
    ;;
  xyes)
    AC_MSG_ERROR([--with-cgm requires an argument])
    ;;
  *)
    if ! test -d "$CGM_DIR"; then
      AC_MSG_ERROR([$CGM_DIR : not a directory.])
    fi
    CGM_MISSING=no
    CGM_MAKE=
    AC_CHECK_FILE([${CGM_DIR}/lib/cgm.make],
                  [CGM_MAKE="${CGM_DIR}/lib/cgm.make"],
                  [AC_CHECK_FILE([${CGM_DIR}/cgm.make],
                                 [CGM_MAKE="${CGM_DIR}/cgm.make"; USING_DEV_CGM=yes],
                                 [AC_MSG_ERROR([$CGM_DIR : not a configured CGM]);
                                  CGM_MISSING=yes])
                  ])
    ;;
esac

if test "x$CGM_MISSING" = "xno"; then
  CGM_CONFIG_OPTIONS="include $CGM_MAKE"

  FATHOM_MAKE_INC_VAR([$CGM_MAKE],[CGM_DEFINES], [CGM_CPPFLAGS="$make_val"])
  FATHOM_MAKE_INC_VAR([$CGM_MAKE],[CGM_INCLUDES],[CGM_CPPFLAGS="$CGM_CPPFLAGS $make_val"])
  FATHOM_MAKE_INC_VAR([$CGM_MAKE],[CGM_LTFLAGS], [CGM_LTFLAGS="$make_val"])
  FATHOM_MAKE_INC_VAR([$CGM_MAKE],[CGM_LDFLAGS], [CGM_LDFLAGS="$make_val"])
  FATHOM_MAKE_INC_VAR([$CGM_MAKE],[CGM_LIBS],    [CGM_LIBS="$make_val"])

  AC_LANG_PUSH([C++])
  old_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CGM_CPPFLAGS $CPPFLAGS"
  HAVE_OCC=no
  HAVE_FACET=no
  AC_MSG_CHECKING([for underlying geometry engine in CGM])
  AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([@%:@include <CGMConfig.h>],
  [ #ifndef CGM_HAVE_FACET_ENGINE_ONLY
  // Perfect!
  #else
  # error Only Facet engine available
  #endif
  ])
  ],
  [HAVE_OCC=yes
   AC_DEFINE(HAVE_CGM_OCC, [1], [MOAB uses CGM configured with OpenCascade])
   AC_MSG_RESULT(OCC)],
  [HAVE_FACET=yes
   AC_DEFINE(HAVE_CGM_FACET, [1], [MOAB uses CGM configured with Facet Engine only])
  AC_MSG_RESULT(Facets)] )
  CPPFLAGS="$old_CPPFLAGS"
  AC_LANG_POP([C++])

  AC_MSG_CHECKING([for GeometryQueryTool::ray_fire(RefFace*,...)])
  AC_LANG_PUSH([C++])
  old_CPPFLAGS="$CPPFLAGS"
  old_LDFLAGS="$LDFLAGS"
  old_LIBS="$LIBS"
  CPPFLAGS="$CGM_CPPFLAGS $CPPFLAGS"
  LDFLAGS="$CGM_LDFLAGS $CPPFLAGS"
  LIBS="$CGM_LIBS $LIBS"
  HAVE_CGM_FIRE_RAY=no
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include "GeometryQueryTool.hpp"
                      #include "CubitVector.hpp"],
                    [CubitVector a,b;
                    DLIList<double> c;
                    GeometryQueryTool::instance()->fire_ray((RefFace*)0, a, b, c );
                    ])],
    [HAVE_CGM_FIRE_RAY=yes;
    AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)])
  CPPFLAGS="$old_CPPFLAGS"
  LDFLAGS="$old_LDFLAGS"
  LIBS="$old_LIBS"
  AC_LANG_POP([C++])

  EXPORT_LDFLAGS="$EXPORT_LDFLAGS $CGM_LDFLAGS"
  EXPORT_LTFLAGS="$EXPORT_LTFLAGS $CGM_LTFLAGS"
fi

AC_SUBST(CGM_CPPFLAGS)
AC_SUBST(CGM_LDFLAGS)
AC_SUBST(CGM_LTFLAGS)
AC_SUBST(CGM_LIBS)
AC_SUBST(CGM_DIR)

AM_CONDITIONAL( HAVE_CGM, [test "x$CGM_MISSING" = "xno"] )
AM_CONDITIONAL( HAVE_OCC, [test "x$HAVE_OCC" = "xyes"] )

if (test "x$CGM_MISSING" = "xno"); then
  AC_DEFINE([HAVE_CGM],[1],["Define if configured with CGM support."])
  if (test "x$HAVE_CGM_FIRE_RAY" != "xno"); then
    AC_DEFINE([HAVE_CGM_FIRE_RAY],[1],["Define if configured with CGM and Ray fire support."])
  fi
fi


################################################################################
#                           iRel/Lasso Part 2 of 2
################################################################################

AC_MSG_CHECKING([if iRel support is to be built])
AC_MSG_RESULT([$ENABLE_irel])
AM_CONDITIONAL([ENABLE_irel],[test "xyes" = "x$ENABLE_irel"])

if test "x$ENABLE_irel" = "xyes"; then
  if (test "x$ENABLE_imesh" != "xyes" || test "x$CGM_MISSING" != "xno"); then
    AC_MSG_ERROR([Cannot build iRel API. Enable both iGeom (CGM) and iMesh implementations.])
  fi
  AC_DEFINE([HAVE_IREL],[1],[Define if configured with iRel interfaces.])

  AC_CHECK_FILE([${CGM_DIR}/lib/iGeom-Defs.inc],
              [IGEOM_CONFIG_OPTIONS="include ${CGM_DIR}/lib/iGeom-Defs.inc";
               FATHOM_MAKE_INC_VAR([${CGM_DIR}/lib/iGeom-Defs.inc],[IGEOM_DEFINES], [IGEOM_DEFINES="$make_val"],
                  [IGEOM_DEFINES=]) ],
              [AC_CHECK_FILE([${CGM_DIR}/itaps/iGeom-Defs.inc],
                             [IGEOM_CONFIG_OPTIONS="include ${CGM_DIR}/itaps/iGeom-Defs.inc"],
                             [AC_MSG_WARN([$CGM_DIR : iGeom is not configured correctly])]
                            )
              ])
  AC_SUBST(IGEOM_DEFINES)
  AC_SUBST(IGEOM_CONFIG_OPTIONS)
fi

HAVE_IGEOM="no"
HAVE_IMESH="no"
HAVE_IREL="no"
test "x$CGM_MISSING" != "xno" || HAVE_IGEOM="yes"
test "x$ENABLE_imesh" != "xyes" || HAVE_IMESH="yes"
test "x$ENABLE_irel" != "xyes" || HAVE_IREL="yes"
AC_SUBST(HAVE_IGEOM)
AC_SUBST(HAVE_IMESH)
AC_SUBST(HAVE_IREL)
AM_CONDITIONAL([USING_DEV_CGM],[test "xyes" = "x$USING_DEV_CGM"])

###############################################################################
#                           Optional Tools
###############################################################################
  # Define a macro to avoid typing this for each individual tool
  # Usage: MB_OPTIONAL_TOOL( name, default, default_if_tools_enabled )
  #  name      - name of option
  #  default   - yes/no, $ENABLE_TOOLS overrides this if set
  # Actions:
  #  sets ENABLE_${tool} to 'yes' or 'no'
  #  creates ENABLE_${tool} automake conditional
AC_DEFUN([MB_OPTIONAL_TOOL],[
  mb_default_$1=$2

  if test "x" != "x$ENABLE_TOOLS"; then
    mb_default_$1=$3
  fi
  AC_ARG_ENABLE( [$1],
[AS_HELP_STRING([--enable-$1],[Build tool: $1])
AS_HELP_STRING([--disable-$1],[Don't build $1])],
                 [ENABLE_$1=$enableval],[ENABLE_$1=${mb_default_$1}] )
  AM_CONDITIONAL([ENABLE_$1],[test "x${ENABLE_$1}" != "xno"])
  if (test "x${ENABLE_$1}" != "xno"); then
    AC_DEFINE(m4_toupper(HAVE_$1),[1],Configure with tool: m4_toupper($1))
  fi
  AC_MSG_CHECKING([if $1 is to be built])
  AC_MSG_RESULT([${ENABLE_$1}])
  DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-$1=\"${ENABLE_$1}\""
])
  # The default for all tools
AC_ARG_ENABLE( [tools], [
AS_HELP_STRING([--enable-tools],[Build all tools by default])
AS_HELP_STRING([--disable-tools],[Disable all tools by default])],
                         [ENABLE_TOOLS=$enableval; DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --enable-tools"],
                         [ENABLE_TOOLS=] )
  # Individual tools
MB_OPTIONAL_TOOL([mbconvert],    [yes], [yes])
MB_OPTIONAL_TOOL([hexmodops],    [yes], [yes])
MB_OPTIONAL_TOOL([vtkMOABReader],[${VTKMOAB_DEFAULT}], [${VTKMOAB_DEFAULT}])
MB_OPTIONAL_TOOL([mbsize],       [yes], [yes])
MB_OPTIONAL_TOOL([mbskin],       [yes], [yes])
MB_OPTIONAL_TOOL([mbtagprop],    [yes], [yes])
MB_OPTIONAL_TOOL([mbmem],        [yes], [yes])
MB_OPTIONAL_TOOL([spheredecomp], [yes], [yes])
MB_OPTIONAL_TOOL([mbsurfplot],   [yes], [yes])
if (test "x$WITH_ZOLTAN" != "xno" || test "x$METIS_DIR" != "x"); then
  # mbpart_deftools=no
  # test "x$METIS_DIR" != "x" || mbpart_deftools="yes"
  # MB_OPTIONAL_TOOL([mbpart],     [${mbpart_deftools}], [yes] )
  MB_OPTIONAL_TOOL([mbpart],     [yes], [yes] )
else
  MB_OPTIONAL_TOOL([mbpart],     [no], [no] )
fi

MB_OPTIONAL_TOOL([dagmc],        [no], [yes])
if test "xyes" = "x$ENABLE_dagmc"; then
   DAGMC_LIBS="-ldagmc"
   AC_SUBST(DAGMC_LIBS)
fi

MB_OPTIONAL_TOOL([gsets],        [yes], [yes])
MB_OPTIONAL_TOOL([mbmerge],        [yes], [yes])
MB_OPTIONAL_TOOL([mbdepth],      [yes], [yes])
if test "x$enablempi" != "xno"; then
  MB_OPTIONAL_TOOL([mbcoupler],    [no], [yes] )
  MB_OPTIONAL_TOOL([mcnpmit],      [no], [yes])
  MB_OPTIONAL_TOOL([refiner],      [no], [yes])
else
  AS_ECHO("Disabling mbcoupler and refiner since these tools require MPI/parallel configuration")
  MB_OPTIONAL_TOOL([mbcoupler],    [no], [no] )
  MB_OPTIONAL_TOOL([mcnpmit],      [no], [no])
  MB_OPTIONAL_TOOL([refiner],      [no], [no])
fi
MB_OPTIONAL_TOOL([h5mtools],     [${enablehdf5}], [${enablehdf5}])
MB_OPTIONAL_TOOL([mbcslam],      [no], [yes])
MB_OPTIONAL_TOOL([mbquality],    [yes], [yes])

MB_OPTIONAL_TOOL([ahf],          [no], [no])
MB_OPTIONAL_TOOL([mbumr],        [yes], [yes])
if test "xyes" = "x$ENABLE_ahf"; then
  AC_DEFINE([HAVE_AHF],[1],[Enable use of AHF data-structures for querying adjacency information])
fi

AM_CONDITIONAL(ENABLE_AHF, [test "xyes" = "x$ENABLE_ahf"])

if test "xyes" = "x$ENABLE_vtkMOABReader"; then
  if test "xno" = "x$WITH_VTK"; then
    AC_MSG_ERROR([vtkMOABReader requires VTK])
  elif test "x" = "x$WITH_VTK"; then
    WITH_VTK=yes
  fi
fi

dnl Define a function that checks for the existence of a library
dnl and if it exists, prepends the corresponding link flag to a
dnl sepecified variable
dnl Arguments:
dnl  - The library name
dnl  - The name of the variable to which to prepend the library link flag
dnl  - List of extra libraries required for link test
dnl  - Optional library path linker argument (e.g. -L/opt/foo)
  AC_DEFUN([FATHOM_CHECK_LIB_LIST],[
    old_LIBS="$LIBS"
    LIBS="$5 ${$3} $old_LIBS"
    AC_CHECK_LIB([$1],[main],[$3="-l$1 ${$3}"],[],[$4])
    ac_cv_lib_$1=ac_cv_lib_$1_main

    LIBS="$old_LIBS"
  ])

dnl Define a function that checks for the existence of a library
dnl and if it exists, prepends the corresponding link flag to a
dnl sepecified variable
dnl Arguments:
dnl  - The library name
dnl  - The name of the variable to which to prepend the library link flag
dnl  - List of extra libraries required for link test
dnl  - Optional library path linker argument (e.g. -L/opt/foo)
  AC_DEFUN([FATHOM_CHECK_VTK_LIB_LIST],[
    old_LIBS="$LIBS"
    LIBS="$5 ${$3} $old_LIBS"
    AC_CHECK_LIB([$1$VTK_LIB_SUFFIX], main,
       [$3="-l$1$VTK_LIB_SUFFIX ${$3}"],
       [],[$4])dnl
    LIBS="$old_LIBS"
  ])


################################################################################
#                    VTK for vtkMOABReader
################################################################################
AC_DEFUN([FATHOM_MERGE_LISTS],[
  $1="$2"
  # Reverse list two
  for item3 in $3; do
    found=no
    for item2 in $2; do
      if test "x$item2" = "x$item3"; then found=yes; fi
    done
    if test "$found" = "no"; then $1="${$1} $item3"; fi
  done
  $1="${$1} $2"
])

if  (test "x$WITH_VTK" != "xno" && test "x$WITH_VTK" != "x"); then
  AC_PATH_X
  AC_PATH_XTRA
  if test "xyes" = "x$no_x"; then
    AC_MSG_ERROR([X11 not found])
  fi

  AC_LANG_PUSH([C++])

  VTK_INCLUDES=
  case "x$WITH_VTK" in
    xyes|x)
      VTK_LIB_DIR=
      VTK_INC_DIR=
      for dir in  /usr/local/include/vtk /usr/local/include/vtk-* /usr/include/vtk  /usr/include/vtk-*; do
        if test "x" = "x$VTK_INC_DIR"; then
          AC_CHECK_FILE([${dir}/vtkUnstructuredGrid.h],[VTK_INC_DIR="-I$dir"])
        fi
      done
      if test x = x"$VTK_INC_DIR"; then
        AC_CHECK_HEADER([vtkUnstructuredGrid.h],[],
                        [AC_MSG_ERROR([VTK includes not found])])
      else
        VTK_INCLUDES="$VTK_INC_DIR"
      fi
      old_LIBS="$LIBS"
      for dir in /usr/lib/vtk /usr/lib /usr/bin /usr; do
        if test "x" = "x$VTK_LIB_DIR"; then
          LIBS="$old_LIBS -L$dir"
          FATHOM_CHECK_VTK_LIB_LIST([vtkCommonCore],[$VTK_LIB_SUFFIX], [VTK_LIB_DIR],[],[-L$dir])
        fi
      done
      if test "x" = "x$VTK_LIB_DIR"; then
        AC_MSG_ERROR([VTK libraries not found])
      fi
      LIBS="$old_LIBS"
      ;;
    x?*)
      VTK_LIB_DIR=
      VTK_INC_DIR=
      for dir in ${WITH_VTK}/include/vtk* ${WITH_VTK}/include ${WITH_VTK}/include/paraview* ${WITH_VTK}/VTK ${WITH_VTK}; do
        if test "x" = "x$VTK_INC_DIR"; then
          AC_CHECK_FILE([${dir}/vtkUnstructuredGrid.h],[VTK_INC_DIR="-I$dir"])
        fi
      done
      if test "x" = "x$VTK_INC_DIR"; then
        # Check for non-installed VTK source
        AC_CHECK_FILE([vtkUnstructuredGrid.h],[
          for subdir in . Common Common/DataModel Filtering GenericFiltering Graphics Hybrid Imaging Parallel Patented Rendering Utilities Utilities/vtkexpat Utilities/expat Utilities/DICOMParser Utilties/vtkpng Utilities/vtkzlib Utilities/freetype/include/freetype Wrapping; do
        	  AC_MSG_CHECKING([for directory ${WITH_VTK}/$subdir])
        	  if test -d ${WITH_VTK}/$subdir; then
        	    VTK_INC_DIR="$VTK_INC_DIR -I${WITH_VTK}/$subdir"
        	    AC_MSG_RESULT([yes])
        	  else
        	    AC_MSG_RESULT([no])
        	  fi
        	done
        	],[AC_MSG_ERROR([VTK includes not found])]
        )
      fi
      VTK_INCLUDES="$VTK_INC_DIR"
      old_LIBS="$LIBS"
      for dir in ${WITH_VTK}/lib ${WITH_VTK}/lib/vtk ${VTK_DIR}/bin ${WITH_VTK}/bin ${WITH_VTK}; do
        if test "x" = "x$VTK_LIB_DIR"; then
          LIBS="$old_LIBS -L$dir"
          FATHOM_CHECK_VTK_LIB_LIST([vtkCommonCore],[$VTK_LIB_SUFFIX], [VTK_LIB_DIR],[],[-L$dir])
        fi
      done
      if test "x" = "x$VTK_LIB_DIR"; then
        AC_MSG_ERROR([VTK libraries not found])
      else
        AC_DEFINE([HAVE_VTK],[1],["Define if configured with VTK I/O library support."])
      fi
      LIBS="$old_LIBS"
      ;;
  esac

   # These are ordered such that a given library depends only on those libraries
   # that preceed it in the list.
  FATHOM_CHECK_LIB_LIST([pthread],[AC_DEFINE([HAVE_PTHREAD],[1],["Define if configured with pthread support."])],[vtk_sys_libs])
  FATHOM_CHECK_LIB_LIST([dl],[AC_DEFINE([HAVE_LDL],[1],["Define if configured with dynamic library loading support."])],     [vtk_sys_libs])
  FATHOM_CHECK_LIB_LIST([m],[AC_DEFINE([HAVE_LM],[1],["Define if configured with math library support."])],      [vtk_sys_libs])
  FATHOM_CHECK_LIB_LIST([gcc],[],    [vtk_sys_libs])
  x_libs_arg="$X_EXTRA_LIBS -lX11 $X_PRE_LIBS"
  FATHOM_CHECK_LIB_LIST([Xext],[],[vtk_x_libs],[$x_libs_arg],[-L$x_libraries])
  FATHOM_CHECK_LIB_LIST([ICE],[], [vtk_x_libs],[$x_libs_arg],[-L$x_libraries])
  FATHOM_CHECK_LIB_LIST([SM],[],  [vtk_x_libs],[$x_libs_arg],[-L$x_libraries])
  FATHOM_CHECK_LIB_LIST([Xt],[],  [vtk_x_libs],[$x_libs_arg],[-L$x_libraries])
  FATHOM_CHECK_LIB_LIST([GL],[AC_DEFINE([HAVE_GL],[1],["Define if configured with openGL library support."])],  [vtk_x_libs],[$x_libs_arg],[-L$x_libraries])
  FATHOM_CHECK_LIB_LIST([GLU],[], [vtk_x_libs],[$x_libs_arg],[-L$x_libraries])
  vtk_libs_arg="-L$x_libraries $vtk_x_libs $X_EXTRA_LIBS -lX11 $X_PRE_LIBS $vtk_sys_libs"

  FATHOM_CHECK_VTK_LIB_LIST([vtksys],[$VTK_LIB_SUFFIX],        [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkCommonCore],[$VTK_LIB_SUFFIX], [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkDICOMParser],[$VTK_LIB_SUFFIX],[vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkfreetype],[$VTK_LIB_SUFFIX],   [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkexpat],[$VTK_LIB_SUFFIX],      [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkzlib],[$VTK_LIB_SUFFIX],       [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkpng],[$VTK_LIB_SUFFIX],        [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkjpeg],[$VTK_LIB_SUFFIX],       [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtktiff],[$VTK_LIB_SUFFIX],       [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkftgl],[$VTK_LIB_SUFFIX],       [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkFiltersCore],[$VTK_LIB_SUFFIX],  [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkImagingCore],[$VTK_LIB_SUFFIX],    [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([verdict],[$VTK_LIB_SUFFIX],       [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkGraphics],[$VTK_LIB_SUFFIX],   [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkNetCDF],[$VTK_LIB_SUFFIX],     [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkexoIIc],[$VTK_LIB_SUFFIX],     [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkmetaio],[$VTK_LIB_SUFFIX],     [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkIOCore],[$VTK_LIB_SUFFIX],         [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkRenderingCore],[$VTK_LIB_SUFFIX],  [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  FATHOM_CHECK_VTK_LIB_LIST([vtkParallelCore],[$VTK_LIB_SUFFIX],   [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])
  #FATHOM_CHECK_VTK_LIB_LIST([vtkHybrid],[$VTK_LIB_SUFFIX],     [vtk_libs],[$vtk_libs_arg],[-L$VTK_LIB_DIR])

   # Make sure we found at least vtkCommonCore and vtkRendering
  if ! echo "x $vtk_libs" | grep lvtkCommonCore >/dev/null; then
    AC_MSG_ERROR("VTK library 'vtkCommonCore' not found.")
  fi

  AC_LANG_POP([C++])

  FATHOM_MERGE_LISTS([XLIBS_tmp],[$X_EXTRA_LIBS -lX11 $X_PRE_LIBS],[$vtk_x_libs])
  FATHOM_MERGE_LISTS([XLIBS],[$vtk_sys_libs],[$XLIBS_tmp])
  VTK_LIBS="$LIBS -L$VTK_LIB_DIR $vtk_libs $XLIBS"
  VTK_INCLUDES="$VTK_INCLUDES"
  AC_SUBST(VTK_LIBS)
  AC_SUBST(VTK_INCLUDES)
  AC_SUBST(VTK_LIB_DIR)
  AC_SUBST(VTK_INC_DIR)
fi

################################################################################
#                           Output Files
################################################################################
rm -r -f a.out

# --------------------------------------------------------------
# Print configuration info to the header file
# --------------------------------------------------------------
AC_DEFINE_UNQUOTED(CONFIGURE_INFO, ["$0 run on `date`"], [Configuration information.])
AC_DEFINE_UNQUOTED(CONFIGURE_COMMAND, ["$USER_CONFIGURE_CMD"], [Configuration command along with user-specified options.])
AC_SUBST(USER_CONFIGURE_CMD)

# Some old versions of autoconf don't define docdir.
# Define it if autoconf did not.
if test "x" = "x$docdir"; then
  docdir='${datadir}/doc/moab'
  AC_SUBST(docdir)
fi

# Let us define examplesdir.
# Define it if autoconf did not already.
if test "x" = "x$examplesdir"; then
  examplesdir='${datadir}/examples'
  AC_SUBST(examplesdir)
fi

CPPFLAGS="$MB_CPPFLAGS"
FCPPFLAGS=`echo $MB_CPPFLAGS | sed -e "s/-D/$FC_DEFINE/g"`
FCDEFS=`echo $MB_DEFS | sed -e "s/-D/$FC_DEFINE/g"`
AC_SUBST([CPPFLAGS])
AC_SUBST([FCPPFLAGS])
AC_SUBST([MB_DEFS])
AC_SUBST([FCDEFS])
AM_LDFLAGS="$AM_LDFLAGS $EXPORT_LTFLAGS $EXPORT_LDFLAGS"
AC_SUBST([AM_LDFLAGS])
AC_SUBST([EXPORT_LTFLAGS])
AC_SUBST([EXPORT_LDFLAGS])

AC_ARG_VAR([FC], [FORTRAN compiler command])
# AC_CONFIG_HEADERS([config.h])
# Create a temporary file with e.g. HAVE_MPI
#AC_CONFIG_HEADERS([.MOABConfig.h:config/MOABConfig.h.in])
# Prefixes everything in the temporary file with MOAB_, e.g. MOAB_HAVE_MPI
#AX_PREFIX_CONFIG_H([src/moab/MOABConfig.h],[MOAB],[.MOABConfig.h])

AC_OUTPUT_COMMANDS( [if test -f src/moab_export.h; then true; else echo "#define MOAB_EXPORT" > src/moab_export.h; fi] )
AC_OUTPUT_COMMANDS( [if test -f itaps/imesh/imesh_export.h; then true; else echo "#define IMESH_EXPORT" > itaps/imesh/imesh_export.h; fi] )
AC_OUTPUT_COMMANDS( [if test -f itaps/irel/irel_export.h; then true; else echo "#define IREL_EXPORT" > itaps/irel/irel_export.h; fi] )
AC_OUTPUT_COMMANDS( [if test -f tools/refiner/refiner_export.h; then true; else echo "#define REFINER_EXPORT" > tools/refiner/refiner_export.h; fi] )
AC_OUTPUT_COMMANDS( [if test -f tools/dagmc/dagmc_export.h; then true; else echo "#define DAGMC_EXPORT" > tools/dagmc/dagmc_export.h; fi] )
AC_CONFIG_HEADERS([src/moab/Version.h
                   src/moab/EntityHandle.hpp
                   src/parallel/moab_mpi_config.h
                   src/moab/MOABConfig.h:config/MOABConfig.h.in])
AX_PREFIX_CONFIG_H([src/moab/MOABConfig.h],[MOAB],[src/moab/MOABConfig.h])

AC_CONFIG_FILES([Makefile
                 moab.make
                 MOABConfig.cmake
                 examples/makefile
                 examples/old/deform_mesh/makefile
                 src/Makefile
                 src/io/Makefile
                 src/io/mhdf/Makefile
                 src/LocalDiscretization/Makefile
                 src/RefineMesh/Makefile
                 src/parallel/Makefile
                 src/verdict/Makefile
                 test/Makefile
                 test/dagmc/Makefile
                 test/h5file/Makefile
                 test/dual/Makefile
                 test/obb/Makefile
                 test/perf/Makefile
                 test/perf/point_location/Makefile
                 test/io/Makefile
                 test/parallel/Makefile
                 itaps/iBase_f.h
                 itaps/Makefile
                 itaps/imesh/Makefile
                 itaps/imesh/iMesh-Defs.inc
                 itaps/fbigeom/Makefile
                 itaps/fbigeom/FBiGeom-Defs.inc
                 itaps/irel/Makefile
                 itaps/irel/iRel-Defs.inc
                 tools/Makefile
                 tools/refiner/Makefile
                 tools/mbcoupler/Makefile
                 tools/mbcslam/Makefile
                 tools/mcnpmit/Makefile
                 tools/dagmc/Makefile
                 tools/vtkMOABReader/CMakeLists.txt
                 doc/user.dox
                 doc/config.tex
                 MeshFiles/Makefile
                 MeshFiles/unittest/Makefile
                 MeshFiles/unittest/io/Makefile
                 MeshFiles/unittest/fbigeom/Makefile
                 MeshFiles/unittest/irel/Makefile
                 MeshFiles/unittest/dagmc/Makefile
                 MeshFiles/unittest/mbcslam/Makefile
                 MeshFiles/unittest/h5file/Makefile
                 MeshFiles/examples/Makefile
                 ])

# DISTCHECK_CONFIGURE_FLAGS="$USER_CONFIGURE_CMD"
# make distcheck will use this DISTCHECK_CONFIGURE_FLAGS
# variable to be passed to configure line
# some people include a lot of env vars here
# I think we need only those; only distcheck is affected by this
## original is commented out below; we use only a few
## for var in CC CFLAGS CPPFLAGS CXX CXXCPP LDFLAGS LIBS F90 FC F77; do
for var in CC CXX F90 FC F77 LDFLAGS LIBS; do
  eval isset=\${$var+set}
  if test "$isset" = 'set' ; then
    eval val=$`echo $var`
    DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS  $var=\"$val\""
  fi
done

AC_SUBST([DISTCHECK_CONFIGURE_FLAGS])

dnl Before configuring libtool check for --enable-all-static option
AC_ARG_ENABLE(all-static,
  AS_HELP_STRING([--enable-all-static],
                 [Build completely static (standalone) binaries.]),
  [
    if ( test "x$enable_static" != "xno" ); then
      if ( test "$enableval" = "yes" ); then
        LDFLAGS="-all-static $LDFLAGS"
      fi
    else
      AC_MSG_WARN([ Static linking disabled. Ignoring --enable-all-static option. Please add --enable-static also. ])
    fi
])

AC_OUTPUT

AC_MSG_RESULT([C:   $CC $CFLAGS $CPPFLAGS])
AC_MSG_RESULT([C++: $CXX $CXXFLAGS $CPPFLAGS])
if (test "xno" != "x$ENABLE_FORTRAN"); then
  AC_MSG_RESULT([Fortran90: $FC $FCFLAGS $FCPPFLAGS])
fi
if (test "xno" != "x$ENABLE_FORTRAN"); then
  AC_MSG_RESULT([Fortran77: $F77 $FFLAGS $FCPPFLAGS])
fi
AC_MSG_RESULT([Linker: $LD $EXPORT_LDFLAGS $CXX_LDFLAGS $LDFLAGS -L${PWD}/src/.libs -lMOAB $PNETCDF_LIBS $NETCDF_LIBS $CGNS_LIBS $HDF5_LIBS $CCMIO_LIBS $CGM_LIBS $ZOLTAN_LIBS $PARMETIS_LIBS $METIS_LIBS $LIBS])

if test "x$enablehdf5" = "xno"; then
  AC_MSG_WARN([
*************************************************************************
*            MOAB has been configured w/out the HDF5 library.
*              Support for native file format is disabled!
*************************************************************************])
fi

if test "x$enablenetcdf" = "xno"; then
  AC_MSG_WARN([
*************************************************************************
*           MOAB has been configured w/out the NetCDF library.
*     Support for ExodusII/Genesis and other file formats is disabled!
*************************************************************************])
elif test "xno" = "x$NETCDF_SUFFICIENT_DIMS_VARS"; then
  AC_MSG_WARN([
*************************************************************************
*  The configured NetCDF library does not have sufficient constant limits
*  to support reading of ExodusII with large numbers of blocks or sidesets
*  or nodesets.  Please consider re-compiling your NetCDF library after
*  increasing the constants NC_MAX_DIMS and NC_MAX_VARS in netcdf.h to
*  65536 and 524288, respectively.  See:
*  http://cubit.sandia.gov/help-version11.1/finite_element_model/export/exporting_exodus2_file.htm
*************************************************************************])
fi

if test "x$WARN_PARALLEL_HDF5" = "xyes"; then
  AC_MSG_WARN([
*************************************************************************
*        MOAB has been configured with parallel and HDF5 support
*     but the configured HDF5 library does not support parallel IO.
*            Some parallel IO capabilities will be disabled.
*************************************************************************])
fi

if test "x$WARN_PARALLEL_HDF5_NO_COMPLEX" = "xyes"; then
  AC_MSG_WARN([
*************************************************************************
*     Your parallel HDF5 library is configured without
*     H5_MPI_COMPLEX_DERIVED_DATATYPE_WORKS .  For the types of IO
*     patterns MOAB typically does this will result in degrading
*     collective IO calls to independent IO, which may have a very
*     significant impact on IO performance.
*************************************************************************])
fi

# define([ConfigurationSuggestion], [`esyscmd(config/suggest_configure_command)'])
# m4_echo(echo "m4_expand([ConfigurationSuggestion])")
