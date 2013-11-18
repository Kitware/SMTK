# A macro to turn a string specifying a namespace
# into strings that open and close the namespace.
# Example:
#   begin_end_namespace("std" BEGIN_NS END_NS)
# will set BEGIN_NS to "namespace std {"
# and END_NS to "} /* namespace std */".
#
# This macro handles the case of nested namespaces
# such as std::tr1.
#
# Leading and trailing double-colons are not allowed.

macro(begin_end_namespace NAMESPACE BEGIN_NS END_NS)

string(REPLACE "::" ";" _NS_LIST "${NAMESPACE}")
foreach(_NS ${_NS_LIST})
  set(${BEGIN_NS} "${${BEGIN_NS}}namespace ${_NS} {")
  set(${END_NS} "} /* namespace ${_NS} */${${END_NS}}")
endforeach()

endmacro()
