#######################################################################################
# Check for CCMIO library ((C++)
# Sets HAVE_CCMIO to 'yes' or 'no'
# If HAVE_CCMIO == yes, then exports:
#   CCMIO_CPPFLAGS
#   CCMIO_LDFLAGS
#   CCMIO_LIBS
#######################################################################################
AC_DEFUN([FATHOM_CHECK_CCMIO],[

AC_MSG_CHECKING([if CCMIO support is enabled])
AC_ARG_WITH(ccmio, 
[AS_HELP_STRING([--with-ccmio=DIR], [Specify CCMIO location])
AS_HELP_STRING([--without-ccmio], [Disable support for CCMIO file format])],
[CCMIO_ARG=$withval
DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-ccmio=\"${withval}\""
]
, [CCMIO_ARG=])
if test "xno" != "x$CCMIO_ARG"; then
  AC_MSG_RESULT([yes])
else
  AC_MSG_RESULT([no])
fi

 # if CCMIO support is not disabled
AC_MSG_CHECKING([if CCMIO support available])
AC_MSG_RESULT([])
HAVE_CCMIO=no
if test "xno" != "x$CCMIO_ARG"; then
  HAVE_CCMIO=yes

    # if a path is specified, update LIBS and INCLUDES accordingly
  if test "xyes" != "x$CCMIO_ARG" && test "x" != "x$CCMIO_ARG"; then
    if test -d "${CCMIO_ARG}/linux/64/lib"; then
      CCMIO_LDFLAGS="-L${CCMIO_ARG}/linux/64/lib"
    elif test -d "${CCMIO_ARG}"; then
      CCMIO_LDFLAGS="-L${CCMIO_ARG}/lib"
    elif test -d "${CCMIO_ARG}/lib"; then
      CCMIO_LDFLAGS="-L${CCMIO_ARG}"
    else
      AC_MSG_ERROR("$CCMIO_ARG is not a directory.")
    fi
    if test -d "${CCMIO_ARG}/include"; then
      CCMIO_CPPFLAGS="-I${CCMIO_ARG}/include"
    else
      CCMIO_CPPFLAGS="-I${CCMIO_ARG}"
    fi
  fi
  
  old_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$CCMIO_CPPFLAGS $CPPFLAGS"
  old_LDFLAGS="$LDFLAGS"
  LDFLAGS="$CCMIO_LDFLAGS $LDFLAGS"
  
   # Check for C library
  AC_CHECK_HEADERS( [ccmio.h], [], [HAVE_CCMIO=no] )
  AC_CHECK_HEADERS( [ccmioutility.h], [], [HAVE_CCMIO=no] )
  AC_CHECK_HEADERS( [ccmiocore.h], [], [HAVE_CCMIO=no] )
  CPPFLAGS="$old_CPPFLAGS"
  LDFLAGS="$old_LDFLAGS"

  if test "x$HAVE_CCMIO" = "xno"; then
    if test "x$CCMIO_ARG" != "x"; then 
      AC_MSG_ERROR("Ccmio not found or not working")
    else
      AC_MSG_CHECKING([unsuccessful, Ccmio support disabled])
      AC_MSG_RESULT([])
    fi
    CCMIO_CPPFLAGS=
    CCMIO_LDFLAGS=
  else
    CCMIO_LIBS="-lccmio -ladf"
  fi
fi

]) # FATHOM_HAVE_CCMIO
