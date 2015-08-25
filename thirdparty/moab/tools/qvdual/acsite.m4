#######################################################################################
# Implement checks for C and C++ compilers, with all corresponding options
#
# Sets the following variables:
#  CPP      - The C preprocessor
#  CC       - The C compiler
#  CXX      - The C++ compiler
#  CFLAGS   - C compiler flags
#  CXXFLAGS - C++ compiler flags
#  WITH_MPI - 'yes' if parallel support, 'no' otherwise
#
#######################################################################################
AC_DEFUN([SNL_CHECK_COMPILERS], [

  # Check for Parallel
  # Need to check this early so we can look for the correct compiler
AC_ARG_WITH( [mpi], AC_HELP_STRING([[--with-mpi(=DIR)]], [Enable parallel support]),
             [WITH_MPI=$withval],[WITH_MPI=no] )
case "x$WITH_MPI" in
  xno)
    AC_PROG_CC( [cc gcc cl egcs] )
    AC_PROG_CXX( [CC aCC cxx xlC xlC_r pgCC c++ g++ gpp cc++ cl FCC KCC RCC] )
    ;;
  xyes)
    AC_PROG_CC( [mpicc mpcc] )
    AC_PROG_CXX([mpiCC mpCC] )
    ;;
  x*)
    if test -z "$CC";then
      for prog in mpicc mpcc; do
        if test -x ${WITH_MPI}/bin/$prog; then
          CC="${WITH_MPI}/bin/$prog"
        fi
      done
    fi
    if test -z "$CXX";then
      for prog in mpicxx mpiCC mpCC; do
        if test -x ${WITH_MPI}/bin/$prog; then
          CXX="${WITH_MPI}/bin/$prog"
        fi
      done
    fi
    AC_PROG_CC( [mpicc mpcc] )
    AC_PROG_CXX([mpiCC mpCC mpicxx] )
    WITH_MPI=yes
    ;;
esac

AC_PROG_CXXCPP
# Try to determine compiler-specific flags.  This must be done
# before setting up libtool so that it can override libtool settings.
SNL_CC_FLAGS
SNL_CXX_FLAGS
CFLAGS="$SNL_CC_SPECIAL"
CXXFLAGS="$SNL_CXX_SPECIAL"

# On IBM/AIX, the check for OBJEXT fails for the mpcc compiler.
if test "x$OBJEXT" = "x"; then
  OBJEXT=o
fi

  # Check for debug flags
AC_ARG_ENABLE( debug, AC_HELP_STRING([--enable-debug],[Debug symbols (-g)]),
               [enable_debug=$enableval], [enable_debug=] )  
AC_ARG_ENABLE( optimize, AC_HELP_STRING([--enable-optimize],[Compile optimized (-O2)]),
               [enable_optimize=$enableval], 
	       [enable_optimize=
		if test "x" == "x$enable_debug"; then
		  enable_optimize=yes
		fi ] )

# Choose compiler flags from CLI args
if test "xyes" = "x$enable_debug"; then
  CXXFLAGS="$CXXFLAGS -g"
  CFLAGS="$CLFAGS -g"
fi
if test "xyes" = "x$enable_optimize"; then
  CXXFLAGS="$CXXFLAGS -O2"
  CFLAGS="$CFLAGS -O2 -DNDEBUG"
fi

  # Check for 32/64 bit.
  # This requires SNL_CXX_FLAGS and SNL_CC_FLAGS to have been called first
