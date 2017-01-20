# Macro to determine the unsigned integer value of a 
# preprocessor macro.  Arguments are:
#   header, macro, variable, upper bound
# where 'header' is the header in which the macro value
# is defined, 'macro' is the name of the preprocessor macro
# for which the value is to be determined, 'variable' is the
# name of the shell variable in which to store the value,
# and 'upper bound' is the largest value up to which to search
AC_DEFUN([FATHOM_MACRO_VALUE],[
  header="$1"
  macro="$2"
  upper="$4"
  lower="0"
  while test "$lower" -lt "$upper"; do
    curr=$(( ($lower + $upper)/2 ))
    AC_PREPROC_IFELSE(
      [AC_LANG_PROGRAM([#include <${header}>
                        #if $macro >= $curr
                        # error choke me
                        #endif],[])],
      [upper=$curr],
      [AC_PREPROC_IFELSE(
        [AC_LANG_PROGRAM([#include <${header}>
                          #if $macro < $curr
                          # error choke me
                          #endif],[])],
        [break],
        [lower=$curr])])
  done
  AC_PREPROC_IFELSE(
    [AC_LANG_PROGRAM([#include <${header}>
                      #if $curr != $macro
                      # error choke me
                      #endif],[])],
    [$3=$curr],
    [AC_MSG_ERROR([Unable to determine value of $macro defined in $header])])
])

