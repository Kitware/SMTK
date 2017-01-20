# Read the package version number specified as the second argument
# to the AC_INIT macro in a GNU Autoconf configure.in file.
#
# Define the following variables:
# VERSION_STRING:  The second argument to AC_INIT
# MAJOR_VERSION:   For a version string of the form m.n.p..., m
# MINOR_VERSION:   For a version string of the form m.n.p..., n
# PATCH_VERSION:   For a version string of the form m.n.p..., p

macro ( get_ac_init_version )

#file( READ "configure.ac" configure_IN )
#grep AC_INIT configure.ac | sed "s/\(AC_INIT(MOAB,\|)\)//g" | tr ' ' '\0'
set(configure_IN "${PACKAGE_VERSION}")
string( REGEX REPLACE "^.*AC_INIT *\\([^,]+, *([^, )]+).*$" "\\1" VERSION_STRING "${configure_IN}" )
string( REGEX REPLACE "^([0-9]+)(\\..*)?$" "\\1" MAJOR_VERSION "${VERSION_STRING}" )
string( REGEX REPLACE "^[0-9]+\\.([0-9]+)(\\..*)?$" "\\1" MINOR_VERSION "${VERSION_STRING}" )
if ( VERSION_STRING MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+" )
  string( REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*$" "\\1" PATCH_VERSION "${VERSION_STRING}" )
endif ( VERSION_STRING MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+" )

endmacro( get_ac_init_version )