AC_ARG_ENABLE( 32bit, AC_HELP_STRING([--enable-32bit],[Force 32-bit objects]),
[
  if test "xyes" != "x$enableval"; then
    AC_MSG_ERROR([Unknown argument --enable-32bit=$enableval])
  elif test "x" = "x$SNL_CXX_32BIT"; then
    AC_MSG_ERROR([Don't know how to force 32-bit C++ on this platform.  Try setting CXXFLAGS manually])
  elif test "x" = "x$SNL_CC_32BIT"; then
    AC_MSG_ERROR([Don't know how to force 32-bit C on this platform.  Try setting CFLAGS manually])
  fi
  CXXFLAGS="$CXXFLAGS $SNL_CXX_32BIT"
  CFLAGS="$CFLAGS $SNL_CC_32BIT"
  enable_32bit=yes
])
# This requires SNL_CXX_FLAGS and SNL_CC_FLAGS to have been called first
AC_ARG_ENABLE( 64bit, AC_HELP_STRING([--enable-64bit],[Force 64-bit objects]),
[
  if test "xyes" != "x$enableval"; then
    AC_MSG_ERROR([Unknown argument --enable-64bit=$enableval])
  elif test "x" = "x$SNL_CXX_64BIT"; then
    AC_MSG_ERROR([Don't know how to force 64-bit C++ on this platform.  Try setting CXXFLAGS manually])
  elif test "x" = "x$SNL_CC_64BIT"; then
    AC_MSG_ERROR([Don't know how to force 64-bit C on this platform.  Try setting CFLAGS manually])
  elif test "xyes" = "x$enable_32bit"; then
    AC_MSG_ERROR([Cannot do both --enable-32bit and --enable-64bit])
  fi
  CXXFLAGS="$CXXFLAGS $SNL_CXX_64BIT"
  CFLAGS="$CFLAGS $SNL_CC_64BIT"
])

]) # SNL_CHECK_COMPILERS



#######################################################################################
# Check for existance of new no-file-extension C++ headers.
# Conditinionally sets the following preprocessor macros:
#   CANT_USE_STD
#    - The new no-extension versions of the c++ headers do not
#      exist or do not define symbols to be in the "std" namespace.
#      For example: if this is defined #include <map.h> rather than <map>.
#   CANT_USE_STD_IO
#    - The new no-extension versions of the c++ IO headers do not
#      exist or do not define symbols to be in the "std" namespace.
#      For example: if this is defined, #include <iostrea.h> rather than <iostream>
#######################################################################################
AC_DEFUN([SNL_CANT_USE_STD], [
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_MSG_CHECKING([for C++-standard header files])
AC_TRY_COMPILE([#include <vector>
                #include <string>
                #include <map>
                #include <algorithm>
               ],
               [std::vector<std::string> v;
                std::map<std::string,std::string> m;
               ],
               [AC_MSG_RESULT(yes)],
               [AC_MSG_RESULT(no); CANT_USE_STD=-DCANT_USE_STD])

AC_MSG_CHECKING([for C++-standard I/O header files])
AC_TRY_COMPILE([#include <iosfwd>
                #include <iostream>
                #include <ostream>
                #include <sstream>
               ],
               [std::cout << std::endl;],
               [AC_MSG_RESULT(yes)],
               [AC_MSG_RESULT(no); CANT_USE_STD_IO=-DCANT_USE_STD_IO])

AC_LANG_RESTORE
]) # SNL_CANT_USE_STD




#######################################################################################
# Check if template definitions (.cpp files) must be
# included (in the .hpp files).
# Sets TEMPLATE_DEFS_INCLUDED=1
#######################################################################################
AC_DEFUN([SNL_TEMPLATE_DEFS_INCLUDED], [
AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_MSG_CHECKING([if template definitions should be included])

src=conftest.cc
templ=conftemp.cc
exe=conftest

echo "template <typename T> class MyTempl { public: T get() const; };" >$templ
echo "template <typename T> T MyTempl<T>::get() const { return 0; }"  >>$templ
echo "template <typename T> class MyTempl { public: T get() const; };" >$src
echo "int main( ) { MyTempl<int> c; return c.get(); }"                >>$src
if $CXX $CXXFLAGS $LDFLAGS $src $templ -o $exe >/dev/null 2>/dev/null; then
  TEMPLATE_DEFS_INCLUDED=
  AC_MSG_RESULT(no)
else
  TEMPLATE_DEFS_INCLUDED=-DTEMPLATE_DEFS_INCLUDED
  AC_MSG_RESULT(yes)
fi

rm -f $src $templ $exe
AC_LANG_RESTORE
]) # SNL_TEMPLATE_DEFS_INCLUDED




#######################################################################################
# Check for HDF5 library and related stuff
# Sets HAVE_HDF5 to 'yes' or 'no'
# If HAVE_HDF5 == yes, then updates INCLUDES and LIBS accordingly.
#######################################################################################
AC_DEFUN([SNL_CHECK_HDF5],[

  # CLI option for linking zlib
AC_ARG_WITH(zlib,
  [AC_HELP_STRING([--with-zlib=DIR],[HDF5 requires zlib, and zlib can be found at...])],
  [WITH_ZLIB=$withval],[WITH_ZLIB=])
case "x$WITH_ZLIB" in
  xyes|xno|x)
    ;;
  *)
    if ! test -d  ${WITH_ZLIB}/lib; then
      AC_MSG_ERROR([Not a directory: ${WITH_ZLIB}/lib])
    fi
    LIBS="$LIBS -L${WITH_ZLIB}/lib"
    ;;
esac
HAVE_ZLIB=no
if test "x$WITH_ZLIB" != "xno"; then
  AC_CHECK_LIB([z],[deflate],[HAVE_ZLIB=yes],
    [if test "x$WITH_ZLIB" != "x"; then AC_MSG_ERROR([Could not find zlib]); fi])
fi

  # CLI option for linking szip
AC_ARG_WITH(szip,
  [AC_HELP_STRING([--with-szip=DIR],[HDF5 requires szip, and szip an be found at...])],
  [WITH_SZIP=$withval],[WITH_SZIP=])
case "x$WITH_SZIP" in
  xyes|xno|x)
    ;;
  *)
    if ! test -d  ${WITH_SZIP}/lib; then
      AC_MSG_ERROR([Not a directory: ${WITH_SZIP}/lib])
    fi
    LIBS="$LIBS -L${WITH_SZIP}/lib"
    ;;
esac
HAVE_SZIP=no
if test "x$WITH_SZIP" != "xno"; then
  AC_CHECK_LIB([sz],[SZ_Decompress],[HAVE_SZIP=yes],
    [if test "x$WITH_SZIP" != "x"; then AC_MSG_ERROR([Could not find libsz]); fi])
fi

  # CLI option for extra HDF5 link flags
AC_ARG_WITH([--with-hdf5-ldflags],[AC_HELP_STRING([--with-hdf5-ldflags=...],
 [Extra LDFLAGS required for HDF5 library (e.g. parallel IO lib)])],
 [HDF5_LDFLAGS="$withval"],[HDF5_LDFLAGS=])
case "x$HDF5_LDFLAGS" in
  xno)
    AC_MSG_ERROR("Invalid argument: --without-hdf5-ldflags")
    ;;
  xyes)
    AC_MSG_ERROR("Nonsensical argument:  --with-hdf5-ldflags without any specified flags")
    ;;
esac


  # CLI option for HDF5
AC_MSG_CHECKING([if HDF5 support is enabled])
AC_ARG_WITH(hdf5, 
[AC_HELP_STRING([--with-hdf5=DIR], [Specify HDF5 library to use for native file format])
AC_HELP_STRING([--without-hdf5], [Disable support for native HDF5 file format])],
[HDF5_ARG=$withval], [HDF5_ARG=yes])
if test "xno" = "x$HDF5_ARG"; then
  AC_MSG_RESULT([no])
else
  AC_MSG_RESULT([yes])
fi

 # if HDF5 support is not disabled, check to make sure we have HDF5
if test "xno" != "x$HDF5_ARG"; then
    # Check for IBM parallel IO library
  if test "x$WITH_MPI" != "xno"; then
    AC_CHECK_LIB([gpfs],[gpfs_stat],[LIBS="-lgpfs $LIBS"])
  fi

    # Add flag to defines
  LIBS="$HDF5_LDFLAGS $LIBS"

    # if a path is specified, update LIBS and INCLUDES accordingly
  if test "xyes" != "x$HDF5_ARG"; then
    if test -d "${HDF5_ARG}/lib"; then
      LIBS="$LIBS -L${HDF5_ARG}/lib"
    elif test -d "${HDF5_ARG}"; then
      LIBS="$LIBS -L${HDF5_ARG}"
    else
      AC_MSG_ERROR("$HDF5_ARG is not a directory.")
    fi
    if test -d "${HDF5_ARG}/include"; then
      INCLUDES="$INCLUDES -I${HDF5_ARG}/include"
    else
      INCLUDES="$INCLUDES -I${HDF5_ARG}"
    fi
  fi
  
    # check for libraries and headers
  old_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $INCLUDES"
  AC_CHECK_HEADERS( [hdf5.h], [], [AC_MSG_ERROR("HDF5 header not found")] )
  CPPFLAGS="$old_CPPFLAGSS"
  
  HAVE_LIB_HDF5=no
  AC_CHECK_LIB( [hdf5], [H5Fopen], [HAVE_LIB_HDF5=yes] )
  if test $HAVE_LIB_HDF5 = no; then
    if test $HAVE_ZLIB = yes; then
      unset ac_cv_lib_hdf5_H5Fopen
      AC_CHECK_LIB( [hdf5], [H5Fopen], [HAVE_LIB_HDF5=yes; LIBS="-lz $LIBS"], [], [-lz] )
    fi
  fi
  if test $HAVE_LIB_HDF5 = no; then
    if test $HAVE_SZIP = yes; then
      unset ac_cv_lib_hdf5_H5Fopen
      AC_CHECK_LIB( [hdf5], [H5Fopen], [HAVE_LIB_HDF5=yes; LIBS="-lsz $LIBS"], [], [-lsz] )
    fi
  fi
  if test $HAVE_LIB_HDF5 = no; then
    if test $HAVE_SZIP = yes; then
      if test $HAVE_ZLIB = yes; then
        unset ac_cv_lib_hdf5_H5Fopen
        AC_CHECK_LIB( [hdf5], [H5Fopen], [HAVE_LIB_HDF5=yes; LIBS="-lsz -lz $LIBS"], [], [-lz -lsz] )
      fi
    fi
  fi
  if test HAVE_LIB_HDF5 = no; then
    AC_MSG_ERROR([Could not find HDF5 library (-lhdf5)])
  fi
  LIBS="-lhdf5 $LIBS"
  HAVE_HDF5=yes
else
  HAVE_HDF5=no
fi


]) # SNL_CHECK_HDF5




