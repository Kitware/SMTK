#
# Find Verdict include directories and libraries
#
# verdict_INCLUDE_DIRECTORIES - where to find verdict.h
# verdict_LIBRARIES           - list of libraries to link against when using verdict
# verdict_FOUND               - Do not attempt to use verdict if "no", "0", or undefined.

set( verdict_PREFIX "" CACHE PATH "Path to search for Verdict header and library files" )

find_path( verdict_INCLUDE_DIRECTORIES verdict.h
  ${verdict_PREFIX}
  ${verdict_PREFIX}/include
  /usr/local/include
  /usr/include
)

find_library( verdict_LIBRARY
  NAMES verdict
  ${verdict_PREFIX}
  ${verdict_PREFIX}/lib64
  ${verdict_PREFIX}/lib
  /usr/local/lib64
  /usr/lib64
  /usr/local/lib
  /usr/lib
)

set( verdict_LIBRARIES
  ${verdict_LIBRARY}
)

if ( verdict_INCLUDE_DIRECTORIES AND verdict_LIBRARIES )
  set( verdict_FOUND 1 )
else ( verdict_INCLUDE_DIRECTORIES AND verdict_LIBRARIES )
  set( verdict_FOUND 0 )
endif ( verdict_INCLUDE_DIRECTORIES AND verdict_LIBRARIES )

mark_as_advanced(
  verdict_PREFIX
  verdict_INCLUDE_DIRECTORIES
  verdict_LIBRARY
)
