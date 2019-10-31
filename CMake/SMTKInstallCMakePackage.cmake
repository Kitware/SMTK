if (NOT (DEFINED smtk_cmake_dir AND
         DEFINED smtk_cmake_build_dir AND
         DEFINED smtk_cmake_destination))
  message(FATAL_ERROR
    "SMTKInstallCMakePackage is missing input variables.")
endif ()

set(prefix_file "${smtk_cmake_build_dir}/smtk-prefix.cmake")
file(WRITE "${prefix_file}"
  "set(_smtk_import_prefix \"\${CMAKE_CURRENT_LIST_DIR}\")\n")
set(destination "${smtk_cmake_destination}")
while (destination)
  get_filename_component(destination "${destination}" DIRECTORY)
  file(APPEND "${prefix_file}"
    "get_filename_component(_smtk_import_prefix \"\${_smtk_import_prefix}\" DIRECTORY)\n")
endwhile ()

configure_file(
  "${smtk_cmake_dir}/smtkConfig.cmake.in"
  "${smtk_cmake_build_dir}/smtkConfig.cmake"
  @ONLY)
include(CMakePackageConfigHelpers)
write_basic_package_version_file("${smtk_cmake_build_dir}/smtkConfigVersion.cmake"
  VERSION "${SMTK_VERSION}"
  COMPATIBILITY AnyNewerVersion)

# For convenience, a package is written to the top of the build tree. At some
# point, this should probably be deprecated and warn when it is used.
file(GENERATE
  OUTPUT  "${CMAKE_BINARY_DIR}/smtkConfig.cmake"
  CONTENT "include(\"${smtk_cmake_build_dir}/smtkConfig.cmake\")\n")
configure_file(
  "${smtk_cmake_build_dir}/smtkConfigVersion.cmake"
  "${CMAKE_BINARY_DIR}/smtkConfigVersion.cmake"
  COPYONLY)

set(smtk_cmake_module_files
  EncodeStringFunctions.cmake
  SMTKMacros.cmake
  SMTKOperationXML.cmake
  SMTKPluginMacros.cmake
  qtSMTKViewImplementation.cxx.in
  qtSMTKViewImplementation.h.in
  serverSource.cxx.in)

set(smtk_cmake_files_to_install
  "${prefix_file}")

# FIXME: This is required because the VTK module system thinks the headers
# aren't being installed.
if (SMTK_ENABLE_VTK_SUPPORT)
  list(APPEND smtk_cmake_files_to_install
    "${smtk_cmake_build_dir}/SMTKVTKModules-vtk-module-properties.cmake")

  if (SMTK_ENABLE_PARAVIEW_SUPPORT)
    list(APPEND smtk_cmake_files_to_install
      "${smtk_cmake_build_dir}/SMTKParaViewVTKModules-vtk-module-properties.cmake")
  endif ()

  if (SMTK_ENABLE_VXL_SUPPORT)
    list(APPEND smtk_cmake_files_to_install
      "${smtk_cmake_build_dir}/SMTKVXLExt-vtk-module-properties.cmake")
  endif ()

  if (SMTK_ENABLE_POLYGON_SESSION)
    list(APPEND smtk_cmake_files_to_install
      "${smtk_cmake_build_dir}/SMTKPolygonExt-vtk-module-properties.cmake")
  endif ()

  if (SMTK_ENABLE_DISCRETE_SESSION)
    list(APPEND smtk_cmake_files_to_install
      "${smtk_cmake_build_dir}/SMTKDiscreteModules-vtk-module-properties.cmake")
  endif ()
endif ()

foreach (smtk_cmake_module_file IN LISTS smtk_cmake_module_files)
  configure_file(
    "${smtk_cmake_dir}/${smtk_cmake_module_file}"
    "${smtk_cmake_build_dir}/${smtk_cmake_module_file}"
    COPYONLY)
  list(APPEND smtk_cmake_files_to_install
    "${smtk_cmake_module_file}")
endforeach ()

include(SMTKInstallCMakePackageHelpers)
if (NOT SMTK_RELOCATABLE_INSTALL)
  list(APPEND smtk_cmake_files_to_install
    "${smtk_cmake_build_dir}/smtk-find-package-helpers.cmake")
endif ()

foreach (smtk_cmake_file IN LISTS smtk_cmake_files_to_install)
  if (IS_ABSOLUTE "${smtk_cmake_file}")
    file(RELATIVE_PATH smtk_cmake_subdir_root "${smtk_cmake_build_dir}" "${smtk_cmake_file}")
    get_filename_component(smtk_cmake_subdir "${smtk_cmake_subdir_root}" DIRECTORY)
    set(smtk_cmake_original_file "${smtk_cmake_file}")
  else ()
    get_filename_component(smtk_cmake_subdir "${smtk_cmake_file}" DIRECTORY)
    set(smtk_cmake_original_file "${smtk_cmake_dir}/${smtk_cmake_file}")
  endif ()
  install(
    FILES       "${smtk_cmake_original_file}"
    DESTINATION "${smtk_cmake_destination}/${smtk_cmake_subdir}"
    COMPONENT   "development")
endforeach ()

install(
  FILES       "${smtk_cmake_build_dir}/smtkConfig.cmake"
              "${smtk_cmake_build_dir}/smtkConfigVersion.cmake"
  DESTINATION "${smtk_cmake_destination}"
  COMPONENT   "development")

if (SMTK_ENABLE_VTK_SUPPORT)
  vtk_module_export_find_packages(
    CMAKE_DESTINATION "${smtk_cmake_destination}"
    FILE_NAME         "smtk-vtk-module-find-packages.cmake"
    MODULES           ${smtk_modules})
endif ()