#######################################################################################
# Check for NetCDF library ((C++)
# Sets HAVE_NETCDF to 'yes' or 'no'
# If HAVE_NETCDF == yes, then updates INCLUDES and LIBS accordingly.
#######################################################################################
AC_DEFUN([SNL_CHECK_NETCDF],[

AC_MSG_CHECKING([if NetCDF support is enabled])
AC_ARG_WITH(netcdf, 
[AC_HELP_STRING([--with-netcdf=DIR], [Specify NetCDF library to use for ExodusII file format])
AC_HELP_STRING([--without-netcdf], [Disable support for ExodusII file format])],
[NETCDF_ARG=$withval], [NETCDF_ARG=yes])
if test "xno" = "x$NETCDF_ARG"; then
  AC_MSG_RESULT([no])
else
  AC_MSG_RESULT([yes])
fi

 # if NetCDF support is not disabled
if test "xno" != "x$NETCDF_ARG"; then
    # Add flag to defines
  DEFINES="$DEFINES -DNETCDF_FILE"

    # Check for stream headers and set STRSTREAM_H_SPEC accordingly
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_CHECK_HEADER( [strstream.h], [NETCDF_DEF="<strstream.h>"], [
    AC_CHECK_HEADER( [sstream.h], [NETCDF_DEF="<sstream.h>"], [
      AC_CHECK_HEADER( [strstream], [NETCDF_DEF="<strstream>"], [
        AC_CHECK_HEADER( [sstream], [NETCDF_DEF="<sstream>"] )
  ] ) ] ) ] )
  AC_LANG_RESTORE

    # if a path is specified, update LIBS and INCLUDES accordingly
  if test "xyes" != "x$NETCDF_ARG"; then
    if test -d "${NETCDF_ARG}/lib"; then
      LIBS="$LIBS -L${NETCDF_ARG}/lib"
    elif test -d "${NETCDF_ARG}"; then
      LIBS="$LIBS -L${NETCDF_ARG}"
    else
      AC_MSG_ERROR("$NETCDF_ARG is not a directory.")
    fi
    if test -d "${NETCDF_ARG}/include"; then
      INCLUDES="$INCLUDES -I${NETCDF_ARG}/include"
    elif test -d "${NETCDF_ARG}/inc"; then
      INCLUDES="$INCLUDES -I${NETCDF_ARG}/inc"
    else
      INCLUDES="$INCLUDES -I${NETCDF_ARG}"
    fi
  fi
  
  old_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CPPFLAGS $INCLUDES"
  old_CXXFLAGS="$CXXFLAGS"
  CXXFLAGS="$CXXFLAGS $INCLUDES"
   # Check for C library
  AC_CHECK_HEADERS( [netcdf.h], [], [AC_MSG_ERROR([[NetCDF header not found.]])] )
  AC_MSG_CHECKING([for netcdf.hh])
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  HAVE_NETCDF_HH=no
  AC_TRY_COMPILE( 
[#include "netcdf.hh"], [], [HAVE_NETCDF_HH=yes; NETCDF_DEF=], [
    AC_TRY_COMPILE( 
[#define STRSTREAM_H_SPEC $NETCDF_DEF
 #include "netcdf.hh"], [], [HAVE_NETCDF_HH=yes], [NAVE_NETCDF_HH=no])])
  AC_MSG_RESULT([$HAVE_NETCDF_HH])
  if test $HAVE_NETCDF_HH != yes; then
    AC_MSG_ERROR([NetCDF C++ header not found])
  fi
  if test "x$NETCDF_DEF" != "x"; then
  CPPFLAGS="$CPPFLAGS -DSTRSTREAM_H_SPEC=$NETCDF_DEF"
  CXXFLAGS="$CXXFLAGS -DSTRSTREAM_H_SPEC=$NETCDF_DEF"
  fi
  AC_MSG_CHECKING([[for netcdf_c++ library]])
  LIBS="$LIBS -lnetcdf_c++ -lnetcdf"
  AC_TRY_LINK(
    [#include <netcdf.hh>], [NcFile ncf("foo",NcFile::ReadOnly);],
    [AC_MSG_RESULT([yes])], [AC_MSG_RESULT([no]); AC_MSG_ERROR([NetCDF C++ API not found])] )
  AC_LANG_RESTORE
  CPPFLAGS="$old_CPPFLAGS"
  CXXFLAGS="$old_CXXFLAGS"
  if test "x$NETCDF_DEF" != "x"; then
    DEFINES="$DEFINES '-DSTRSTREAM_H_SPEC=$NETCDF_DEF'"
  fi
  
  HAVE_NETCDF=yes
else
  HAVE_NETCDF=no
fi

]) # SNL_HAVE_NETCDF




#######################################################################################
# Macro to set the following variables:
# ACIS_VERSION
# ACIS_LIB_DIR
# ACIS_DEBUG_LIB_DIR
# ACIS_PLATFORM
# This macro expects ACIS_DIR to be set.
# If ACIS_SYSTEM is set, ACIS_LIB_DIR will
# be constructed using that value rather than
# via a trial-and-error search.
# If ACIS_VERSION is set, the corresponding test
# will be skipped.
#######################################################################################
AC_DEFUN([SNL_ACIS_ENV], [

AC_CHECK_FILE([$ACIS_DIR/include/acis.hxx], [], [AC_MSG_ERROR("Invalid ACIS path")])
 
if test "x" == "x$ACIS_SYSTEM"; then
  dir_list="$ACIS_DIR/bin/*"
else
  dir_list="$ACIS_DIR/bin/$ACIS_SYSTEM"
fi

for dir in $dir_list; do
    # first try non-debug libraries
  case "$dir" in
    *_debug)
      ;;
    *)
      if ! test "$ACIS_VERSION" -gt "600"; then
        SNL_ACIS_VERSION( [$dir] )
        ACIS_LIB_DIR="$dir"
      fi
      ;;
  esac
    # next try debug libraries
  case "$dir" in
    *_debug)
      if ! test "$ACIS_VERSION" -gt "600"; then
        SNL_ACIS_VERSION( [$dir] )
        ACIS_DEBUG_LIB_DIR="$dir"
      fi
      ;;
  esac
done

if ! test "$ACIS_VERSION" -gt "600"; then
  AC_MSG_ERROR([ACIS configuration failed.  Verify ACIS directory.  Try specifying ACIS system and version])
fi

# If either ACIS_LIB_DIR or ACIS_DEBUG_LIB_DIR is not defined,
# make both the same
if test "x" = "x$ACIS_LIB_DIR"; then
  ACIS_LIB_DIR="$ACIS_DEBUG_LIB_DIR"
elif test "x" = "x$ACIS_DEBUG_LIB_DIR"; then
  ACIS_DEBUG_LIB_DIR="$ACIS_LIB_DIR"
fi

# Determine the ACIS platform name from the lib dir
AC_MSG_CHECKING([ACIS platform name])
case "$ACIS_LIB_DIR" in
  $ACIS_DIR/bin/aix*)  
    ACIS_PLATFORM=aix
    ;;
  $ACIS_DIR/bin/hp700*)
    ACIS_PLATFORM=hp700
    ;;
  $ACIS_DIR/bin/linux*)
    ACIS_PLATFORM=linux
    ;;
  $ACIS_DIR/bin/mac*)
    ACIS_PLATFORM=mac
    ;;
  $ACIS_DIR/bin/osf1*)
    ACIS_PLATFORM=osf1
    ;;
  $ACIS_DIR/bin/sgi*)
    ACIS_PLATFORM=sgi
    ;;
  $ACIS_DIR/bin/solaris*)
    ACIS_PLATFORM=solaris
    ;;
  *)
    AC_MSG_ERROR([Cannot determine ACIS platform name.])
    ;;
