#######################################################################################
# Check for Damsel library ((C++)
# Sets HAVE_DAMSEL to 'yes' or 'no'
# If HAVE_DAMSEL == yes, then exports:
#   DAMSEL_CPPFLAGS
#   DAMSEL_LDFLAGS
#   DAMSEL_LIBS
#######################################################################################
AC_DEFUN([FATHOM_CHECK_DAMSEL],[

AC_MSG_CHECKING([if DAMSEL support is enabled])
AC_ARG_WITH(damsel, 
[AS_HELP_STRING([--with-damsel=DIR], [Specify DAMSEL location])
AS_HELP_STRING([--without-damsel], [Disable support for DAMSEL file format])],
[DAMSEL_ARG=$withval
DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-damsel=\"${withval}\""
]
, [DAMSEL_ARG=])
if test "x" != "x$DAMSEL_ARG" && test "xno" != "x$DAMSEL_ARG"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

 # if DAMSEL support is enabled
AC_MSG_CHECKING([if DAMSEL support available])
HAVE_DAMSEL=no
if (test "x" != "x$DAMSEL_ARG" && test "xno" != "x$DAMSEL_ARG"); then
  HAVE_DAMSEL=yes
  AC_MSG_RESULT([yes])

    # if a path is specified, update LIBS and INCLUDES accordingly
  if test "xyes" != "x$DAMSEL_ARG" && test "x" != "x$DAMSEL_ARG"; then
    if test -d "${DAMSEL_ARG}/lib"; then
      DAMSEL_LDFLAGS="-L${DAMSEL_ARG}/lib"
    elif test -d "${DAMSEL_ARG}"; then
      DAMSEL_LDFLAGS="-L${DAMSEL_ARG}"
    else
      AC_MSG_ERROR("$DAMSEL_ARG is not a directory.")
    fi
    if test -d "${DAMSEL_ARG}/include"; then
      DAMSEL_CPPFLAGS="-I${DAMSEL_ARG}/include"
    else
      DAMSEL_CPPFLAGS="-I${DAMSEL_ARG}"
    fi
  fi
  
  old_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$DAMSEL_CPPFLAGS $CPPFLAGS"
  old_LDFLAGS="$LDFLAGS"
  LDFLAGS="$DAMSEL_LDFLAGS $LDFLAGS"
  
   # Check for C library
  AC_CHECK_HEADERS( [damsel.h], [], [HAVE_DAMSEL=no] )
  CPPFLAGS="$old_CPPFLAGS"
  LDFLAGS="$old_LDFLAGS"

  if test "x$HAVE_DAMSEL" = "xno"; then
    if test "x$DAMSEL_ARG" != "x"; then 
      AC_MSG_ERROR("Damsel not found or not working")
    else
      AC_MSG_CHECKING([unsuccessful, Damsel support disabled])
      AC_MSG_RESULT([])
    fi
    DAMSEL_CPPFLAGS=
    DAMSEL_LDFLAGS=
  else
    DAMSEL_LIBS="-ldamsel"
  fi

    # DAMSEL requires HDF5 too
  if test "xno" == "x$enablehdf5"; then
    if test "x" == "x$DAMSEL_ARG"; then
      DAMSEL_ARG=no
    elif test "xno" != "xDAMSEL_ARG"; then
      AC_MSG_ERROR([Damsel requires --with-hdf5])
    fi
    HAVE_DAMSEL=no
  fi

else
  AC_MSG_RESULT([no])
fi

]) # FATHOM_HAVE_DAMSEL
