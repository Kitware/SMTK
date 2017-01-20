#######################################################################################
# Extract the value of a variable from a makefile fragment.
# Arguments:
#   - The path of the makefile fragment
#   - The name of the makefile variable
#   - Action on success (the shell variable "make_val" will contain the value
#         of the variable)
#   - Action on failure
#######################################################################################
AC_DEFUN([FATHOM_MAKE_INC_VAR], [
make_val=
snl_makefile="snl_check.mak"
rm -f $snl_makefile

if test ! -f $1 ; then
  AC_MSG_WARN([File not found: $1])
  $4
else
cat >$snl_makefile <<FATHOM_END_OF_MAKEFILE
default:
	@echo "\$($2)"

include $1
FATHOM_END_OF_MAKEFILE
if make -f $snl_makefile > /dev/null 2>&1; then
  make_val=`make -s -f $snl_makefile`
  rm -f $snl_makefile
  $3
else
  rm -f $snl_makefile
  $4
fi
fi
])
