function(smtk_test_plugin test_plugin_file)
  if (NOT EXISTS ${test_plugin_file})
    message(WARNING "Cannot locate test plugins file <${test_plugin_file}>.")
  else()
    set(response_file)
    if (WIN32)
      # Force response file usage. The command line gets way too long on Windows
      # without this. Once VTK_USE_FILE and PARAVIEW_USE_FILE are gone, this can be
      # removed again.
      set(response_file -DCMAKE_NINJA_FORCE_RESPONSE_FILE:BOOL=ON)
    endif ()

    string(MD5 hashed_test_dir ${test_plugin_file})
    set(test_dir
      "${CMAKE_BINARY_DIR}/Testing/Temporary/PluginTests/${scratch_dir}/${hashed_test_dir}")
    set(src_dir "${test_dir}/src")
    set(build_dir "${test_dir}/build")
    file(MAKE_DIRECTORY ${src_dir})
    file(MAKE_DIRECTORY ${build_dir})
    configure_file(${test_plugin_file} ${src_dir}/CMakeLists.txt COPYONLY)

    get_filename_component(test_name ${test_plugin_file} NAME_WE)
    add_test(NAME ${test_name}
      COMMAND ${CMAKE_CTEST_COMMAND}
      --build-and-test ${src_dir} ${build_dir}
      --build-generator ${CMAKE_GENERATOR}
      --build-options
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DENABLE_TESTING=ON
        -DMOAB_DIR=${MOAB_DIR}
        -DParaView_DIR=${ParaView_DIR}
        -Dpybind11_DIR=${pybind11_DIR}
        -DQt5_DIR=${Qt5_DIR}
        -DRemus_DIR=${Remus_DIR}
        -Dsmtk_DIR=${PROJECT_BINARY_DIR}
        -DZeroMQ_ROOT_DIR=${ZeroMQ_ROOT_DIR}
        ${response_file}
      )
  endif()
endfunction(smtk_test_plugin)