esac
AC_MSG_RESULT([$ACIS_PLATFORM])
])



#######################################################################################
# Macro to check for ACIS translators.
#######################################################################################
AC_DEFUN([SNL_ACIS_TRANSLATOR], [
AC_REQUIRE([SNL_ACIS_ENV])
case "$ACIS_VERSION" in
  11??)
    ACIS_XLATE_LIBS='-lxacis2k -lxcore2k -lxutil'
    ;;
  1[23]??)
    ACIS_XLATE_LIBS='-lSPAXAssemblyRep -lSPAXInterop -lSPAXBase -lxacis2k -lxcore2k -lxutil'
    ;;
  14??)
    ACIS_XLATE_LIBS='-lSPAXAssemblyRep -lSPAXInterop -lSPAXAcisBase -lSPAXDefaultGeometryRep -lSPAXGeometryRepresentation -lSPAXPropertiesBRepImporter -lSPAXPropertiesBase -lSPAXBase -lxacis2k -lxcore2k'
    ;;
  *)
    ACIS_XLATE_LIBS='-lSPAXEBOMBase -lSPAXEBOMNewAssemblyExporter -lSPAXEBOMNewAssemblyImporter -lSPAXXMLTk -lpthread -lSPAXPropertiesNewAssemblyImporter -lSPAXAssemblyRep -lSPAXInterop -lSPAXAcisBase -lSPAXDefaultGeometryRep -lSPAXGeometryRepresentation -lSPAXPropertiesBRepImporter -lSPAXPropertiesBase -lSPAXBase -lxacis2k -lxcore2k'
    ;;
