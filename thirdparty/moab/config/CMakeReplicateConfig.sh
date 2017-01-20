#! /bin/bash
#
# Given a source CMake configuration, replicate variables to be used in a 
# destination whilst eliminating path specific key variables during transformation.
# Usage: CMakeReplicateConfig.sh SOURCE_CMAKE_CACHE_FILE DEST_CMAKE_CACHE_FILE
#
echo "CMakeReplicateConfig.sh: options: $#; input: $1; output: $2"
if [ "$#" -ne 2 ] || ! [ -f "$1" ] ; then
  echo "Please specify source and destination CMakeCache.txt filepaths correctly."
  echo "Usage: CMakeReplicateConfig.sh SOURCE_CMAKE_CACHE_FILE DEST_CMAKE_CACHE_FILE"
  exit 1
fi

cat $1 | sed -n '/CMAKE_CACHEFILE_DIR:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_HOME_DIRECTORY:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_COMPILER:FILEPATH./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS_DEBUG:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS_MINSIZEREL:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS_RELEASE:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS_RELWITHDEBINFO:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_COMPILER-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_COMPILER_WORKS:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS_DEBUG-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS_MINSIZEREL-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS_RELEASE-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_CXX_FLAGS_RELWITHDEBINFO-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_DETERMINE_CXX_ABI_COMPILED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_COMPILER:FILEPATH./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS_DEBUG:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS_MINSIZEREL:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS_RELEASE:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS_RELWITHDEBINFO:STRING./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_COMPILER-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS_DEBUG-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS_MINSIZEREL-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS_RELEASE-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_C_FLAGS_RELWITHDEBINFO-ADVANCED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/CMAKE_DETERMINE_C_ABI_COMPILED:INTERNAL./{n;x;d;};x;1d;$G;p' \
       | sed -n '/EXECUTABLE_OUTPUT_PATH:PATH./{n;x;d;};x;1d;$G;p' \
       | sed -n '/LIBRARY_OUTPUT_PATH:PATH./{n;x;d;};x;1d;$G;p' \
       | sed -n '/produce slightly less optimized./{n;x;d;};x;1d;$G;p' \
       | cat -s > $2

