#######################################################################################
# Check for CGNS library
# Sets HAVE_CGNS to 'yes' or 'no'
# If HAVE_CGNS == yes, then exports:
#   CGNS_CPPFLAGS
#   CGNS_LDFLAGS
#   CGNS_LIBS
#######################################################################################
AC_DEFUN([FATHOM_CHECK_CGNS],[

AC_MSG_CHECKING([if CGNS support is enabled])
AC_ARG_WITH(cgns, 
[AS_HELP_STRING([--with-cgns<:@=DIR@:>@], [Specify CGNS library to use for CGNS file format])
AS_HELP_STRING([--without-cgns], [Disable support for CGNS file format])],
[CGNS_ARG=$withval
DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-cgns=\"${withval}\""
]
, [CGNS_ARG=])
if test "xno" != "x$CGNS_ARG"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

 # if CGNS support is not disabled
HAVE_CGNS=no
if test "xno" != "x$CGNS_ARG"; then
  HAVE_CGNS=yes
  
    # if a path is specified, update LIBS and INCLUDES accordingly
  if test "xyes" != "x$CGNS_ARG" && test "x" != "x$CGNS_ARG"; then
    if test -d "${CGNS_ARG}/lib"; then
      CGNS_LDFLAGS="-L${CGNS_ARG}/lib"
    elif test -d "${CGNS_ARG}"; then
      CGNS_LDFLAGS="-L${CGNS_ARG}"
    else
      AC_MSG_ERROR("$CGNS_ARG is not a directory.")
    fi
    if test -d "${CGNS_ARG}/include"; then
      CGNS_CPPFLAGS="-I${CGNS_ARG}/include"
    elif test -d "${CGNS_ARG}/inc"; then
      CGNS_CPPFLAGS="-I${CGNS_ARG}/inc"
    else
      CGNS_CPPFLAGS="-I${CGNS_ARG}"
    fi
  fi
  
  old_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CGNS_CPPFLAGS $CPPFLAGS"
  old_LDFLAGS="$LDFLAGS"
  LDFLAGS="$CGNS_LDFLAGS $HDF5_LDFLAGS $LDFLAGS"
  
   # Check for C library
  AC_LANG_PUSH([C])

  AC_CHECK_HEADER([cgnslib.h], [],
                  [AC_MSG_WARN([[CGNS header not found.]]); HAVE_CGNS=no] )

      # Check if cgns is usable by itself
  AC_CHECK_LIB( [cgns], [cg_open], [CGNS_LIBS="-lcgns"], [
      # Check if cgns is usable with HDF5
    unset ac_cv_lib_cgns
    unset ac_cv_lib_cgns_cg_open
      # If we haven't already looked for HDF5 libraries, again now incase
      # they're in the CGNS lib directory.
    FATHOM_DETECT_HDF5_LIBS
    LDFLAGS="$LDFLAGS $HDF5_LDFLAGS"
    AC_CHECK_LIB( [cgns], [cg_open], [CGNS_LIBS="-lcgns -lhdf5_hl"], [
      # Try one more time with HDF5 and libcurl
      unset ac_cv_lib_cgns
      unset ac_cv_lib_cgns_cg_open
      AC_CHECK_LIB( [cgns], [cg_open], [CGNS_LIBS="-lcgns -lhdf5_hl -lcurl"], 
        [HAVE_CGNS=no], [-lhdf5_hl $HDF5_LIBS -lcurl] )],
      [-lhdf5_hl $HDF5_LIBS] )],
    )
  
  CPPFLAGS="$old_CPPFLAGS"
  LDFLAGS="$old_LDFLAGS"
  AC_LANG_POP([C])

  if test "x$HAVE_CGNS" = "xno"; then
    if test "x$CGNS_ARG" != "x"; then 
      AC_MSG_ERROR("CGNS not found or not working")
    else
      AC_MSG_WARN("CGNS support disabled")
    fi
    CGNS_CPPFLAGS=
    CGNS_LDFLAGS=
  fi
fi

]) # FATHOM_HAVE_CGNS