esac
old_LIBS="$LIBS"
LIBS="$LIBS -L$ACIS_LIB_DIR $ACIS_XLATE_LIBS $ACIS_BASE_LIBS"
AC_CHECK_LIB([xstep],[main],[ACIS_STEP_TRANSLATOR=-DACIS_STEP_TRANSLATOR])
AC_CHECK_LIB([xiges],[main],[ACIS_IGES_TRANSLATOR=-DACIS_IGES_TRANSLATOR])
LIBS="$old_LIBS"
])


#######################################################################################
# *******************************************************************************
# **************************** INTERNAL STUFF ***********************************
# *******************************************************************************
#######################################################################################


#######################################################################################
# Check for compiler-specific flags.
# Sets the following environmental variables:
#   SNL_CXX_SPECIAL : Any compiler-specific flags which must be specified
#   SNL_CXX_32BIT   : Flag to force compilation of 32-bit code
#   SNL_CXX_64BIT   : Flag to force compilation of 64-bit code
#######################################################################################
AC_DEFUN([SNL_CXX_FLAGS], [
AC_REQUIRE([AC_PROG_CXX])

# Detect compiler 

# First check for mpiCC and extract real compiler
SNL_OLD_CXX="$CXX"
if $CXX -help 2>&1 | grep MPI > /dev/null; then
  CXX=`$CXX -compile-info | cut -d ' ' -f 1`
  AC_CHECK_PROG([CXX],[$CXX],[$CXX],[$SNL_OLD_CXX])
  if $CXX -v 2>&1 | grep gcc > /dev/null; then GXX=yes; fi
elif $CXX 2>&1 | grep xlC > /dev/null; then
  CXX=xlC
  AC_CHECK_PROG([CXX],[xlC],[xlC],[$SNL_OLD_CXX])
  GXX=no
fi

AC_MSG_CHECKING([for known c++ compilers])
# Autoconf does G++ for us
if test x$GXX = xyes; then
  cxx_compiler=GNU
  SNL_CXX_SPECIAL="-Wall -pipe"
# Detect IBM VisualAge compiler
elif $CXX 2>&1 | grep VisualAge > /dev/null; then
  cxx_compiler=VisualAge
# Sun Workshop/Forte
elif $CXX -V 2>&1 | grep Sun > /dev/null; then
  cxx_compiler=Sun
# Intel Compiler
elif $CXX -V 2>&1 | grep Intel > /dev/null; then
  cxx_compiler=Intel
# Compaq compiler
elif $CXX -V 2>&1 | grep Compaq > /dev/null; then
  cxx_compiler=Compaq
# IRIX
elif $CXX -version 2>&1 | grep MIPSpro > /dev/null; then
  cxx_compiler=MIPSpro
else
  # Try deciding based on compiler name
  case "$CXX" in
    xlC*)
      cxx_compiler=VisualAge
      ;;
    aCC)
      cxx_compiler=HP
      ;;
    icc)
      cxx_compiler=Intel
      ;;
    cl|cl.exe)
      cxx_compiler=VisualStudio
      ;;
    CC)
      case "$target_os" in
        solaris*)
          cxx_compiler=Sun
          ;;
        irix*)
          cxx_compiler=MIPSpro
          ;;
      esac
  esac
