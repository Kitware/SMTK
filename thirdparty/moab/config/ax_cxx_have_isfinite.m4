dnl test from https://svn.code.sf.net/p/cppunit/code/trunk/cppunit/config/ax_cxx_have_isfinite.m4 
dnl with <math.h> changed to <cmath> (no std:: added)
dnl ---
dnl @synopsis AX_CXX_HAVE_ISFINITE
dnl
dnl If isfinite() is available to the C++ compiler:
dnl   define HAVE_ISFINITE
dnl   add "-lm" to LIBS
dnl
AC_DEFUN([AX_CXX_HAVE_ISFINITE],
  [ax_cxx_have_isfinite_save_LIBS=$LIBS
   LIBS="$LIBS -lm"

   AC_CACHE_CHECK(for isfinite, ax_cv_cxx_have_isfinite,
    [AC_LANG_SAVE
     AC_LANG_CPLUSPLUS
     AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
         [[#include <cmath>]],
         [[int f = isfinite( 3 );]])],
       [ax_cv_cxx_have_isfinite=yes],
       [ax_cv_cxx_have_isfinite=no])
     AC_LANG_RESTORE])

   if test "$ax_cv_cxx_have_isfinite" = yes; then
     AC_DEFINE([HAVE_ISFINITE],1,[define if compiler has isfinite])
   else
     LIBS=$ax_cxx_have_isfinite_save_LIBS
   fi
])

dnl ---
dnl @synopsis AX_CXX_HAVE_STDISFINITE
dnl
dnl If std::isfinite() is available to the C++ compiler:
dnl   define HAVE_STDISFINITE
dnl   add "-lm" to LIBS
dnl
AC_DEFUN([AX_CXX_HAVE_STDISFINITE],
  [ax_cxx_have_stdisfinite_save_LIBS=$LIBS
   LIBS="$LIBS -lm"

   AC_CACHE_CHECK(for std::isfinite, ax_cv_cxx_have_stdisfinite,
    [AC_LANG_SAVE
     AC_LANG_CPLUSPLUS
     AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
         [[#include <cmath>]],
         [[bool f = std::isfinite( 3 );]])],
       [ax_cv_cxx_have_stdisfinite=yes],
       [ax_cv_cxx_have_stdisfinite=no])
     AC_LANG_RESTORE])

   if test "$ax_cv_cxx_have_stdisfinite" = yes; then
     AC_DEFINE([HAVE_STDISFINITE],1,[define if compiler has std::isfinite])
   else
     LIBS=$ax_cxx_have_stdisfinite_save_LIBS
   fi
])


dnl ---
dnl @synopsis AX_CXX_HAVE_FINITE
dnl
dnl If finite() is available to the C++ compiler:
dnl   define HAVE_FINITE
dnl   add "-lm" to LIBS
dnl
AC_DEFUN([AX_CXX_HAVE_FINITE],
  [ax_cxx_have_finite_save_LIBS=$LIBS
   LIBS="$LIBS -lm"

   AC_CACHE_CHECK(for finite, ax_cv_cxx_have_finite,
    [AC_LANG_SAVE
     AC_LANG_CPLUSPLUS
     AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
         [[#include <cmath>]],
         [[bool f = finite( 3 );]])],
       [ax_cv_cxx_have_finite=yes],
       [ax_cv_cxx_have_finite=no])
     AC_LANG_RESTORE])

   if test "$ax_cv_cxx_have_finite" = yes; then
     AC_DEFINE([HAVE_FINITE],1,[define if compiler has finite])
   else
     LIBS=$ax_cxx_have_finite_save_LIBS
   fi
])

