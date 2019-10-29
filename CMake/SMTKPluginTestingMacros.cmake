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
function(smtk_test_plugin test_plugin_file_url)

  # Create a testing directory for the plugin based off of its hashed file name.
  string(MD5 hashed_test_dir ${test_plugin_file_url})
  string(SUBSTRING ${hashed_test_dir} 0 8 hashed_test_dir)
  set(test_dir "${CMAKE_BINARY_DIR}/PluginTests/${hashed_test_dir}")

  # Set up a source directory for the plugin.
  set(src_dir "${test_dir}/src")
  file(MAKE_DIRECTORY ${src_dir})

  # Set up a build directory for the plugin.
  set(build_dir "${test_dir}/build")
  file(MAKE_DIRECTORY ${build_dir})

  # Download the contract file into the source directory.
  file(DOWNLOAD ${test_plugin_file_url} ${src_dir}/CMakeLists.txt)

  # Check result for success
  if (NOT EXISTS ${src_dir}/CMakeLists.txt)
    message(WARNING "Cannot download test plugins file <${test_plugin_file_url}>.")
    return()
  endif()

  # Derive a test name from the contract file name.
  get_filename_component(test_name ${test_plugin_file_url} NAME_WE)

  # Add a test that builds and tests the plugin, but does not install it.
  add_test(NAME ${test_name}
    COMMAND ${CMAKE_CTEST_COMMAND}
    --build-and-test ${src_dir} ${build_dir}
    --build-generator ${CMAKE_GENERATOR}
    --build-options
      -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
      -DENABLE_TESTING=ON
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -Dsmtk_DIR=${PROJECT_BINARY_DIR}
  )
  # Plugin tests can take a little longer since they need to
  # clone a repo and build a project.
  set_tests_properties(${test_name} PROPERTIES TIMEOUT 600)

  # If on Windows, pass the environment PATH to the test.
  if (WIN32)
    # We need to add this smtk's binary directory to the path so the plugin
    # tests can find our newly built SMTK binaries.
    set(smtk_test_path $ENV{PATH})
    list(INSERT smtk_test_path 0 ${PROJECT_BINARY_DIR}/bin)

    # We need to escape semicolons so they won't be erased from the path when
    # resolved by the test's cmake instance.
    string(REPLACE ";" "\\\;" smtk_path_env "${smtk_test_path}")

    # Finally, append the path to the test's environment.
    set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "PATH=${smtk_path_env}")
  endif ()

  # On all operating systems we pass the environment PYTHONPATH to the test.

  # We need to add this smtk's build directory to the pythonpath so the plugin
  # tests can find our newly built SMTK python modules.
  set(smtk_test_path $ENV{PYTHONPATH})
  list(INSERT smtk_test_path 0 ${PROJECT_BINARY_DIR})

  # We need to escape semicolons so they won't be erased from the pythonpath
  # when resolved by the test's cmake instance.
  string(REPLACE ";" "\\\;" smtk_pythonpath_env "${smtk_test_path}")

  # Finally, append the pythonpath to the test's environment.
  set_property(TEST ${test_name} APPEND PROPERTY ENVIRONMENT "PYTHONPATH=${smtk_pythonpath_env}")

  set_tests_properties(${test_name} PROPERTIES LABELS "Plugin")

endfunction(smtk_test_plugin)