fi
if test x$cxx_compiler = x; then
  AC_MSG_RESULT([unknown])
  AC_MSG_WARN([Unrecognized c++ compiler: $CXX])
  cxx_compiler=unknown
else
  AC_MSG_RESULT([$cxx_compiler])
fi

# Now decide special compiler flags using compiler/cpu combination
AC_MSG_CHECKING([for known compiler/OS combinations])
case "$cxx_compiler:$host_cpu" in
  GNU:sparc*)
    SNL_CXX_32BIT=-m32
    SNL_CXX_64BIT=-m64
    ;;
  GNU:powerpc*)
    SNL_CXX_32BIT=-m32
    SNL_CXX_64BIT=-m64
    ;;
  GNU:i?86)
    SNL_CXX_32BIT=-m32
    SNL_CXX_64BIT=-m64
    ;;
  GNU:mips*)
    SNL_CXX_32BIT="-mips32 -mabi=32"
    SNL_CXX_64BIT="-mips64 -mabi=64"
    ;;
  VisualAge:*)
    SNL_CXX_32BIT=-q32
    SNL_CXX_64BIT=-q64
    # Do V5.0 namemangling for compatibility with ACIS, and enable RTTI
    SNL_CXX_SPECIAL="-qrtti=all -qnamemangling=v5"
    AR="ar -X 32_64"
    NM="nm -B -X 32_64"
    ;;
  MIPSpro:mips)
    SNL_CXX_32BIT=-n32
    SNL_CXX_64BIT=-64
    SNL_CXX_SPECIAL=-LANG:std
    ;;
  MIPSpro:*)
    SNL_CXX_SPECIAL=-LANG:std
    ;;
  Sun:sparc*)
    SNL_CXX_32BIT=-xarch=generic
    SNL_CXX_64BIT=-xarch=generic64
    ;;
  *)
    ;;
