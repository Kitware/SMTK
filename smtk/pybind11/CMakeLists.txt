set(build_path "${CMAKE_BINARY_DIR}/${SMTK_PYTHON_MODULEDIR}/smtk")
set(install_path "${SMTK_PYTHON_MODULEDIR}/smtk")

if (WIN32 AND (_smtk_superbuild_prefix OR SMTK_DLL_PATHS))
  set(smtk_dll_paths)

  if (_smtk_superbuild_prefix)
    list(APPEND smtk_dll_paths
      # The superbuild tries to place all loadable DLLs in the `bin` directory.
      "${_smtk_superbuild_prefix}/bin")
  endif ()

  list(APPEND smtk_dll_paths
    ${SMTK_DLL_PATHS})

  if (smtk_dll_paths)
    string(REPLACE ";" "',\n    '" smtk_dll_paths_python "${smtk_dll_paths}")
    file(WRITE "${build_path}/_build_paths.py"
      "paths = [\n    '${smtk_dll_paths_python}',\n]\n")
  else ()
    file(REMOVE "${build_path}/_build_paths.py")
  endif ()
endif ()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py.in"
  "${build_path}/__init__.py" @ONLY
  )

install(
  FILES "${build_path}/__init__.py"
  DESTINATION "${install_path}"
  )

if (smtk_dll_paths AND
    # Don't install for a relocatable install tree.
    NOT SMTK_RELOCATABLE_INSTALL AND
    # Also not needed for a unified install tree.
    NOT SMTK_UNIFIED_INSTALL_TREE)
  install(
    FILES "${build_path}/_build_paths.py"
    DESTINATION "${install_path}"
    )
endif ()
