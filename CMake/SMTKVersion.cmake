file(STRINGS version.txt version_string )

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
