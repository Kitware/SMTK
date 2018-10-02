# smtk uses the version number determined from this logic to set the project
# command, so we cannot use ${PROJECT_SOURCE_DIR} to find version.txt. This file
# is directly included by the top-level CMakeLists.txt, so
# ${CMAKE_CURRENT_SOURCE_DIR} can be used in its place.
file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/version.txt version_string )

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)[-]*(.*)"
      version_matches "${version_string}")

set(SMTK_VERSION_MAJOR ${CMAKE_MATCH_1})
set(SMTK_VERSION_MINOR ${CMAKE_MATCH_2})
set(SMTK_VERSION "${SMTK_VERSION_MAJOR}.${SMTK_VERSION_MINOR}")
if (CMAKE_MATCH_3)
  set(SMTK_VERSION_PATCH ${CMAKE_MATCH_3})
  set(SMTK_VERSION "${SMTK_VERSION}.${SMTK_VERSION_PATCH}")
else()
  set(SMTK_VERSION_PATCH 0)
endif()
# To be thorough, we should split the label into "-prerelease+metadata"
# and, if prerelease is specified, use it in determining precedence
# according to semantic versioning rules at http://semver.org/ .
# For now, just make the information available as a label:
if (CMAKE_MATCH_4)
  set(SMTK_VERSION_LABEL "${CMAKE_MATCH_4}")
endif()
