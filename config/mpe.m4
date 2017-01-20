#######################################################################################
# Configure support for MPE and LMPE libs for profiling/logging MPI calls
# Arguments: None
# Output:  Sets WITH_MPE to yes or no
#          Sets MPE_LIBS to "-llmpe -lmpe" iff WITH_MPE == yes
#######################################################################################
AC_DEFUN([FATHOM_CHECK_MPE], [
  AC_ARG_ENABLE([mpe],
                [AS_HELP_STRING([--enable-mpe],
                                [Enable use of MPE library for tracing/logging MPI communication])],
                [WITH_MPE=$enableval],[WITH_MPE=no])
  AC_LANG_PUSH([C])
  if test "x$WITH_MPE" != "xno"; then
    if test "x$WITH_MPE" != "xyes"; then
      AC_MSG_ERROR(["Unexpected value for --enable-mpe option: $WITH_MPE])
    fi
    old_LIBS="$LIBS"
    AC_CHECK_HEADER([mpe.h], [], [AC_MSG_ERROR([--enable-mpe failed because mpe.h was not found])])
    AC_CHECK_LIB([mpe], [MPE_Init_log], [], [AC_MSG_ERROR([--enable-mpe failed because MPE library was not found])])
    AC_CHECK_LIB([lmpe], [MPI_Init], [], [AC_MSG_ERROR([--enable-mpe failed because LMPE library was not found])], [-lmpe])
    LIBS="$old_LIBS"
    MPE_LIBS="-llmpe -lmpe"
  fi
])
