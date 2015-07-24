# Inspired by CMake Distcheck for LAAS-CNRS
#
# DEFINE_DISTCHECK
# ---------------
#
# Add a distcheck target to check the generated tarball.
#
# This step calls `make dist' to generate a copy of the MOAB sources as it 
# stands in the current git HEAD i.e., unversioned files are skipped.
#
# Then:
# - create _build and _inst to respectively create a build and an installation
#   directory.
# - copy the CMakeCache.txt file and apply several transformations.
# - run cmake with _inst as the installation prefix
# - run make, make check, make install and make uninstall
# - remove _build and _inst.
# - remove dist directory and confirm success.
#
# During the compilation phase, all files in the source tree are modified
# to *not* be writeable to detect bad compilation steps which tries to modify
# the source tree. Permissions are reverted at the end of the check.
#
MACRO(DEFINE_DISTCHECK)
  FIND_PROGRAM(SED sed)
  FIND_PROGRAM(TAR tar)
  FIND_PROGRAM(GZIP gzip)
  STRING(TOLOWER "${PACKAGE_NAME}-${PACKAGE_VERSION}" DISTBASENAME)
  SET(INSTDIR ${CMAKE_BINARY_DIR}/${DISTBASENAME}/_inst)

  ADD_CUSTOM_TARGET(dist 
    COMMAND
    cd ${CMAKE_SOURCE_DIR}
    && git archive --format=tar --prefix=${DISTBASENAME}/ HEAD | 
    ${GZIP} > ${CMAKE_BINARY_DIR}/${DISTBASENAME}.tar.gz
  )

  ADD_CUSTOM_TARGET(distcheck
    COMMAND
    rm -rf ${DISTBASENAME}
    && ${GZIP} -d ${DISTBASENAME}.tar.gz
    && ${TAR} -xf ${DISTBASENAME}.tar
    && cd ${DISTBASENAME}/
    && chmod u+w . && mkdir -p _build && mkdir -p _inst
    && ${CMAKE_SOURCE_DIR}/config/CMakeReplicateConfig.sh "${CMAKE_BINARY_DIR}/CMakeCache.txt" _build/CMakeCache.txt
    && chmod u+rwx _build _inst && chmod a-w .
    && cd _build
    && cmake -DCMAKE_INSTALL_PREFIX=${INSTDIR} ..
        || (echo "ERROR: the cmake configuration failed." && false)
    && make -j4
        || (echo "ERROR: the compilation failed." && false)
    && make test
        || (echo "ERROR: the test suite failed." && false)
    && make install
        || (echo "ERROR: the install target failed." && false)
    && make uninstall
        || (echo "ERROR: the uninstall target failed." && false)
    && test x`find ${INSTDIR} -type f | wc -l` = x0
        || (echo "ERROR: the uninstall target does not work." && false)
    && make clean
        || (echo "ERROR: the clean target failed." && false)
    && cd ${CMAKE_BINARY_DIR}/${DISTBASENAME}
    && chmod u+w . _build _inst && rm -rf _build _inst
    && find . -type d -print0 | xargs -0 chmod u+w
    && cd ${CMAKE_BINARY_DIR}
    && rm -rf ${CMAKE_BINARY_DIR}/${DISTBASENAME}
    && ${GZIP} ${DISTBASENAME}.tar
    && echo "=============================================================="
    && echo "${DISTBASENAME}"
            "is ready for distribution."
    && echo "=============================================================="
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Checking generated tarball..."
    )
  ADD_DEPENDENCIES(distcheck dist)
ENDMACRO(DEFINE_DISTCHECK)