esac
AC_MSG_RESULT([$cxx_compiler:$host_cpu])
CXX="$SNL_OLD_CXX"
]) # end SNL_CXX_FLAGS

#######################################################################################
# Check for compiler-specific flags.
# Sets the following environmental variables:
#   SNL_CC_SPECIAL : Any compiler-specific flags which must be specified
#   SNL_CC_32BIT   : Flag to force compilation of 32-bit code
#   SNL_CC_64BIT   : Flag to force compilation of 64-bit code
#######################################################################################
AC_DEFUN([SNL_CC_FLAGS], [
AC_REQUIRE([AC_PROG_CC])

# Detect compiler 

# First check for mpicc and extract real compiler
SNL_OLD_CC="$CC"
if $CC -help 2>&1 | grep MPI > /dev/null; then
  CC=`$CC -compile-info | cut -d ' ' -f 1`
  AC_CHECK_PROG([CC],[$CC],[$CC],[$SNL_OLD_CC])
  if $CC -v 2>&1 | grep gcc > /dev/null; then GCC=yes; fi
elif $CC 2>&1 | grep xlc > /dev/null; then
  CC=xlc
  AC_CHECK_PROG([CC],[xlc],[xlc],[$SNL_OLD_CC])
  GCC=no
fi

AC_MSG_CHECKING([for known C compilers])
# Autoconf does gcc for us
if test x$GCC = xyes; then
  cc_compiler=GNU
  SNL_CC_SPECIAL="-Wall -pipe"
# Detect IBM VisualAge compiler
elif $CC 2>&1 | grep VisualAge > /dev/null; then
  cc_compiler=VisualAge
# Sun Workshop/Forte
elif $CC -V 2>&1 | grep Sun > /dev/null; then
  cc_compiler=Sun
# Intel Compiler
elif $CC -V 2>&1 | grep Intel > /dev/null; then
  cc_compiler=Intel
# Compaq compiler
elif $CC -V 2>&1 | grep Compaq > /dev/null; then
  cc_compiler=Compaq
# IRIX
elif $CC -version 2>&1 | grep MIPSpro > /dev/null; then
  cc_compiler=MIPSpro
else
  # Try deciding based on compiler name
  case "$CC" in
    xlC*)
      cc_compiler=VisualAge
      ;;
    aCC)
      cc_compiler=HP
      ;;
    icc)
      cc_compiler=Intel
      ;;
    cl|cl.exe)
      cc_compiler=VisualStudio
      ;;
    CC)
      case "$target_os" in
        solaris*)
          cc_compiler=Sun
          ;;
        irix*)
          cc_compiler=MIPSpro
          ;;
      esac
  esac
