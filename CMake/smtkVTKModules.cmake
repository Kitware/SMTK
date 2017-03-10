#------------------------------------------------------------------------------

# These macros are borrowed from ParaviewPlugins.cmake.
# The pv-plugin and pv-cs related stuff were removed.

#------------------------------------------------------------------------------
# locates module.cmake files under the current source directory and registers
# them as modules. All identified modules are treated as enabled and are built.
macro(vtk_smtk_process_modules)
  if (VTK_WRAP_PYTHON)
    # this is needed to ensure that the PYTHON_INCLUDE_DIRS variable is set when
    # we process the plugins.
    find_package(PythonLibs)
  endif()

  unset (VTK_MODULES_ALL)
  file(GLOB_RECURSE files RELATIVE
    "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/module.cmake")
  foreach (module_cmake IN LISTS files)
    get_filename_component(base "${module_cmake}" PATH)
    vtk_add_module(
      "${CMAKE_CURRENT_SOURCE_DIR}/${base}"
      module.cmake
      "${CMAKE_CURRENT_BINARY_DIR}/${base}"
      ${_test_languages})
  endforeach()

  set (current_module_set ${VTK_MODULES_ALL})
  list(APPEND VTK_MODULES_ENABLED ${VTK_MODULES_ALL})

  # sort the modules based on depedencies. This will endup bringing in
  # VTK-modules too. We raise errors if required VTK modules are not already
  # enabled.
  include(TopologicalSort)
  topological_sort(VTK_MODULES_ALL "" _DEPENDS)

  set (current_module_set_sorted)
  foreach(module IN LISTS VTK_MODULES_ALL)
    list(FIND current_module_set ${module} _found)
    if (_found EQUAL -1)
      # this is a VTK module and must have already been enabled. Otherwise raise
      # error.
      list(FIND VTK_MODULES_ENABLED ${module} _found)
      if (_found EQUAL -1)
        message(FATAL_ERROR
          "Requested modules not available: ${module}")
      endif()
    else ()
      list(APPEND current_module_set_sorted ${module})
    endif ()
  endforeach()

  foreach(_module IN LISTS current_module_set_sorted)
    if (NOT ${_module}_IS_TEST)
      set(vtk-module ${_module})
    else()
      set(vtk-module ${${_module}_TESTS_FOR})
    endif()
    add_subdirectory("${${_module}_SOURCE_DIR}" "${${_module}_BINARY_DIR}")
    unset(vtk-module)
  endforeach()

  unset (VTK_MODULES_ALL)
  unset (current_module_set)
  unset (current_module_set_sorted)
endmacro()

# this macro is used to setup the environment for loading/building VTK modules
macro(vtk_smtk_setup_module_environment _name)
  # Setup enviroment to build VTK modules outside of VTK source tree.
  set (BUILD_SHARED_LIBS ${VTK_BUILD_SHARED_LIBS})

  if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  endif()
  if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
  endif()
  if (NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
  endif()

  set (VTK_INSTALL_RUNTIME_DIR "bin")
  set (VTK_INSTALL_LIBRARY_DIR "lib")
  set (VTK_INSTALL_ARCHIVE_DIR "lib")
  set (VTK_INSTALL_INCLUDE_DIR "include")
  set (VTK_INSTALL_PACKAGE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${_name}")

  if (NOT VTK_FOUND)
    set (VTK_FOUND ${ParaView_FOUND})
  endif()
  if (VTK_FOUND)
    set (VTK_VERSION
      "${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${VTK_BUILD_VERSION}")
  endif()

  include(vtkExternalModuleMacros)
  if (SMTK_ENABLE_PYTHON_WRAPPING AND VTK_WRAP_PYTHON)
    if (NOT PYTHON_MAJOR_VERSION OR NOT PYTHON_MINOR_VERSION)
      # Currently, VTK's FindPythonLibs.cmake sets the above variables, but
      # CMake's FindPythonLibs.cmake does not. An issue has been made for VTK
      # to update its FindPythonLibs to reflect the parameters of the newer
      # CMake version (see https://gitlab.kitware.com/vtk/vtk/issues/16848).
      # Until this is resolved, we manually set these legacy parameters here.

      string (FIND ${PYTHONLIBS_VERSION_STRING} "." first_separator)
      string (SUBSTRING ${PYTHONLIBS_VERSION_STRING} 0 ${first_separator} PYTHON_MAJOR_VERSION)
      math (EXPR first_separator ${first_separator}+1)
      string (SUBSTRING ${PYTHONLIBS_VERSION_STRING} ${first_separator} -1 remainder)
      string (FIND ${remainder} "." second_separator)
      string (SUBSTRING ${remainder} 0 ${second_separator} PYTHON_MINOR_VERSION)
    endif()
    include(vtkPythonWrapping)
  endif()

  # load information about existing modules.
  foreach (mod IN LISTS VTK_MODULES_ENABLED)
    vtk_module_load("${mod}")
  endforeach()

endmacro()
