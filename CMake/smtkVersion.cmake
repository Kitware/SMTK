# We require smtk's version information to be included the CMake Project
# declaration in order to properly include smtk versions when using smtk as an
# SDK. We therefore do not have access to the variable ${PROJECT_SOURCE_DIR}
# at the time smtkVersion.cmake is read. We therefore use
# ${CMAKE_CURRENT_SOURCE_DIR}, since smtkVersion.cmake is called at the top
# level of smtk's build. We also guard against subsequent calls to
# smtkVersion.cmake elsewhere in the build setup where
# ${CMAKE_CURRENT_SOURCE_DIR} may no longer be set to the top level directory.

if (NOT DEFINED SMTK_VERISON)

  file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/version.txt version_string )

  string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)[-]*(.*)"
    version_matches "${version_string}")

  set(SMTK_VERSION_MAJOR ${CMAKE_MATCH_1})
  set(SMTK_VERSION_MINOR ${CMAKE_MATCH_2})
  set(SMTK_VERSION "${SMTK_VERSION_MAJOR}.${SMTK_VERSION_MINOR}")
  if (DEFINED CMAKE_MATCH_3)
    set(SMTK_VERSION_PATCH ${CMAKE_MATCH_3})
    set(SMTK_VERSION "${SMTK_VERSION}.${SMTK_VERSION_PATCH}")
  else()
    set(SMTK_VERSION_PATCH 0)
  endif()
  # To be thorough, we should split the label into "-prerelease+metadata"
  # and, if prerelease is specified, use it in determining precedence
  # according to semantic versioning rules at http://semver.org/ .
  # For now, just make the information available as a label:
  if (DEFINED CMAKE_MATCH_4)
    set(SMTK_VERSION_LABEL "${CMAKE_MATCH_4}")
  endif()

  set(SMTK_VERSION_MINOR_STRING ${SMTK_VERSION_MINOR})
  string(REGEX REPLACE "^0" "" SMTK_VERSION_MINOR_INT ${SMTK_VERSION_MINOR})
endif()
