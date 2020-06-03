# Don't perform clang-tidy on third party code.
set(CMAKE_C_CLANG_TIDY "")
set(CMAKE_CXX_CLANG_TIDY "")

set(CMAKE_INSTALL_CMAKEDIR
  "${SMTK_INSTALL_CONFIG_DIR}/Delaunay")
set(CMAKE_INSTALL_INCLUDEDIR
  "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_VERSION}")
add_subdirectory(delaunay)
unset(CMAKE_INSTALL_CMAKEDIR)

# disable warning flags for delaunay.
option(SMTK_IGNORE_DELAUNAY_WARNINGS "disable warning flags for delaunay" ON)
mark_as_advanced(SMTK_IGNORE_DELAUNAY_WARNINGS)

if (SMTK_IGNORE_DELAUNAY_WARNINGS)
  set(ignore_rules_for_delaunay "")

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # disables: warning C4251: dll-interface warnings involving stl
    set(ignore_rules_for_delaunay "${ignore_rules_for_delaunay} /wd4251")
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # disables warnings for comparing integers of different types
    set(ignore_rules_for_delaunay "${ignore_rules_for_delaunay} -Wno-sign-compare")
  endif()

  foreach(delaunay_target DelaunayDiscretization DelaunayMesh DelaunayMisc
      DelaunayShape DelaunayValidation DelaunayVisualization)
    set_property(TARGET ${delaunay_target} APPEND_STRING
      PROPERTY COMPILE_FLAGS ${ignore_rules_for_delaunay})
  endforeach()
endif()
