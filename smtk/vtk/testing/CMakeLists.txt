add_executable(displayModel displayModel.cxx)
target_link_libraries(displayModel SMTKCore vtkSMTK ${VTK_MODULES})
add_test(displayModel ${EXECUTABLE_OUTPUT_PATH}/displayModel "${SMTK_SOURCE_DIR}/model/testing/cube.json")