fi
if test x$cc_compiler = x; then
  AC_MSG_RESULT([unknown])
  AC_MSG_WARN([Unrecognized C compiler: $CC])
  cc_compiler=unknown
else
  AC_MSG_RESULT([$cc_compiler])
fi

# Now decide special compiler flags using compiler/cpu combination
AC_MSG_CHECKING([for known compiler/OS combinations])
case "$cc_compiler:$host_cpu" in
  GNU:sparc*)
    SNL_CC_32BIT=-m32
    SNL_CC_64BIT=-m64
    ;;
  GNU:powerpc*)
    SNL_CC_32BIT=-m32
    SNL_CC_64BIT=-m64
    ;;
  GNU:i?86)
    SNL_CC_32BIT=-m32
    SNL_CC_64BIT=-m64
    ;;
  GNU:mips*)
    SNL_CC_32BIT="-mips32 -mabi=32"
    SNL_CC_64BIT="-mips64 -mabi=64"
    ;;
  VisualAge:*)
    SNL_CC_32BIT=-q32
    SNL_CC_64BIT=-q64
    AR="ar -X 32_64"
    NM="nm -B -X 32_64"
    ;;
  MIPSpro:mips)
    SNL_CC_32BIT=-n32
    SNL_CC_64BIT=-64
    SNL_CC_SPECIAL=-LANG:std
    ;;
  MIPSpro:*)
    SNL_CC_SPECIAL=-LANG:std
    ;;
  Sun:sparc*)
    SNL_CC_32BIT=-xarch=generic
    SNL_CC_64BIT=-xarch=generic64
    ;;
  *)
    ;;
esac
AC_MSG_RESULT([$cc_compiler:$host_cpu])
CC="$SNL_OLD_CC"
]) # end SNL_CC_FLAGS


#######################################################################################
# Macro to get ACIS_VERSION
# Expected arguments: ACIS library path
#######################################################################################
AC_DEFUN([SNL_ACIS_VERSION], [
AC_REQUIRE([AC_PROG_LIBTOOL])
AC_LANG_SAVE
AC_LANG_CPLUSPLUS
AC_MSG_CHECKING([ACIS version of $1])
src=conftest.cc
exe=conftest

cat <<END_SNL_ACIS_VERSION_SRC  >$src
#include <stdio.h>
int get_major_version();
int get_minor_version();
int get_point_version();
int main(int,char**) { printf("%d\n", 
100*get_major_version() +
 10*get_minor_version() +
    get_point_version()); 
return 0;}

END_SNL_ACIS_VERSION_SRC

FLAGS="$CXXFLAGS $LDFLAGS -L$1 -R$1"
if ./libtool --mode=link $CXX $FLAGS $src -lSpaBase -o $exe >/dev/null 2>/dev/null; then
  ACIS_VERSION=`./$exe || echo 0`
  AC_MSG_RESULT($ACIS_VERSION)
else
  ACIS_VERSION=0
  AC_MSG_RESULT(failed)
fi

#rm -f $src $exe
AC_LANG_RESTORE
]) # SNL_ACIS_VERSION
