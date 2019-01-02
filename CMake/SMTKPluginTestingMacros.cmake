#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

# smtk_test_plugin <plugin_file>
#
# add a test that builds and tests the smtk plugin as described in the test
# plugin file.
function(smtk_test_plugin test_plugin_file)

  if (NOT EXISTS ${test_plugin_file})
    message(WARNING "Cannot locate test plugins file <${test_plugin_file}>.")
    return()
  endif()

  # If on Windows, force response file usage. The command line gets way too long
  # on Windows without this. Once VTK_USE_FILE and PARAVIEW_USE_FILE are gone,
  # this can be removed.
  set(response_file)
  if (WIN32)
    set(response_file -DCMAKE_NINJA_FORCE_RESPONSE_FILE:BOOL=ON)
  endif ()

  #
  string(MD5 hashed_test_dir ${test_plugin_file})
  set(test_dir "${CMAKE_BINARY_DIR}/PluginTests/${scratch_dir}/${hashed_test_dir}")
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
      -Dsmtk_DIR=${PROJECT_BINARY_DIR}
      ${response_file}
    )
  set_tests_properties(${test_name} PROPERTIES LABELS "Plugin")

endfunction(smtk_test_plugin)
