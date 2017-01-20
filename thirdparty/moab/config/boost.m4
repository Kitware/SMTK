#######################################################################################
# Check for Boost libraryies
# Sets HAVE_BOOST to 'yes' or 'no' and adds any user-specified path to INCLUDES
# Unless user specified --without-boost, will check for any passed headers.
# For any found headers, -DHAVE_[header] will be added to DEFINES
#######################################################################################
AC_DEFUN([FATHOM_CHECK_BOOST],[
AC_MSG_CHECKING([if boost library is enabled])
FATHOM_BOOST_OPT_HEADER_LIST="$1"
AC_ARG_WITH(boost, 
[AS_HELP_STRING([--with-boost=DIR], [Specify directory where boost is installed])
AS_HELP_STRING([--without-boost], [Disable support for boost libraries])],
[BOOST_ARG=$withval
DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS --with-boost=\"${withval}\""
]
, [BOOST_ARG=yes])
if test "xno" = "x$BOOST_ARG"; then
  AC_MSG_RESULT([no])
else
  AC_MSG_RESULT([yes])
  # If user-specified directory, add to includes
  if test "xyes" != "x$BOOST_ARG"; then
    INCLUDES="$INCLUDES -I$BOOST_ARG"
  fi
  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS
  AC_CHECK_HEADERS( [$FATHOM_BOOST_OPT_HEADER_LIST],[def=`echo "$ac_header" | $as_tr_cpp`; DEFINES="$DEFINES -DHAVE_$def"] )
  AC_LANG_RESTORE
fi
]) # FATHOM_CHECK_BOOST
