if(NOT WIN32)
  add_executable(unitModelBody unitModelBody.cxx)
  target_link_libraries(unitModelBody SMTKCore)
  add_test(unitModelBody ${EXECUTABLE_OUTPUT_PATH}/unitModelBody)


  add_executable(unitImportExportJSON unitImportExportJSON.cxx)
  target_link_libraries(unitImportExportJSON SMTKCore)
  add_test(unitImportExportJSON ${EXECUTABLE_OUTPUT_PATH}/unitImportExportJSON
    "${CMAKE_CURRENT_SOURCE_DIR}/cube.json")
endif()

add_executable(unitArrangement unitArrangement.cxx)
target_link_libraries(unitArrangement SMTKCore)
add_test(unitArrangement ${EXECUTABLE_OUTPUT_PATH}/unitArrangement)
